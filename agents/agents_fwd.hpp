#ifndef PIGEON_AGENTS_FWD_HPP_INCLUDED
#define PIGEON_AGENTS_FWD_HPP_INCLUDED

#include <type_traits>
#include <tuple>
#include <vector>
#include <glm/glm.hpp>


namespace model {

  using tick_t = size_t;
  using vec_t = glm::vec2;
  using pos_t = glm::vec2;
  using vel_t = glm::vec2;

  // publish our species type(s) to the model core
  class Pigeon;
  class Pred;


  using pigeon_tag = std::integral_constant<size_t, 0>;
  using pred_tag = std::integral_constant<size_t, 1>;
  

  using species_pop = std::tuple<
    std::vector<Pigeon>,
    std::vector<Pred>
  >;


  template <typename Tag>
  struct known_color_maps {
    static constexpr size_t size = 0;
    static constexpr const char** descr = nullptr;
  };

  template <>
  struct known_color_maps<pigeon_tag>;

  template <>
  struct known_color_maps<pred_tag>;


  template <typename Tag>
  struct snapshot_entry;
  
  template <>
  struct snapshot_entry<pigeon_tag>;

  template <>
  struct snapshot_entry<pred_tag>;

  using species_snapshots = std::tuple<
    std::vector<snapshot_entry<pigeon_tag>>,
    std::vector<snapshot_entry<pred_tag>>
  >;

}

#endif
