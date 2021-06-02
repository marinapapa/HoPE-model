/*! \file cmd_line.h
* \brief simple command line parser
* \author Hanno Hildenbrandt
*/

#ifndef CMD_LINE_H_INCLUDED
#define CMD_LINE_H_INCLUDED

#include <stdexcept>
#include <cassert>
#include <string.h>
#include <sstream>
#include <utility>
#include <string>
#include <functional>
#include <type_traits>
#include <mutex>
#include <set>
#include <vector>


//! \brief namespace of the command line parser
namespace cmd {


  //! \brief Exception type of the command line parser
  class parse_error : public std::invalid_argument
  {
  public:
    parse_error(const char* msg) : std::invalid_argument(msg)
    {}
  };


  //! \brief command line parser
  class cmd_line_parser
  {
  public:
    cmd_line_parser() 
    {}


    cmd_line_parser(const cmd_line_parser& rhs)
    {
      std::lock_guard<std::mutex> _(rhs.mutex_);
      argv_ = rhs.argv_;
      memoized_ = rhs.memoized_;
    }


    //! ctor main() interface
    cmd_line_parser(int argc, const char** argv, bool ignore_arg0 = true)
    {
      for (int i=(ignore_arg0) ? 1 : 0; i < argc; ++i) argv_.emplace_back(argv[i]);
    }


    //! merge two command line parsers
    friend cmd_line_parser merge(const cmd_line_parser& a, const cmd_line_parser& b);


    //! returns true if no arguments are given
    bool empty() const { return argv_.empty(); }


    //! ctor single white-space delimited string
    explicit cmd_line_parser(const char* cmdline)
    {
      std::string buf;
      const char* p0 = cmdline;
      for (;;)
      {
        buf.clear();
        while (isspace(*p0)) ++p0;
        if (*p0 == '\0') break;
        std::function<bool(char)> delim = [](char chr) { return 0 != isspace(chr); };
        while (!delim(*p0))
        {
          if (*p0 == '\0') break;
          if (*p0 == '\"') 
          {
            p0++; 
            delim = [](char chr) { return chr == '\"'; };
          }
          else
          {
            buf.push_back(*p0++);
          }
        }
        if (*p0) ++p0;
        argv_.push_back(buf);
      }
    }


    //! ctor single white-space delimited string
    explicit cmd_line_parser(const std::string& cmdline) : cmd_line_parser(cmdline.c_str())
    {}


    //! ctor single white-space delimited string
    explicit cmd_line_parser(std::vector<std::string>& argv) : argv_(std::move(argv))
    {}


    //! Returns true if \p name exists in argument, false otherwise
    //! \param name the name of the flag
    bool flag(const char* name) const;


    //! \brief Parse name value pair
    //! \tparam T the type to parse
    //! \param name the name
    //! \param val the value
    //! \param delim delimiter
    //! \returns true if the name-value could be read.
    //!
    //! \p val contains the parsed value on success, otherwise
    //! its unchanged.
    template <typename T>
    bool optional(const char* name, T& val, char delim = '=') const;


    //! \brief Parse name-value pair
    //! \tparam T the return type
    //! \param name the name
    //! \param delim delimiter
    //! \returns the parsed value on success.
    //!
    //! Throws parse_error on failure.
    template <typename T>
    T required(const char* name, char delim = '=') const;


    //! \brief recognize argument
    //! \param name the name of the argument
    //! \param delim delimiter
    void recognize(const char* name, char delim = '=') const;


    //! \brief Checks for unwanted arguments
    //! \return vector of superfluous argument names
    std::vector<std::string> unrecognized() const;


    //! \brief returns command line [1,capacity]
    std::string argv() const;


  private:
    std::vector<std::string> argv_;
    mutable std::mutex mutex_;
    mutable std::set<std::pair<std::string, char>> memoized_;
    void memoize(const char* name, char delim) const;
  };


  // split argument at delim
  inline std::pair<std::string, std::string> split_arg(const char* carg, char delim)
  {
    const char* s = strchr(carg, delim);
    if (nullptr == s)
    {
      return{ "", "" };
    }
    return{{carg, s}, {s + 1}};
  }


  template <typename T>
  inline void convert_arg(std::pair<std::string, std::string> const& arg, T& x)
  {
    std::istringstream iss(arg.second);
    if (!(iss >> x))
    {
      throw parse_error((std::string("invalid value for argument ") + arg.first).c_str());
    }
  }


  inline void parse_cmd_flag(const char* name, bool& val, const std::vector<std::string>& argv)
  {
    for (const auto& arg : argv) 
    {
      if (0 == strcmp(arg.c_str(), name)) { val = true; return; }
    }
  }


  template <typename T>
  inline bool parse_optional_arg(const char* name, T& val, const std::vector<std::string>& argv, char delim = '=')
  {
    for (const auto& arg : argv) 
    {
      auto sarg = split_arg(arg.c_str(), delim);
      if (sarg.first == name) { convert_arg(sarg, val); return true; }
    }
    return false;
  }


  template <typename T>
  inline void parse_required_arg(const char* name, T& val, const std::vector<std::string>& argv, char delim = '=')
  {
    for (const auto& arg : argv) 
    {
      auto sarg = split_arg(arg.c_str(), delim);
      if (sarg.first == name) { convert_arg(sarg, val); return; }
    }
    throw parse_error(((std::string("missing argument '") + name) + '\'').c_str());
  }


  inline bool cmd_line_parser::flag(const char* name) const
  {
    memoize(name, 0);
    bool flag = false;
    parse_cmd_flag(name, flag, argv_);
    return flag;
  }


  template <typename T>
  inline bool cmd_line_parser::optional(const char* name, T& val, char delim) const
  {
    memoize(name,delim);
    return parse_optional_arg(name, val, argv_, delim);
  }


  template <typename T>
  inline T cmd_line_parser::required(const char* name, char delim) const
  {
    memoize(name,delim);
    T val;
    parse_required_arg(name, val, argv_, delim);
    return val;
  }


  inline void cmd_line_parser::recognize(const char* name, char delim) const
  {
    memoize(name, delim);
  }


  inline std::vector<std::string> cmd_line_parser::unrecognized() const
  {
    std::lock_guard<std::mutex> _(mutex_);
    std::vector<std::string> tmp;
    for (size_t i=0; i<argv_.size(); ++i) 
    {
      bool known = false;
      for (const auto& a : memoized_) 
      {
        auto sarg = split_arg(argv_[i].c_str(), a.second);
        if (sarg.first == a.first)
        {
          known = true;
          break;
        }
      }
      if (!known) tmp.emplace_back(argv_[i]);
    }
    return tmp;
  }


  //! \brief returns command line [1,capacity]
  inline std::string cmd_line_parser::argv() const
  {
    std::string res;
    for (size_t i=1; i<argv_.size(); ++i)
    {
      res += argv_[i];
      res += ' ';
    }
    return res;
  }


  inline void cmd_line_parser::memoize(const char* name, char delim) const
  {
    std::lock_guard<std::mutex> _(mutex_);
    memoized_.emplace(name, delim);
  }


  //! merge two command line parsers
  inline cmd_line_parser merge(const cmd_line_parser& a, const cmd_line_parser& b)
  {
    cmd_line_parser tmp;
    {
      // lock both mutexes without deadlock
      std::lock(a.mutex_, b.mutex_);
      // make sure both already-locked mutexes are unlocked at the capacity of scope
      std::lock_guard<std::mutex> _(a.mutex_, std::adopt_lock);
      std::lock_guard<std::mutex> __(b.mutex_, std::adopt_lock);
      tmp.argv_ = a.argv_; tmp.argv_.insert(tmp.argv_.end(), b.argv_.cbegin(), b.argv_.cend());
      tmp.memoized_ = a.memoized_; tmp.memoized_.insert(b.memoized_.cbegin(), b.memoized_.cend());
    }
    return tmp;
  }


}

#endif
