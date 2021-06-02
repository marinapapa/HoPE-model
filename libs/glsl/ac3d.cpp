// ac3d.cpp .ac importer.
//
// Hanno Hildenbrandt 2010

#include <fstream>
#include <algorithm>
#include <exception>
#include <map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ac3d.hpp"

#ifdef AC3D_USE_HILBERT_SORTING
  #include <hrtree/isfc/key_gen.hpp>
  #include <hrtree/isfc/hilbert.hpp>
  #include <glmutils/bbox.hpp>
  HRTREE_ADAPT_POINT_FUNCTION(glm::vec3, float, 3, glm::value_ptr);
#endif



namespace glsl { 
  
  namespace {

    typedef std::vector<glm::vec3> pos_vect;
 

    std::istream& read_glm_array(std::istream& is, int n, float* v)
    {
      for (int i=0; i<n; ++i) {
        is >> v[i];
      }
      return is;
    }

    std::istream& operator >> (std::istream& is, glm::vec2& v)
    {
      return read_glm_array(is, 2, glm::value_ptr(v));
    }


    std::istream& operator >> (std::istream& is, glm::vec3& v)
    {
      return read_glm_array(is, 3, glm::value_ptr(v));
    }


    std::istream& operator >> (std::istream& is, glm::mat3& v)
    {
      return read_glm_array(is, 3*3, glm::value_ptr(v));
    }
 

    glm::vec2 asVec2(glm::tvec2<short> svec2) noexcept
    {
      return glm::vec2(svec2) * (1.0f / std::numeric_limits<short>::max());
    }


    glm::tvec2<short> asSvec2(glm::vec2 v) noexcept
    {
      return glm::tvec2<short>(v * (float)(std::numeric_limits<short>::max()));
    }


    // Homogeneous transformation
    glm::vec3 transformPoint(const glm::mat4& M, const glm::vec3& v) noexcept 
    {
      const auto s = 1.0f / (M[0][3] * v.x + M[1][3] * v.y + M[2][3] * v.z + M[3][3]);
      return glm::vec3( (M[0][0] * v.x + M[1][0] * v.y + M[2][0] * v.z + M[3][0]) * s,
                        (M[0][1] * v.x + M[1][1] * v.y + M[2][1] * v.z + M[3][1]) * s,
                        (M[0][2] * v.x + M[1][2] * v.y + M[2][2] * v.z + M[3][2]) * s );
    }


    struct cmpeq_vertex_eps
    {
      cmpeq_vertex_eps(const V4F_N3F_T2S& pivot) noexcept : pivot_(pivot) {}
      bool operator() (const V4F_N3F_T2S& x)
      {
        return glm::all(glm::epsilonEqual(pivot_.v, x.v, 0.0001f)) &&
               glm::all(glm::epsilonEqual(pivot_.n, x.n, 0.2f)) &&
               (pivot_.t == x.t);
      }
      V4F_N3F_T2S pivot_;
    };


    struct cmpeq_normal_crease
    {
      cmpeq_normal_crease(const glm::vec3& a, float crease) 
      : a_(a), 
        crease_(std::cos(glm::radians(crease))) 
      {}

      bool operator() (const glm::vec3& b) const
      {
        float cdot = glm::dot(a_, b);
        return crease_ <= cdot;
      }

      glm::vec3 a_;
      float crease_;
    };


    struct acTriangle
    {
      int vi[3];
      glm::vec2 tex[3];
    };

  #ifdef AC3D_USE_HILBERT_SORTING
    struct hilbert_cmp_tri_center
    {
      typedef hrtree::hilbert<3,21>::type key_type;
      typedef hrtree::key_gen<key_type, glm::vec3> gen_type;

      hilbert_cmp_tri_center(const pos_vect& V, const glmutils::bbox3& d) : v(V), domain(d.p0(), d.p1()) {}

      bool operator () (const acTriangle& a, const acTriangle& b) const
      {
        glm::vec3 ca = (1.0f/3.0f) * (v[a.vi[0]] + v[a.vi[1]] + v[a.vi[2]]);
        glm::vec3 cb = (1.0f/3.0f) * (v[b.vi[0]] + v[b.vi[1]] + v[b.vi[2]]);
        return domain(ca) < domain(cb);
      }

      gen_type domain;
      const pos_vect& v;
    };
  #endif

    struct acObject
    {
      acObject() noexcept : kids(-1), twoSided(false) {}

      glm::vec3 face_normal(size_t face) const
      {
        const glm::vec3 v0 = vert[triangles[face].vi[0]];
        const glm::vec3 v1 = vert[triangles[face].vi[1]];
        const glm::vec3 v2 = vert[triangles[face].vi[2]];
        return glm::normalize(glm::cross(v1 - v0, v2 - v0));
      }

      int kids;
      int mat;
      std::string name;
      std::string texture;
      float crease;
      glm::mat3 rot;
      glm::vec3 loc;
      pos_vect vert;
      std::vector<acTriangle> triangles;
      bool twoSided;
    };


    class acParser
    {
    public:
      acParser(const std::string& acFile)
        : acFileName_(acFile), is_(acFile.c_str())
      {
      }

      // Parse ac3d file. May throw.
      void Start();

      // Calculate V4F_N3F_T2S vector from object objectID and its kids.
      ac3d_model Triangulate(size_t objectID);
  
    public:
      typedef void (acParser::*apmf)();
      void takeState(const std::string& token);
      void read_ignore();
      void read_MATERIAL();
      void read_OBJECT();
      void read_texture();
      void read_crease();
      void read_rot();
      void read_loc();
      void read_name();
      void read_numvert();
      void read_numsurf();
      void read_kids();
      void read_rgb();
      void read_amb();
      void read_emis();
      void read_shi();
      void read_trans();
      void read_fail();
  
    private:
      // Translate the object and its kids.
      void TranslateObj(size_t i, glm::mat4 M, pos_vect& v);
      glm::vec3 vertexNormal(const acObject& obj, int face, int id);

    private:
      std::fstream is_;
      ac3d_material material_;
      std::vector<acObject> objects_;
      std::string acFileName_;
      static std::map<const std::string, apmf> states_;
    };


    std::map<const std::string, acParser::apmf> acParser::states_{
      { "AC3Db", &acParser::read_ignore },
      { "MATERIAL", &acParser::read_MATERIAL },
      { "OBJECT", &acParser::read_OBJECT },
      { "name", &acParser::read_name },
      { "data", &acParser::read_ignore },
      { "texture", &acParser::read_texture },
      { "texrep", &acParser::read_ignore },
      { "crease", &acParser::read_crease },
      { "rot", &acParser::read_rot },
      { "loc", &acParser::read_loc },
      { "url", &acParser::read_ignore },
      { "numvert", &acParser::read_numvert },
      { "numsurf", &acParser::read_numsurf },
      { "kids", &acParser::read_kids },
      { "fail", &acParser::read_fail }
    };


    void acParser::Start()
    {
      std::string tok;
      while (!is_.eof())
      {
        is_ >> tok;
        takeState(tok);  
      }
    }


    void acParser::takeState(const std::string& token)
    {
      const auto state = states_.find(token);
      (state != states_.end()) ? (this->*state->second)() : read_fail();
    }


    void acParser::read_ignore() 
    { 
      char buf[128]; is_.getline(&buf[0], 128); 
    }


    void acParser::read_OBJECT() 
    {
      objects_.emplace_back();
      std::string tok;
      is_ >> tok;
      while (!is_.eof())
      {
        is_ >> tok;        // skip
        takeState(tok);
        if (objects_.back().kids >= 0) break;  // last object token
      }
    }


    void acParser::read_MATERIAL() 
    {
      std::string tok;
      is_ >> tok;            // ignore material name
      is_ >> tok >> material_.rgb;
      is_ >> tok >> material_.amb;
      is_ >> tok >> material_.emis;
      is_ >> tok >> material_.spec;
      is_ >> tok >> material_.shi;
      is_ >> tok >> material_.trans;
    }


    void acParser::read_texture() 
    { 
      std::string texString;
      is_ >> texString;
      // Trim leading/trailing '"'
      objects_.back().texture = texString.substr(1, texString.length() - 2u);
    }


    void acParser::read_name() 
    { 
      std::string name;
      is_ >> name;
      // Trim leading/trailing '"'
      objects_.back().name = name.substr(1, name.length() - 2u);
    }


    void acParser::read_crease() 
    { 
      is_ >> objects_.back().crease; 
    }
  

    void acParser::read_rot() 
    { 
      is_ >> objects_.back().rot; 
    }
  

    void acParser::read_loc() 
    { 
      is_ >> objects_.back().loc; 
    }
  

    void acParser::read_numvert() 
    {
      int N; is_ >> N;
      for (int i=0; i<N; ++i)
      {
        glm::vec3 v; is_ >> v;
        objects_.back().vert.push_back(v);
      }
    }
  

    void acParser::read_numsurf() 
    {
      int S; is_ >> S;
      for (int i=0; i<S;) 
      {
        int refs{};
        unsigned flags{};
        std::string tok; is_ >> tok;
        if (0 == strcmp(tok.c_str(), "SURF")) 
        {
          is_ >> std::hex >> flags >> std::dec;
          if (flags >> 5) objects_.back().twoSided = true;
        }
        else if (0 == strcmp(tok.c_str(), "mat")) is_ >> refs;
        else if (0 == strcmp(tok.c_str(), "refs")) 
        {
          is_ >> refs;
          if (3 != refs) read_fail();
          acTriangle surf{};
          for (int r=0; r<refs; ++r) 
          {
            is_ >> surf.vi[r];
            is_ >> surf.tex[r];
          }
          objects_.back().triangles.push_back(surf);
          ++i;
        }
        else read_fail();
      }
    }


    void acParser::read_kids() 
    { 
      is_ >> objects_.back().kids; 
    }
  

    void acParser::read_fail() 
    {  
      throw std::exception((std::string("Parsing AC3D file '") + acFileName_ + "' failed").c_str()); 
    }


    ac3d_model acParser::Triangulate(size_t objectID)
    {
      ac3d_model model;
      pos_vect v; TranslateObj(objectID, glm::rotate(glm::mat4(1), 0.0f, glm::vec3(0,1,0)), v);
      acObject& obj(objects_[objectID]);
  #ifdef AC3D_USE_HILBERT_SORTING    
      // Sort triangle by hilbert value (increase cache coherence)
      hilbert_cmp_tri_center cmp(v, model.bbox);
      std::sort(obj.triangles.begin(), obj.triangles.end(), std::ref(cmp));
  #endif
      if (obj.vert.empty()) return model;
      model.twoSided = obj.twoSided;
      for (int i=0; i<static_cast<int>(obj.triangles.size()); ++i)
      {
        const acTriangle& tri(obj.triangles[i]);
        V4F_N3F_T2S vertex[3];
        for (int j=0; j<3; ++j) 
        {
          vertex[j].v = glm::vec4(v[tri.vi[j]], 1.0f);
          vertex[j].n = vertexNormal(obj, i, tri.vi[j]);
          vertex[j].t = asSvec2(glm::vec2(tri.tex[j].x, 1.0f - tri.tex[j].y));
          auto it = std::find_if(model.vertices.begin(), model.vertices.end(), cmpeq_vertex_eps(vertex[j]));
          if (it != model.vertices.end())
          {
            model.indices.push_back(static_cast<unsigned>(std::distance(model.vertices.begin(), it)));
          }
          else
          {
            model.vertices.push_back(vertex[j]);
            model.indices.push_back(static_cast<unsigned>(model.vertices.size()) - 1u);
          }
        }
      }
      model.texFile = obj.texture;
      model.material = material_;
      return model;
    }


    void acParser::TranslateObj(size_t i, glm::mat4 M, pos_vect& v)
    {
      acObject& obj(objects_[i]);
      M = glm::translate(M, obj.loc);
      for (auto x : obj.vert) { v.emplace_back(transformPoint(M, x)); }
      for (int k=0; k<obj.kids; ++k) 
      { // add kid
        TranslateObj(i+1+k, M, v);
      }
    }


    glm::vec3 acParser::vertexNormal(const acObject& obj, int face, int id)
    {
      // Terrible ineffective but how cares?
      glm::vec3 normal = obj.face_normal(size_t(face));
      int adjacent_normals = 1;
      const cmpeq_normal_crease cmp_crease(normal, obj.crease);
      for (size_t i = 1; i < obj.triangles.size(); ++i)
      {
        const acTriangle& tri = obj.triangles[i];
        if ((id == tri.vi[0]) || (id == tri.vi[1]) || (id == tri.vi[2]))
        {
          const glm::vec3 adjacent_normal = obj.face_normal(i);
          if (cmp_crease(adjacent_normal))
          {
            normal += adjacent_normal;
            ++adjacent_normals;
          }
        }
      }
      return glm::normalize(normal / float(adjacent_normals));
    }

  }


  ac3d_model ImportAC3D(const std::string& ac3dFile)
  {
    acParser parser(ac3dFile);
    parser.Start();
    return parser.Triangulate(1);
  }

}

