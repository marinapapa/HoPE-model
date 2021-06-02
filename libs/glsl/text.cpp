#include <cctype>
#include <cstdlib>
#include <string.h>
#include <filesystem>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <glmutils/bbox.hpp>
#include "text.hpp"


namespace filesystem = std::filesystem;


namespace glsl {

  namespace {

    bool read_args_impl(const char* first, char** end, float& arg)
    {
      arg = std::strtof(first, end);
      return *end != first;
    }


    bool read_args_impl(const char* first, char** end, int& arg)
    {
      arg = static_cast<int>(std::strtol(first, end, 10));
      return *end != first;
    }


    template <typename T, typename... Tail>
    bool read_args_impl(const char* first, char** end, T& arg, Tail&... args)
    {
      auto success = read_args_impl(first, end, arg);
      return success && read_args_impl(first = *end, end, args...);
    }


    bool read_args(const char* first)
    {
      while (std::isspace(static_cast<unsigned char>(*first))) {
        ++first;
      }
      return *first == '}';
    }


    template <typename T>
    bool read_args(const char* first, T& arg)
    {
      char* end;
      if (read_args_impl(first, &end, arg)) {
        return read_args(end);
      }
      return false;
    }


    template <typename T, typename... Tail>
    bool read_args(const char* first, T& arg, Tail&... args)
    {
      char* end;
      if (read_args_impl(first, &end, arg)) {
        if (read_args_impl(first = end, &end, args...)) {
          return read_args(end);
        }
      }
      return false;
    }

    template <typename Manip, typename... Args>
    bool try_execute_command(glsl::bmfont::Text2D& text2D, const char* argf, Args&&... args)
    {
      if (read_args(argf, args...)) {
        text2D << Manip(args...);
        return true;
      }
      return false;
    }
  }


  bool Text::less::operator()(const string_view& a, const std::string& b) const
  {
    return std::lexicographical_compare(a.begin, a.end, b.begin(), b.end());
  }


  bool Text::less::operator()(const std::string& a, const string_view& b) const
  {
    return std::lexicographical_compare(a.begin(), a.end(), b.begin, b.end);
  }


  bool Text::less::operator()(const std::string& a, const std::string& b) const
  {
    return a < b;
  }


  Text::Text(GLuint prog, font_map_t* faces)
    : prog_(prog), 
      Faces_(faces), 
      text2D_(faces->begin()->second.get()),
      vertex_count_per_face_(faces->size())

  {
    resize_buffer(1024);
    // setup command functions
    command_map_["white"] = [](bmf::Text2D& text2D, const char* argf) {
      return try_execute_command<bmf::white>(text2D, argf);
    };
    command_map_["color"] = [](bmf::Text2D& text2D, const char* argf) {
      return try_execute_command<bmf::color_manip>(text2D, argf, 1.f, 1.f, 1.f);
    };
    command_map_["orig"] = [](bmf::Text2D& text2D, const char* argf) {
      return try_execute_command<bmf::orig_manip>(text2D, argf, 0, 0);
    };
    command_map_["ofs"] = [](bmf::Text2D& text2D, const char* argf) {
      return try_execute_command<bmf::ofs_manip>(text2D, argf, 0, 0);
    };
    command_map_["cursor"] = [](bmf::Text2D& text2D, const char* argf) {
      return try_execute_command<bmf::cursor_manip>(text2D, argf, 0, 0);
    };
    command_map_["tabsize"] = [](bmf::Text2D& text2D, const char* argf) {
      return try_execute_command<bmf::tab_manip>(text2D, argf, 0);
    };
    command_map_["t"] = [](bmf::Text2D& text2D, const char* argf) {
      int x = 0;
      char* end;
      if (read_args_impl(argf, &end, x))
      {
        text2D << bmf::xpos_manip(x);
        return true;
      }
      return false;
    };
    // create font-commands
    for (auto& face : *Faces_)
    {
      command_map_[face.first] = [pgb = face.second.get()](bmf::Text2D& text2D, const char*) {
        text2D.SetGlyphBuffer(pgb);
        return true;
      };
    }
  }


  Text::~Text()
  {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
  }


  void Text::Flush()
  {
    totVertexCount_ = 0;
    for (auto const & face : *Faces_)
    {
      totVertexCount_ += face.second->vertex_count();
    }
    if (totVertexCount_)
    {
      if (totVertexCount_ > buf_size_) 
      {
        resize_buffer((110 * totVertexCount_) / 100);
      }
      size_t i = 0;
      unsigned tvc = 0;
      unsigned stride = sizeof(bmf::TUS2_VS2_CUB4);
      for (auto & face : *Faces_)
      {
        unsigned vc = face.second->vertex_count();
        if (vc)
        {
          std::memcpy((char*)pbuf_ + stride * tvc, face.second->attrib_ptr(), stride * vc);
        }
        vertex_count_per_face_[i++] = vc;
        tvc += vc;
        face.second->clear();
      }
    }
  }


  void Text::resize_buffer(size_t n)
  {
    pbuf_ = bmf::GlyphBuffer::resize_gl_buffer(vbo_, vao_, n);
    buf_size_ = n;
  }


  void Text::Render()
  {
    if (totVertexCount_)
    {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glUseProgram(prog_);
      glBindVertexArray(vao_);
      unsigned tvc = 0;
      size_t i = 0;
      for (auto & face : *Faces_)
      {
        if (vertex_count_per_face_[i]) { 
          face.second->BindTexture(2); 
          glDrawArrays(GL_LINES, tvc, vertex_count_per_face_[i]); 
        }
        tvc += vertex_count_per_face_[i];
        ++i;
      }
      glDisable(GL_BLEND);
      glBindVertexArray(0);

    }
  }


  glm::ivec2 Text::extent(string_view sv)
  {
    bmf::Text2D tmp(text2D_);
    tmp << bmf::orig_manip(0, 0);
    tmp << bmf::cursor_manip(0, 0);
    glm::ivec2 tmpCursor = tmp.GetCursor();
    glmutils::tbbox<glm::ivec2> box(glm::ivec2(0));
    while (sv)
    {
      const char* pivot = sv.begin;
      for (; sv && (*sv.begin != '\\'); ++sv.begin) {}
      glm::ivec2 ext = tmp.Extent(pivot, sv.begin, tmpCursor);
      glmutils::include(box, tmpCursor);
      glmutils::include(box, ext);
      sv.begin = parse_commands(tmp, sv);
      if (*sv == '\\')
      {
        glm::ivec2 ext = tmp.Extent(sv.begin, ++sv.begin + 1, tmpCursor);
        glmutils::include(box, tmpCursor);
        glmutils::include(box, ext);
      }
    }
    return glmutils::extent(box);
  }


  void Text::print(string_view sv)
  {
    while (sv)
    {
      const char* pivot = sv.begin;
      for (; sv && (*sv != '\\'); ++sv) {}
      text2D_.Stream(pivot, sv.begin);
      sv.begin = parse_commands(text2D_, sv);
      if (*sv == '\\')
      { // no command but special character
        text2D_.Stream(sv.begin, ++sv.begin);
      }
    }
  }


  bool Text::match_command(bmf::Text2D& txt, string_view command, string_view arg)
  {
    auto it = command_map_.find(command);
    if (it != command_map_.end())
    {
      return it->second(txt, arg.begin);
    }
    return false;
  }


  const char* Text::parse_commands(bmf::Text2D& txt, string_view command)
  {
  parse_next_command:
    if (command && *command == '\\')
    {
      for (auto cmd(command); cmd; ++cmd)
      {
        if (*cmd == '{')
        {
          for (auto arg(cmd++); arg; ++arg)
          {
            if (*arg == '}')
            {
              if (match_command(txt, { command.begin + 1, cmd.begin }, { cmd.begin + 1, arg.begin }))
              {
                command.begin = arg.begin + 1;
                goto parse_next_command;
              }
            }
          }
        }
      }
    }
    return command.begin;
  }

}

