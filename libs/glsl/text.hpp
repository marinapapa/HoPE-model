#ifndef GLSL_TEXT_HPP_INCLUDED
#define GLSL_TEXT_HPP_INCLUDED

#include <memory>
#include <string>
//#include <string_view>
#include <map>
#include <functional>
#include "bmfont.hpp"


namespace bmf = glsl::bmfont;

namespace glsl {


  // Commands understood from Text::print
  // 
  // \color{float float float}
  // \orig{int int}
  // \cursor{int int}
  // \ofs{float float}
  // \tabsize{int}
  // \big{}
  // \bigb{}
  // \normal{}
  // \normalb{}
  // \small{}
  // \smallb{}
  // \t
  // \n
  //
  class Text
  {
    struct string_view
    {
      explicit operator bool () const noexcept { return begin != end; }
      constexpr bool empty() noexcept { return begin == end; }
      constexpr char operator*() { return *begin; }
      string_view& operator++() noexcept { ++begin; return *this; }
      string_view operator++(int) const noexcept { return { begin + 1, end }; }
      string_view pair() const { return { begin, begin + 1 }; }
      const char* begin;
      const char* end;
    };

    struct less
    {
      typedef int is_transparent;
      bool operator()(const string_view& a, const std::string& b) const;
      bool operator()(const std::string& a, const string_view& b) const;
      bool operator()(const std::string& a, const std::string& b) const;
    };

  public:
    using font_map_t = std::map<std::string, std::unique_ptr<bmf::GlyphBuffer>, less>;

  public:
    Text(GLuint prog, font_map_t* faces);
    ~Text();

    int font_size() const { return text2D_.FontSize(); }
    int line_height() const { return text2D_.LineHeight(); }
    int base() const { return text2D_.Base(); }
    glm::ivec2 cursor() const { return text2D_.GetCursor(); };
    glm::ivec2 orig() const { return text2D_.GetOrig(); };

    glm::ivec2 extent(string_view sv);
    void print(string_view sv);
    void set_color(const glm::vec4& color) { text2D_ << bmf::color_manip(color); }
    void set_orig(const glm::ivec2& orig) { text2D_ << bmf::orig_manip(orig); }
    void set_cursor(const glm::ivec2& cursor) { text2D_ << bmf::cursor_manip(cursor); }
    void set_tabsize(int tab) { text2D_ << bmf::tab_manip(tab); }

    void Flush();
    void Render();

  private:
    font_map_t* Faces_ = nullptr;
    std::map<std::string, std::function<bool(bmf::Text2D&, const char*)>, less> command_map_;
    bmf::Text2D text2D_;

    bool match_command(bmf::Text2D& txt, string_view, string_view arg);
    const char* parse_commands(bmf::Text2D& txt, string_view command);
    void resize_buffer(size_t n);

    unsigned totVertexCount_ = 0;
    std::vector<unsigned> vertex_count_per_face_;
    GLuint vbo_ = GL_NONE;
    void* pbuf_ = nullptr;
    size_t buf_size_ = 0;
    GLuint vao_ = GL_NONE;
    GLuint prog_ = GL_NONE;
  };

}

#endif
