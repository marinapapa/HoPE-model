#ifndef MODEL_MODEL_HPP_INCLUDED
#define MODEL_MODEL_HPP_INCLUDED

#include <tuple>
#include <array>
#include <memory>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <rndutils.hpp>
#include AGENTS_CONFIG_FWD


namespace model {

  extern thread_local rndutils::default_engine reng;
  static constexpr size_t n_species = std::tuple_size_v<species_pop>;


  struct neighbor_info
  {
    float dist2;      // distance square
    unsigned idx;     // index of neighbor
    float bangl;       // angle from focal individual
  };


  class neighbor_info_view
  {
  public:
    neighbor_info_view() : first_(nullptr), n_(0) {}
    neighbor_info_view(const neighbor_info* first, size_t n) : first_(first), n_(n) {}

    const neighbor_info* begin() const noexcept { return first_; }
    const neighbor_info* end() const noexcept { return first_ + n_; }
    const neighbor_info* cbegin() const noexcept { return first_; }
    const neighbor_info* cend() const noexcept { return first_ + n_; }
    const neighbor_info& operator[](size_t i) const { 
      assert(i < n_);
      return *(first_ + i); 
    }
    bool empty() const noexcept { return n_ == 0; }
    size_t size() const noexcept { return n_; }

  private:
    const neighbor_info* first_;
    const size_t n_;
  };

  
  // OpenGL stuff
#pragma pack(push, 16)
  struct instance_proxy
  {
    vec_t pos;
    vec_t vel;
    vec_t side;
    float tex;
    float alpha;
  };
#pragma pack(pop)

}

#endif
