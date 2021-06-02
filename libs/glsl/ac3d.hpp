//! \file ac3d.hpp AC3D file importer
//! \ingroup Graphics

#ifndef GLSL_AC3D_HPP_INCLUDED
#define GLSL_AC3D_HPP_INCLUDED

#include <string>
#include <vector>
#include "glm/glm.hpp"


//#define AC3D_USE_HILBERT_SORTING


namespace glsl {

  struct V4F_N3F_T2S {
    glm::vec4 v;
    glm::vec3 n;
    glm::tvec2<short> t;
  };


  struct ac3d_material
  {
    glm::vec3 rgb;
    glm::vec3 amb;
    glm::vec3 emis;
    glm::vec3 spec;
    float shi;
    float trans;
  };


  struct ac3d_model
  {
    std::vector<V4F_N3F_T2S> vertices; 
    std::vector<unsigned> indices;
    std::string texFile;
    bool twoSided;
    ac3d_material material;
  };


  //! \brief Import ac3d file. 
  ac3d_model ImportAC3D(const std::string& ac3dFile);

}


#endif
