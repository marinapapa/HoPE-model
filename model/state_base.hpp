#ifndef MODEL_STATES_BASE_HPP_INCLUDED
#define MODEL_STATES_BASE_HPP_INCLUDED

#include "simulation.hpp"
#include "model/flight.hpp"


namespace model {
  namespace states {


    // abstract state
    template <typename Agent>
    class state
    {
    public:
      using agent_type = Agent;

      // state(size_T idx, const json& J);
      virtual ~state() = default;
      virtual void enter(agent_type* self, size_t idx, tick_t T, const Simulation& sim) = 0;
      virtual void resume(agent_type* self, size_t idx, tick_t T, const Simulation& sim) = 0;
    };


    template <typename ... States>
    class package
    {
    public:
      static constexpr size_t size = sizeof...(States);

      using package_tuple = std::tuple<States...>;
      using base_type = typename std::tuple_element_t<0, package_tuple>::base_type;
      using agent_type = typename base_type::agent_type;
      using package_array = std::array<std::unique_ptr<base_type>, size>;
      using transition_matrix = std::array<std::array<float, size>, size>;

      static package_array create(size_t idx, const json& J)
      {
        package_array a;
        //assert(J.size() == size);
        if (J.size() != size) throw std::runtime_error("Parsing error: Number of states differs in code and config  \n");
        do_create<0>::apply(a, idx, J);
        return a;
      }

    private:
      template <size_t I>
      struct do_create
      {
        template <typename Json>
        static void apply(package_array& a, size_t idx, const Json& J)
        {
          using type = std::tuple_element_t<I, package_tuple>;
          //assert(J[I]["name"] == type::name());
          if (J[I]["name"] != type::name()) throw std::runtime_error("Parsing error: Name of state differs in code (" + std::string(type::name()) + ") and config (" + std::string(J[I]["name"])+ ")  \n");
          a[I].reset(new type(idx, J[I]));
          do_create<I + 1>::apply(a, idx, J);
        }
      };

      template <>
      struct do_create<size>
      {
        template <typename J>
        static void apply(package_array&, size_t, const J&) {}
      };
    };

  }
}


#define make_state_from_this(a) \
public: \
  using agent_type = typename IP::agent_type; \
  using base_type = state<agent_type>; \
  static constexpr const char* name() { return #a; } \
protected: \
  using action_pack = IP; \
  using action_tuple = typename action_pack::package_tuple; \
  action_tuple actions; \
  float all_ws = 0.f; \
  template <size_t I> \
  void chain_actions(agent_type* self, size_t idx, tick_t T, const Simulation& sim) \
  { \
    std::get<I>(actions)(self, idx, T, sim); \
    chain_actions<I + 1>(self, idx, T, sim); \
  } \
  template <> \
  void chain_actions<action_pack::size>(agent_type*, size_t, tick_t T, const Simulation& sim) \
  { \
  } \
 template <size_t I> \
  void chain_on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim) \
  { \
    std::get<I>(actions).on_entry(self, idx, T, sim); \
    chain_on_entry<I + 1>(self, idx, T, sim); \
  } \
  template <> \
  void chain_on_entry<action_pack::size>(agent_type*, size_t, tick_t T, const Simulation& sim) \
  { \
  }\
 template <size_t I> \
  void normalize_actions() \
  { \
    all_ws += std::get<I>(actions).w_; \
    normalize_actions<I + 1>(); \
   if (all_ws > 0) { std::get<I>(actions).w_ /= all_ws; } \
  } \
  template <> \
  void normalize_actions<action_pack::size>() \
  { \
  } 

#endif
