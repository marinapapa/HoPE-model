#ifndef MODEL_ACTIONS_ACTION_BASE_HPP_INCLUDED
#define MODEL_ACTIONS_ACTION_BASE_HPP_INCLUDED

#include "simulation.hpp"


namespace model {
  namespace actions {
   /*
    *  an action shall be modeled along:
    *
    *    template <typename Agent>
    *    class action
    *    {
    *    public:
    *      using agent_type = Agent;
    *      action() = default;
    *      action(size_t idx, const json& J);
    *      void operator(agent_type* self, size_t idx, tick_t T, const Simulation& sim);
    *      void on_entry(agent_type* self, size_t idx, tick_t T, const Simulation& sim);
    *    };
    *
    */

    template <typename Agent, typename ... Actions>
    class package
    {
    public:
      static constexpr size_t size = sizeof...(Actions);
      using package_tuple = std::tuple<Actions...>;
      using agent_type = Agent;

      static package_tuple create(size_t idx, const json& J)
      {
        package_tuple t;
        //assert(J.size() == size);
        if (J.size() != size) throw std::runtime_error("Parsing error: Number of actions differs in code and config  \n");
        do_create<0>::apply(t, idx, J);
        return t;
      }

    private:
      template <size_t I>
      struct do_create
      {
        template <typename Json>
        static void apply(package_tuple& t, size_t idx, const Json& J)
        {
          using type = std::tuple_element_t<I, package_tuple>;
          //assert(J[I]["name"] == type::name());
          if (J[I]["name"] != type::name()) throw std::runtime_error("Parsing error: Name of action differs in code (" + std::string(type::name()) + ") and config (" + std::string(J[I]["name"]) + ")  \n");
          std::get<I>(t) = type(idx, J[I]);
          do_create<I + 1>::apply(t, idx, J);
        }
      };

      template <>
      struct do_create<size>
      {
        template <typename J>
        static void apply(package_tuple&, size_t, const J&) {}
      };
    };

  }
}

#define make_action_from_this(a) \
public: \
  static constexpr const char* name() { return #a; } \
  using agent_type = Agent; \
  using Tag = typename agent_type::Tag

#endif
