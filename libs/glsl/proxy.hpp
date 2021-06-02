/*
 * proxy.hpp
 *
 *  Created on: Aug 5, 2010
 *      Author: hanno
 */

#ifndef GLSL_PROXY_HPP_INCLUDED
#define GLSL_PROXY_HPP_INCLUDED

#include "glsl.hpp"
#include <memory>
#include <typeinfo>
#include <cassert>
#include <string>
#include <stddef.h>


namespace glsl {

  template<typename T, typename H = GLuint>
  class proxy
  {
  protected:
    proxy() : handle_((void*)0, &proxy<T,H>::null_deleter) {}
    explicit proxy(H glo) : handle_((void*)(uintptr_t)glo, &proxy<T,H>::release) {}

    void reset_() const { proxy<T,H> tmp; handle_ = tmp.handle_; }
    void reset_(H glo) const { proxy<T,H> tmp(glo); handle_ = tmp.handle_; }

    bool isValid_() const noexcept { return (0 != handle_.get()); }
    H get_() const { return do_get(handle_.get()); }

    void swap_(T& other) { handle_.swap(other.handle_); }

  private:
    mutable std::shared_ptr<void> handle_;

    static H do_get(void* p) { return (H)((char*)p - (char*)0); }
    static void release(void* p) { if (0 != p) { H x = do_get(p); T::release(x); }; }
    static void null_deleter(void* p) { assert(0 == p); }
  };

}


#endif
