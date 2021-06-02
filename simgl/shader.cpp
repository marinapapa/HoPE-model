#include <string>
#include "shader.h"


namespace shader {


  // Common uniform block.
  const char* uniformBlock = R"glsl(

    #version 440 core
  
    layout (std140, binding = 1) uniform UniformBlock
    {
      mat4 V;            // View
      mat4 P;            // Projection
      mat4 MV;           // ModelView
      mat4 MVP;          // ModelViewProjection
      mat4 ITMV;         // Inverse-transpose model view
      mat4 Ortho;        // Ortho
      mat4 Text;         // Text
      float zoom;
    };

  )glsl";


  const char* birdVertex = R"glsl(

    layout (location = 0) uniform float scale = 1.0;
    layout (location = 0) in vec4 Vertex;
    layout (location = 1) in vec2 TexCoord;
    layout (location = 2) in vec4 pos_vel;
    layout (location = 3) in vec4 side_tex_alpha;

    smooth out vec2 vTex;
    flat out float vSpectrum;
    flat out float vAlpha;
    flat out vec4 vVertex;

    void main()
    {
      vTex = TexCoord;
      vSpectrum = side_tex_alpha.z;
      vAlpha = side_tex_alpha.a;
      vec2 pos = pos_vel.xy;      
      vec2 dir = normalize(pos_vel.zw);
      vec2 v = scale * Vertex.xy;
      mat2 R = mat2(dir.x,  dir.y, -dir.y, dir.x);
      pos += R * v;
      vVertex = MVP * vec4(pos, 0.0, 1.0);
    }

  )glsl";


  const char* birdGeo = R"glsl(

    layout (triangles) in;
    layout (triangle_strip, max_vertices=27) out;

    smooth in vec2 vTex[3];
    flat in vec4 vVertex[3];
    flat in float vSpectrum[3];
    flat in float vAlpha[3];

    smooth out vec2 gTex;
    flat out float gSpectrum;
    flat out float gAlpha;

    void EmitTriangle(vec4 ofs)
    {
      gTex = vTex[0];
      gl_Position = vVertex[0] + ofs;
      EmitVertex();            
      gTex = vTex[1];
      gl_Position = vVertex[1] + ofs;
      EmitVertex();            
      gTex = vTex[2];
      gl_Position = vVertex[2] + ofs;
      EmitVertex();     
      EndPrimitive();
    }

    void main()
    {
      gSpectrum = vSpectrum[0];
      gAlpha = vAlpha[0];
      float s = 2 * zoom * MV[0][0].x;
      EmitTriangle(vec4(0, 0, 0, 0));
      EmitTriangle(vec4(s, 0, 0, 0));
      EmitTriangle(vec4(-s, 0, 0, 0));
      EmitTriangle(vec4(0, s, 0, 0));
      EmitTriangle(vec4(0, -s, 0, 0));
      EmitTriangle(vec4(-s, -s, 0, 0));
      EmitTriangle(vec4(s, -s, 0, 0));
      EmitTriangle(vec4(-s, s, 0, 0));
      EmitTriangle(vec4(s, s, 0, 0));
    }
    
  )glsl";


  const char* birdFragment = R"glsl(

    layout (location = 1) uniform float texMix = 1.0;
    layout (binding = 1) uniform sampler2D Texture;
    layout (binding = 2) uniform sampler2D Spectrum;

    smooth in vec2 gTex;
    flat in float gSpectrum;
    flat in float gAlpha;

    layout (location = 0) out vec4 FragColor;

    void main()
    {
      if (gAlpha == 0.f) discard;
      vec4 color = texture(Texture, gTex);
      float fade = length(color.rgb);      
      float tm = texMix * ((gSpectrum < 0.0) ? 0.0 : 0.75);
      color = mix(color, texture(Spectrum, vec2(gSpectrum, 0.5)), tm);
      FragColor.rgb = color.rgb;
      FragColor.a = (1.0 - fade);
    }

  )glsl";


  const char* texVertex = R"glsl(

    layout (location = 0) in vec4 Vertex;

    flat out vec4 vVertex;
    flat out vec2 vTex;

    // triangle strip into texture
    const vec2[4] texcoord = vec2[4]( 
      vec2(1, 1), vec2(1, 0), vec2(0, 1), vec2(0, 0)
    );

    void main(void)
    {
      vTex = texcoord[gl_VertexID];
      vVertex = MVP * Vertex;
    }

  )glsl";


  const char* texGeo = R"glsl(

    layout (triangles) in;
    layout (triangle_strip, max_vertices=27) out;

    flat in vec4 vVertex[3];
    flat in vec2 vTex[3];

    smooth out vec2 gTex;

    void EmitTriangle(vec4 ofs)
    {
      gl_Position = vVertex[0] + ofs;
      gTex = vTex[0];
      EmitVertex();            
      gl_Position = vVertex[1] + ofs;
      gTex = vTex[1];
      EmitVertex();            
      gl_Position = vVertex[2] + ofs;
      gTex = vTex[2];
      EmitVertex();     
      EndPrimitive();
    }


    void main()
    {
      float s = zoom * 2.0 * MV[0][0].x;
      EmitTriangle(vec4(0, 0, 0, 0));
      EmitTriangle(vec4(s, 0, 0, 0));
      EmitTriangle(vec4(-s, 0, 0, 0));
      EmitTriangle(vec4(0, s, 0, 0));
      EmitTriangle(vec4(0, -s, 0, 0));
      EmitTriangle(vec4(-s, -s, 0, 0));
      EmitTriangle(vec4(s, -s, 0, 0));
      EmitTriangle(vec4(-s, s, 0, 0));
      EmitTriangle(vec4(s, s, 0, 0));
    }
    
  )glsl";

  
  const char* texFragment = R"glsl(

    layout (binding = 1) uniform sampler2D Texture;
    layout (location = 0) uniform float dimm = 1.0;

    smooth in vec2 gTex;
    out vec4 FragColor;

    void main(void) 
    {            
      FragColor = dimm * texture(Texture, gTex);
    }

  )glsl";


  const char* immVertex = R"glsl(

    layout (location = 0) in vec3 Vertex;
    layout (location = 1) in vec4 Color;
    layout (location = 2) in float Tex;

    smooth out vec4 vColor;
    flat out float vTex;
  
    void main(void) 
    {            
      vColor = Color;
      vTex = Tex;
      gl_Position = MVP * vec4(Vertex, 1.0);
    }

  )glsl";


  const char* immVertexTorus  = R"glsl(

    layout (location = 0) in vec3 Vertex;
    layout (location = 1) in vec4 Color;
    layout (location = 2) in float Tex;
    
    layout (location = 0) uniform vec3 dxy;

    smooth out vec4 vColor;
    flat out float vTex;
  
    void main(void) 
    {            
      float dx = dxy[gl_InstanceID % 3];
      float dy = dxy[gl_InstanceID / 3];
      vColor = Color;
      vTex = Tex;
      vec4 vertex = vec4(Vertex.x + dx, Vertex.y + dy, 0.0, 1.0);
      gl_Position = MVP * vertex;
    }

  )glsl";


  const char* immFragment = R"glsl(

    smooth in vec4 vColor;
    flat in float vTex;
    out vec4 FragColor;

    void main(void) 
    {            
      if (vTex == -100.0) discard;
      FragColor = vColor;
    }

  )glsl";


  const char* textVertex = R"glsl(

    layout (location = 0) in vec2 Vertex;
    layout (location = 1) in vec4 Color;
    layout (location = 2) in vec2 TexCoord;

    out vec2 vTexCoord;
    out vec2 vVertex;
    out vec4 vColor;

    void main()
    {
      vColor = Color;
      vTexCoord = TexCoord + vec2(0.5, 0.5);
      vVertex = Vertex + vec2(0.5, 0.5);
    }
  
  )glsl";


  const char* textGeo = R"glsl(

    layout(lines) in;
    layout(triangle_strip, max_vertices=4) out;

    in vec2 vTexCoord[2];
    in vec2 vVertex[2];
    in vec4 vColor[2];

    smooth out vec2 gTexCoord;
    flat out vec4 gColor;

    void main()
    {
      vec2 t0 = vTexCoord[0];
      vec2 t1 = vTexCoord[1];
      vec2 v0 = vVertex[0];
      vec2 v1 = vVertex[1];

      gColor = vColor[1];
      gTexCoord = vec2(t0.x, t1.y);
      gl_Position = Text * vec4(v0.x, v1.y, 0, 1);
      EmitVertex();

      gTexCoord = vec2(t0.x, t0.y);
      gl_Position = Text * vec4(v0.x, v0.y, 0, 1);
      EmitVertex();

      gTexCoord = vec2(t1.x, t1.y);
      gl_Position = Text * vec4(v1.x, v1.y, 0, 1);
      EmitVertex();

      gTexCoord = vec2(t1.x, t0.y);
      gl_Position = Text * vec4(v1.x, v0.y, 0, 1);
      EmitVertex();

      EndPrimitive();
    }
  )glsl";


  const char* textFragment = R"glsl(

    layout (binding = 2) uniform sampler2DRect FontTexture;

    smooth in vec2 gTexCoord;
    flat in vec4 gColor;

    out vec4 FragColor;

    void main()
    {
      vec4 color = gColor;
      color.a = texture2DRect(FontTexture, gTexCoord).r;
      FragColor = color;
    }
  )glsl";


  const char* ribbonVertex = R"glsl(

    layout (location = 0) uniform int S = 10;
    layout (location = 1) uniform int N = 10;
    layout (location = 2) uniform int first = 0;
    layout (location = 3) uniform float fade = 0.0;

    layout (std430, binding=2) buffer proxy
    { 
      vec4 data[];
    };

    smooth out float vSpectrum;
    flat out float vAlpha;
    flat out vec4 vVertex;

    void main()
    {
      int i = 2 * (first + gl_InstanceID + N * gl_VertexID);
      vec4 pos_vel = data[i];
      vec4 side_tex_alpha = data[i + 1];
      vSpectrum = side_tex_alpha.z;
      vAlpha = fade * (S - gl_VertexID) * side_tex_alpha.a;
      vec2 pos = pos_vel.xy;      
      vVertex = MVP * vec4(pos, 0.0, 1.0);
    }

  )glsl";


  const char* ribbonGeo = R"glsl(

    layout (points) in;
    layout (points, max_vertices=9) out;

    flat in vec4 vVertex[1];
    flat in float vSpectrum[1];
    flat in float vAlpha[1];

    flat out float gSpectrum;
    flat out float gAlpha;

    void EmitPoint(vec4 ofs)
    {
      gl_Position = vVertex[0] + ofs;
      gl_PointSize = 3;
      EmitVertex();            
      EndPrimitive();
    }

    void main()
    {
      gSpectrum = vSpectrum[0];
      gAlpha = vAlpha[0];
      float s = zoom * 2.0 * MV[0][0].x;
      EmitPoint(vec4(0, 0, 0, 0));
      EmitPoint(vec4(s, 0, 0, 0));
      EmitPoint(vec4(-s, 0, 0, 0));
      EmitPoint(vec4(0, s, 0, 0));
      EmitPoint(vec4(0, -s, 0, 0));
      EmitPoint(vec4(-s, -s, 0, 0));
      EmitPoint(vec4(s, -s, 0, 0));
      EmitPoint(vec4(-s, s, 0, 0));
      EmitPoint(vec4(s, s, 0, 0));
    }
    
  )glsl";


  const char* ribbonFragment = R"glsl(

    layout (binding = 2) uniform sampler2D Spectrum;

    flat in float gSpectrum;
    flat in float gAlpha;

    layout (location = 0) out vec4 FragColor;

    void main()
    {
      if (gAlpha <= 0.f) discard;
      //vec4 color = vec4(1,0,0,1);
      //float fade = length(color.rgb);      
      //float texMix = (gSpectrum < 0.0) ? 0.0 : 0.9;
      //color = mix(color, texture(Spectrum, vec2(gSpectrum, 0.5)), texMix);
      //FragColor.rgb = color.rgb;
      //FragColor.a = (1.0 - fade);
      vec4 color = texture(Spectrum, vec2(gSpectrum), 0.5);
      FragColor = color;
      FragColor.a = gAlpha;
    }

  )glsl";


  namespace {

    GLuint ShaderFromLiteral(const char* shaderSource, GLenum shaderType) noexcept
    {
      const auto sh = glCreateShader(shaderType);
      glShaderSource(sh, 1, &shaderSource, 0);
      glCompileShader(sh);
      return sh;
    }

  }


  GLuint ProgFromLiterals(const char* vertexShader, const char* fragmentShader, const char* geometryShader)
  {
    std::string mb = shader::uniformBlock;
    auto vSh = ShaderFromLiteral((mb + vertexShader).c_str(), GL_VERTEX_SHADER);
    auto fSh = ShaderFromLiteral((mb + fragmentShader).c_str(), GL_FRAGMENT_SHADER);
    GLuint gSh = (geometryShader) ? ShaderFromLiteral((mb + geometryShader).c_str(), GL_GEOMETRY_SHADER) : 0;
    const GLuint prog = glCreateProgram();
    glAttachShader(prog, vSh);
    glAttachShader(prog, fSh);
    if (gSh) glAttachShader(prog, gSh);
    glLinkProgram(prog);
    glDetachShader(prog, vSh);
    glDetachShader(prog, fSh);
    glDeleteShader(vSh);
    glDeleteShader(fSh);
    if (gSh) 
    {
      glDetachShader(prog, gSh);
      glDeleteShader(gSh);
    }
#ifdef GLSL_DEBUG_OUTPUT
    GLchar buf[1024];
    GLsizei length;
    glGetProgramInfoLog(prog, 1024, &length, buf);
    if (length) {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 1, GL_DEBUG_SEVERITY_HIGH, length, buf);
    }
#endif
    return prog;
  }


}
