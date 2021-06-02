#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include "bmfont.hpp"
#include <glmutils/bbox.hpp>
#include "stb_image.h"


namespace glsl { namespace bmfont {


#pragma pack(push, 1)
  struct FileKerningPair
  {
    uint32_t first;
    uint32_t second;
    int16_t  amount;
  };

  struct FileChar
  {
    uint32_t id;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    int16_t  xoffset;
    int16_t  yoffset;
    int16_t  xadvance;
    uint8_t  page;
    uint8_t  chnl;
  };
#pragma pack(pop)


  struct CharDescr
  {
    struct hash
    {
      std::size_t operator() (const CharDescr& key) const noexcept
      {
        return std::hash<wchar_t>{}(key.id);
      }
    };

    struct key_equal
    {
      bool operator() (const CharDescr& a, const CharDescr& b) const noexcept
      {
        return a.id == b.id;
      }
    };

    CharDescr() {}
    explicit CharDescr(wchar_t Id): id(Id), xadvance(0) {}
    CharDescr(const struct FileChar& chr, int YOfs)
    :  id((wchar_t)chr.id),
      xadvance(chr.xadvance),
      TexCoord0(chr.x, chr.y),
      TexCoord1(chr.x + chr.width, chr.y + chr.height),
      Vert0(chr.xoffset, chr.yoffset - YOfs),
      Vert1(chr.xoffset + chr.width, chr.yoffset + (chr.height -YOfs))
    {}

    wchar_t id;
    uint16_t xadvance;
    glm::tvec2<uint16_t> TexCoord0, TexCoord1;      // [left-top, right-bottom] [tu, tv]
    glm::tvec2<int16_t> Vert0, Vert1;               // [left-top, right-bottom] [x, y]
  };


  struct KerningPair
  {
    struct hash
    {
      std::size_t operator() (const KerningPair& key) const noexcept
      {
        return std::hash<unsigned>{}((size_t{ key.first } << 32) | key.second);
      }
    };

    struct key_equal
    {
      bool operator() (const KerningPair& a, const KerningPair& b) const noexcept
      {
        return (a.first == b.first) && (a.second == b.second);
      }
    };

    KerningPair() {}
    KerningPair(const struct FileKerningPair& kp)
      : first((wchar_t)kp.first), second((wchar_t)kp.second), amount(kp.amount)
    {
    }

    KerningPair(int First, int Second)
      : first(First), second(Second), amount(0)
    {
    }

    wchar_t first, second;
    int16_t amount;
  };


  // BMFontLoader
  // Loader of AngleCode's BMFont binary font files.
  // s. www.AngleCode.com
  //
  struct FontLoader
  {
#pragma pack(push, 1)
    struct Info
    {
      char     header[4];
      int8_t   id;
      int32_t  size;
      int16_t  fontSize;
      int8_t   bitField;
      uint8_t  charSet;
      uint16_t stretchH;
      uint8_t  aa;
      uint8_t  paddingUp;
      uint8_t  paddingRight;
      uint8_t  paddingDown;
      uint8_t  paddingLeft;
      uint8_t  spacingHoriz;
      uint8_t  spacingVerts;
      uint8_t  outline;
    };

    struct Common
    {
      int8_t   id;
      int32_t  size;
      uint16_t lineHeight;
      uint16_t base;
      uint16_t scaleW;
      uint16_t scaleH;
      uint16_t pages;
      uint8_t  bitField;
      uint8_t  alphaChnl;
      uint8_t  redChnl;
      uint8_t  greenChnl;
      uint8_t  blueChnl;
    };
#pragma pack(pop)


    FontLoader(const char* FontFileName) 
    : InfoBlk(0), FontName(0), CommonBlk(0), 
      PageName(0), PageNameLen(0), 
      CharBlk(0), KerningBlk(0)
    {
      std::ifstream fs(FontFileName, std::ios_base::binary | std::ios_base::in);
      if (fs.fail())
      {
        throw FontException((std::string("Unable to open font file: ") + FontFileName).c_str());
      }

      // Read file into buffer
      std::streambuf* pbuf = fs.rdbuf();
      std::streamsize size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
      pbuf->pubseekpos(0, std::ios::in);
      Buffer.reset( new char[size_t(size + 16)] );
      memset((void*)Buffer.get(), '\0', size_t(size + 16));
      pbuf->sgetn(Buffer.get(), size);

      // Sanity check
      if ((size < 5) || (0 != strncmp(Buffer.get(), "BMF" "\3", 4)))
      {
        throw glsl::bmfont::FontException("BMFFontLoader: Invalid font file");
      }

      // Calculate reinterpreted pointer
      InfoBlk = (Info*)(Buffer.get());
      FontName = (char*)(InfoBlk) + sizeof(Info);
      CommonBlk = (Common*)(FontName + strlen(FontName) + 1);
      if (1 == (CommonBlk->bitField & (1 << 7)))
      {
        throw FontException("BMFFontLoader: packed format unsupported");
      }
      if (1 != (CommonBlk->pages))
      {
        throw FontException("BMFFontLoader: multiple pages unsupported");
      }
      PageName = (char*)(CommonBlk) + sizeof(Common) + 5;
      PageNameLen = strlen(PageName);
      char* Block = PageName + CommonBlk->pages * (PageNameLen + 1);
      CharCount = *(int*)(Block + 1) / sizeof(FileChar);
      CharBlk = (FileChar*)(Block + 5);
      Block = (char*)CharBlk + CharCount * sizeof(FileChar);
      KerningCount = *(int*)(Block + 1) / sizeof(FileKerningPair);
      KerningBlk = (FileKerningPair*)(Block + 5);
    }

    Info*     InfoBlk;
    char*     FontName;
    Common*   CommonBlk;
    char*     PageName;
    size_t    PageNameLen;
    int       CharCount;
    FileChar* CharBlk;
    int       KerningCount;
    FileKerningPair* KerningBlk;
    std::unique_ptr<char[]> Buffer;
  };


  struct GlyphBufferImpl
  {
    GlyphBufferImpl(const char* fontFile)
    {
      FontLoader FL(fontFile);
      std::string FontFile(fontFile);
      FontName = FL.FontName;
      FontSize = FL.InfoBlk->fontSize;
      BitField = FL.InfoBlk->bitField;
      LineHeight = FL.CommonBlk->lineHeight;
      Base = (int)FL.CommonBlk->base;
      TexSize = glm::ivec2(FL.CommonBlk->scaleW, FL.CommonBlk->scaleH);  
      int CharCount = FL.CharCount;
      for (int i=0; i<CharCount; ++i)
      {
        CharMap.emplace(FL.CharBlk[i], Base);
      }
      
      int KerningCount = FL.KerningCount;
      for (int i=0; i<FL.KerningCount; ++i)
      {
        KerningMap.emplace(FL.KerningBlk[i]);
      }

      // Load and compile OpenGL array texture
      glsl::texture tex(GL_TEXTURE_RECTANGLE);
      tex.bind();
      tex.set_wrap_filter(GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
#ifdef _WIN32
      size_t separator = FontFile.find_last_of("/\\");
#else
      size_t separator = FontFile.find_last_of('/');
#endif
      size_t len = (separator == std::string::npos) ? 0 : (separator + 1);
      std::string FontPath(FontFile.begin(), FontFile.begin() + len);
      std::vector< std::string > Page;
      for (int i=0; i < (int)FL.CommonBlk->pages; ++i)
      {
        std::string TexFile = FontPath + std::string(FL.PageName + (i * (FL.PageNameLen + 1)), FL.PageName + ((i+1) * (FL.PageNameLen + 1)));
        int w,h,c;
        stbi_uc* texData = stbi_load(TexFile.c_str(), &w, &h, &c, STBI_grey);
        if (texData)
        {
          glTexImage2D(tex.target(), 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, (void*)texData);
          tex.seti(GL_TEXTURE_MAX_LEVEL, 0);
          stbi_image_free(texData);
        }
        else
        {
          throw FontException((std::string("Unable to decode texture file: ") + TexFile + stbi_failure_reason()).c_str());
        }
      }
      tex.unbind();
      Texture.swap(tex);
    }

    int xadvance(wchar_t current, wchar_t next) const
    {
      int advance = 0;
      {
        auto it = CharMap.find(CharDescr(current));
        if (it != CharMap.end()) advance = it->xadvance;
      }
      {
        auto it = KerningMap.find(KerningPair(current, next));
        if (it != KerningMap.end()) advance += it->amount;
      }
      return advance;
    }

    bool push(wchar_t chr, const glm::ivec2& cursor, const glm::tvec4<unsigned char>& color)
    {
      auto it = CharMap.find(CharDescr(chr));
      if (it != CharMap.end())
      {
        push_attrib(*it, cursor, color);
        return true;
      }
      return false;
    }

    void push_xadvance(wchar_t chr, wchar_t next, glm::ivec2& cursor, const glm::tvec4<unsigned char>& color)
    {
      int advance = 0;
      auto it = CharMap.find(CharDescr(chr));
      if (it != CharMap.end())
      {
        push_attrib(*it, cursor, color);
        advance = it->xadvance;
        auto kit = KerningMap.find(KerningPair(chr, next));
        if (kit != KerningMap.end())
        {
          advance += kit->amount;
        }
        cursor.x += advance;
      }
    }

    unsigned int vertex_count() const noexcept
    {
      return (unsigned int)attribs_.size();
    }
    
    unsigned int primitive_count() const noexcept
    {
      return (unsigned int)attribs_.size() / 2;
    }
    
    void push_attrib(const CharDescr& chr, const glm::ivec2& cursor, const glm::tvec4<unsigned char>& color)
    {
      glm::tvec2<int16_t> i16c(cursor);
      attribs_.emplace_back(chr.TexCoord0, i16c + chr.Vert0, color);
      attribs_.emplace_back(chr.TexCoord1, i16c + chr.Vert1, color);
    }
    
    const TUS2_VS2_CUB4* attrib_ptr() const noexcept
    {
      return (attribs_.empty()) ? nullptr : attribs_.data();
    }

    void clear_attribs()
    {
      attribs_.clear();
    }
    
    glsl::texture Texture;
    std::string FontName;
    int FontSize;
    int BitField;
    int LineHeight;
    int Base;
    glm::ivec2 TexSize;
    std::unordered_set<CharDescr, CharDescr::hash, CharDescr::key_equal> CharMap;
    std::unordered_set<KerningPair, KerningPair::hash, KerningPair::key_equal> KerningMap;
    std::vector<TUS2_VS2_CUB4> attribs_;
  };

  
  GlyphBuffer::GlyphBuffer(const char* FontFile)
  {
    pimpl_.reset( new GlyphBufferImpl(FontFile) );
  }


  std::unique_ptr<GlyphBuffer> GlyphBuffer::Create(const char* FontFile)
  {
    return std::unique_ptr<GlyphBuffer>( new GlyphBuffer(FontFile) );
  }


  GlyphBuffer::~GlyphBuffer()
  {
  }


  const char* GlyphBuffer::FontName() const { return pimpl_->FontName.c_str(); }
  int GlyphBuffer::FontSize() const { return pimpl_->FontSize; }
  int GlyphBuffer::LineHeight() const { return pimpl_->LineHeight; }
  int GlyphBuffer::Base() const { return pimpl_->Base; }
  bool GlyphBuffer::is_unicode() const { return 1 == (pimpl_->BitField & (1 << 1)); }
  bool GlyphBuffer::is_italic() const { return 1 == (pimpl_->BitField & (1 << 2)); }
  bool GlyphBuffer::is_bold() const { return 1 == (pimpl_->BitField & (1 << 3)); }


  int GlyphBuffer::vertex_count() const { return pimpl_->vertex_count(); }
  int GlyphBuffer::primitive_count() const { return pimpl_->primitive_count(); }
  const TUS2_VS2_CUB4* GlyphBuffer::attrib_ptr() const { return pimpl_->attrib_ptr(); }
  void GlyphBuffer::clear() { pimpl_->clear_attribs(); }

  void GlyphBuffer::BindTexture(unsigned int Unit)
  {
    pimpl_->Texture.bind(Unit);
  }

  glm::ivec2 GlyphBuffer::TextureSize() const 
  {
    return pimpl_->TexSize;
  }


  void* GlyphBuffer::resize_gl_buffer(GLuint& vbo, GLuint& vao, size_t glyphs, GLbitfield sflags, GLbitfield mflags)
  {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferStorage(GL_ARRAY_BUFFER, glyphs * sizeof(TUS2_VS2_CUB4), nullptr, sflags);
    auto pbuf = glMapBufferRange(GL_ARRAY_BUFFER, 0, glyphs * sizeof(TUS2_VS2_CUB4), mflags);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, sizeof(TUS2_VS2_CUB4), (void*)(char*)(4));
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TUS2_VS2_CUB4), (void*)(char*)(8));
    glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(TUS2_VS2_CUB4), (void*)(char*)(0));
    return pbuf;
  }


  namespace {

    template <typename CHAR>
    glm::ivec2 Text2DExtent(const GlyphBufferImpl& impl, const CHAR* first, const CHAR* last, 
                            int tabSize, int lineHeight,
                            glm::ivec2& cursor, const glm::ivec2& orig)
    {
      glmutils::tbbox< glm::ivec2 > box(cursor);
      for (; first != last; ++first)
      {
        if (*first == '\n') 
        { 
          cursor.y += lineHeight; 
          glmutils::include(box, cursor);
          cursor.x = orig.x;
          glmutils::include(box, cursor);
        }
        else if (*first == '\t')
        {
          int tabs = cursor.x / tabSize;
          cursor.x = (tabs + 1) * tabSize;
          glmutils::include(box, cursor);
        }
        else
        {
          glmutils::include(box, cursor);
          cursor.x += impl.xadvance((wchar_t)*first, (wchar_t)*(first + 1));
          glmutils::include(box, glm::ivec2(cursor.x, cursor.y + lineHeight));
        }
      }
      return glmutils::extent(box);
    }

    template <typename CHAR>
    void Text2DStream(GlyphBufferImpl& impl, 
                      const CHAR* first,  const CHAR* last,
                      int tabSize, int lineHeight,
                      const glm::tvec4<unsigned char>& color, 
                      glm::ivec2& cursor, const glm::ivec2& orig)
    {
      for (; first != last; ++first)
      {
        if (*first == '\n') 
        { 
          cursor.x = orig.x; cursor.y += lineHeight; 
        }
        else if (*first == '\t')
        {
          int tabs = cursor.x / tabSize;
          cursor.x = (tabs + 1) * tabSize;
        }
        else
        {
          impl.push_xadvance((wchar_t)*first, (wchar_t)*(first + 1), cursor, color);
        }
      }
    }

  }


  Text2D::Text2D(GlyphBuffer* gbuf, const glm::ivec2& orig)
  : cursor_(orig), orig_(orig), 
    tabSize_(100), lineHeight_(gbuf->LineHeight()), 
    color_(255, 255, 255, 255), glyphbuf_(gbuf)
  {
  }

  void Text2D::SetGlyphBuffer(GlyphBuffer* gbuf)
  { 
    glyphbuf_ = gbuf; 
    lineHeight_ = gbuf->LineHeight();
  }

  const GlyphBuffer& Text2D::GetGlyphBuf() const
  {
    return *glyphbuf_;
  }

  GlyphBuffer& Text2D::GetGlyphBuf()
  {
    return *glyphbuf_;
  }

  glm::ivec2 Text2D::Extent(const char* first, const char* last, glm::ivec2& vCursor) const
  {
    return Text2DExtent<char>(*(glyphbuf_->pimpl_), first, last, tabSize_, lineHeight_, vCursor, orig_);
  }

  glm::ivec2 Text2D::Extent(const wchar_t* first, const wchar_t* last, glm::ivec2& vCursor) const 
  {
    return Text2DExtent<wchar_t>(*(glyphbuf_->pimpl_), first, last, tabSize_, lineHeight_, vCursor, orig_);
  }

  glm::ivec2 Text2D::Extent(const char* str, glm::ivec2& vCursor) const
  {
    return Extent(str, str + strlen(str), vCursor);
  }

  glm::ivec2 Text2D::Extent(const wchar_t* str, glm::ivec2& vCursor) const
  {
    return Extent(str, str + wcslen(str), vCursor);
  }

  void Text2D::Stream(const char* first, const char* last)
  {
    return Text2DStream<char>(*(glyphbuf_->pimpl_), first, last, tabSize_, lineHeight_, color_, cursor_, orig_);
  }

  void Text2D::Stream(const wchar_t* first, const wchar_t* last)
  {
    return Text2DStream<wchar_t>(*(glyphbuf_->pimpl_), first, last, tabSize_, lineHeight_, color_, cursor_, orig_);
  }

  Text2D& Text2D::operator << (const char* str)
  {
    Stream(str, str + strlen(str));
    return *this;
  }

  Text2D& Text2D::operator << (const wchar_t* str)
  {
    Stream(str, str + wcslen(str));
    return *this;
  }

  Text2D& Text2D::operator << (const ofs_manip& x) noexcept
  {
    cursor_ += x.val;
    if (!x.baseline) cursor_.y -= (lineHeight_ - glyphbuf_->Base());
    return *this;
  }

  Text2D& Text2D::operator << (const cursor_manip& x) noexcept
  {
    cursor_ = orig_ + x.val;
    if (!x.baseline) cursor_.y -= (lineHeight_ - glyphbuf_->Base());
    return *this;
  }

  Text2D& Text2D::operator << (const orig_manip& x)  noexcept
  { 
    orig_ = cursor_ = x.val;
    return *this; 
  }

} }

