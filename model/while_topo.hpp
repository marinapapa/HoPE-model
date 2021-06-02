#ifndef MODEL_WHILE_TOPO_HPP_INCLUDED
#define MODEL_WHILE_TOPO_HPP_INCLUDED

#include "model.hpp"

namespace model {

  template <typename Fun>
  inline size_t while_topo(const neighbor_info_view& v, size_t topo, Fun&& fun)
  {
    auto n = topo;
    for (auto it = v.cbegin(); n && (it != v.cend()); ++it) {
      if (fun(*it)) --n;
    }
    return topo - n;
  }


  template <typename Agent, typename Action>
  inline bool in_fov(Agent* self, const float nidist2, const vec_t nipos, const Action& act)
  {
    if (nidist2 != 0.0f && nidist2 < act->maxdist2)
    {
      const auto offs = torus::ofs(Simulation::WH(), self->pos, nipos);
      if (glm::dot(self->dir, offs) > glm::sqrt(nidist2) * act->cfov)
      {
        return true;
      }
    }
    return false;
  }

}

#endif
