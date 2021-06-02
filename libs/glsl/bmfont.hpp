#ifndef BMFONT_HPP_INCLUDED
#define BMFONT_HPP_INCLUDED


#include <exception>
#include <memory>
#include <cstdint>
#include <glm/glm.hpp>
#include "texture.hpp"


namespace glsl { namespace bmfont {


  ///Exceptions thrown by the font classes.
  class FontException : public std::exception
  {
  public:
    explicit FontException(const char* msg) : std::exception(msg) {}
  };


#pragma pack(push, 4)
  struct TUS2_VS2_CUB4
  {
    TUS2_VS2_CUB4() noexcept {}
    TUS2_VS2_CUB4(glm::tvec2<uint16_t> t, glm::tvec2<int16_t> v, glm::tvec4<uint16_t> c) noexcept
      : tex(t), vertex(v), color(c)
    {}

    glm::tvec2<uint16_t>  tex;
    glm::tvec2<int16_t>   vertex;
    glm::tvec4<uint8_t>   color;
  };
#pragma pack(pop)


  class GlyphBuffer
  {
    GlyphBuffer(const char*);

  public:
    static std::unique_ptr<GlyphBuffer> Create(const char* FontFile);

    ~GlyphBuffer();
    const char* FontName() const;
    int FontSize() const;
    int LineHeight() const;
    int Base() const;
    bool is_unicode() const;
    bool is_italic() const;
    bool is_bold() const;

    void BindTexture(unsigned int Unit);
    glm::ivec2 TextureSize() const;

    int vertex_count() const;
    int primitive_count() const;
    const TUS2_VS2_CUB4* attrib_ptr() const;
    void clear();
    
    // returns the pointer to the mapped buffer
    static void* resize_gl_buffer(GLuint& vbo,
                                  GLuint& vao,
                                  size_t glyphs,
                                  GLbitfield sflags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT,
                                  GLbitfield mflags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

  private:
    friend class Text2D;
    std::unique_ptr<struct GlyphBufferImpl> pimpl_;
  };


  // Manipulators
  struct white
  {
  };


  struct color_manip
  {
    explicit color_manip(const glm::vec4 color) : val(255.f * color) {}
    explicit color_manip(const glm::tvec4<unsigned char>& color) : val(color) {}
    color_manip(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) : val(r,g,b,a) {}
    color_manip(float r, float g, float b, float a = 1.0f) : val(255*r, 255*g, 255*b, 255*a) {}
    glm::tvec4<unsigned char> val;
  };


  struct tab_manip
  {
    explicit tab_manip(int tabSize) : val(tabSize) {}
    int val;
  };


  struct line_height_manip
  {
    explicit line_height_manip(int lineHeight) : val(lineHeight) {}
    int val;
  };


  struct ofs_manip
  {
    explicit ofs_manip(const glm::ivec2& cursor, bool baseLine = true) : val(cursor), baseline(baseLine) {}
    ofs_manip(int x, int y, bool baseLine = true) : val(x, y), baseline(baseLine) {}
    glm::ivec2 val;
    bool baseline;
  };


  struct cursor_manip
  {
    explicit cursor_manip(const glm::ivec2& cursor, bool baseLine = true) : val(cursor), baseline(baseLine) {}
    cursor_manip(int x, int y, bool baseLine = true) : val(x, y), baseline(baseLine) {}
    glm::ivec2 val;
    bool baseline;
  };


  struct xpos_manip
  {
    explicit xpos_manip(int x) : val(x) {}
    int val;
  };


  struct orig_manip
  {
    explicit orig_manip(const glm::ivec2& orig) : val(orig) {}
    orig_manip(int x, int y) : val(x,y) {}
    glm::ivec2 val;
  };


  class Text2D
  {
  public:
    Text2D(GlyphBuffer* gbuf, const glm::ivec2& orig = glm::ivec2(0,0));

    const GlyphBuffer& GetGlyphBuf() const;
    GlyphBuffer& GetGlyphBuf();
    void SetGlyphBuffer(GlyphBuffer* gbuf);

    const char* FontName() const noexcept { return glyphbuf_->FontName(); }
    int FontSize() const noexcept { return glyphbuf_->FontSize(); }
    int LineHeight() const noexcept { return glyphbuf_->LineHeight(); }
    int Base() const noexcept { return glyphbuf_->Base(); }
    bool is_unicode() const noexcept { return glyphbuf_->is_unicode(); }
    bool is_italic() const noexcept { return glyphbuf_->is_italic(); }
    bool is_bold() const noexcept { return glyphbuf_->is_bold(); }

    glm::ivec2 GetCursor() const noexcept { return cursor_; }
    glm::ivec2 GetOrig() const noexcept { return orig_; }

    glm::ivec2 Extent(const char* first, const char* last, glm::ivec2& vCursor) const;
    glm::ivec2 Extent(const wchar_t* first, const wchar_t* last, glm::ivec2& vCursor) const;
    glm::ivec2 Extent(const char* str, glm::ivec2& vCursor) const;
    glm::ivec2 Extent(const wchar_t* str, glm::ivec2& vCursor) const;

    void Stream(const char* first, const char* last);
    void Stream(const wchar_t* first, const wchar_t* last);
    Text2D& operator << (const char* str);
    Text2D& operator << (const wchar_t* str);

    // Manipulators
    Text2D& operator << (const white& x) noexcept { color_ = decltype(color_)(255); return *this; }
    Text2D& operator << (const color_manip& x) noexcept { color_ = x.val; return *this; }
    Text2D& operator << (const tab_manip& x) noexcept { tabSize_ = x.val; return *this; }
    Text2D& operator << (const line_height_manip& x) noexcept { lineHeight_ = x.val; return *this; }
    Text2D& operator << (const ofs_manip& x) noexcept;
    Text2D& operator << (const cursor_manip& x) noexcept;
    Text2D& operator << (const xpos_manip& x) noexcept { cursor_.x = x.val; return *this; }
    Text2D& operator << (const orig_manip& x) noexcept;

  private:
    glm::ivec2 cursor_;
    glm::ivec2 orig_;
    int tabSize_;
    int lineHeight_;
    glm::tvec4<unsigned char> color_;
    GlyphBuffer* glyphbuf_ = nullptr;
  };


} }

#endif
