// basic graph algorithms
// Hanno 2019

#ifndef GRAPH_HPP_INCLUDED
#define GRAPH_HPP_INCLUDED

#include <vector>
#include <stack>
#include <queue>
#include <tbb/tbb.h>
//#include <ppl.h>


namespace graph {


  template <typename Value>
  using component_t = std::vector<Value>;


  template <typename Value>
  using components_t = std::vector<component_t<Value>>;

  
  // calls fun for each visited vertex, including pivot
  template <typename Value, typename Visited, typename Pred, typename Fun>
  void dfs_c(Value pivot, Value begin, Value first, Value last, Visited& visited, Pred pred, Fun fun)
  {
    std::stack<Value> stack;
    stack.push(pivot);
    visited[pivot - begin] = true;
    while (!stack.empty())
    {
      auto s = stack.top();
      stack.pop();
      fun(s);
      for (auto i = first; i < last; ++i)
      {
        if (!visited[i - begin] && pred(s, i))
        {
          visited[i - begin] = true;
          stack.push(i);
        }
      }
    }
  }


  // calls fun for each visited vertex, including pivot
  template <typename Value, typename Visited, typename Pred, typename Fun>
  void bfs_c(Value pivot, Value begin, Value first, Value last, Visited& visited, Pred pred, Fun fun)
  {
    std::queue<Value> queue;
    queue.push(pivot);
    visited[pivot - begin] = true;
    while (!queue.empty())
    {
      auto s = queue.front();
      queue.pop();
      fun(s);
      for (auto i = first; i < last; ++i)
      {
        if (!visited[i - begin] && pred(s, i))
        {
          visited[i - begin] = true;
          queue.push(i);
        }
      }
    }
  }


  template <typename Value, typename Pred>
  auto connected_components(Value first, Value last, Pred pred)
  {
    components_t<Value> cc;
    std::vector<bool> visited(last - first, false);
    for (auto i = first; i < last; ++i)
    {
      if (!visited[i - first])
      {
        component_t<Value> c;
        bfs_c(i, first, i + 1, last, visited, pred, [&](auto x) { c.push_back(x); });
        cc.emplace_back(std::move(c));
      }
    }
    return cc;
  }


  // calls fun for each visited vertex, including pivot
  template <typename Value, typename Visited, typename Pred, typename Fun>
  void parallel_bfs(Value pivot, Value begin, Value first, Value last, Visited& visited, Pred pred, Fun fun)
  {
    tbb::concurrent_queue<Value> queue;
    queue.push(pivot);
    Value s;
    while (queue.try_pop(s))
    {
      fun(s);
      visited[static_cast<size_t>(pivot) - begin] = true;
      tbb::parallel_for(tbb::blocked_range<Value>(first, last, 128), [&](const auto& r)
      {
        for (auto i = r.begin(); i < r.end(); ++i)
        {
          if (!visited[static_cast<size_t>(i) - begin] && pred(s, i))
          {
            visited[static_cast<size_t>(i) - begin] = true;
            queue.push(i);
          }
        }
      }, tbb::simple_partitioner{});
    }
  }


  template <typename Value, typename Pred>
  auto parallel_connected_components(Value first, Value last, Pred pred)
  {
    components_t<Value> cc;
    std::vector<int> visited(static_cast<size_t>(last) - first, false);
    for (auto i = first; i < last - 1; ++i)
    {
      if (!visited[static_cast<size_t>(i) - first])
      {
        component_t<Value> c;
        parallel_bfs(i, first, i + 1, last, visited, pred, [&c](auto x) { c.push_back(x); });
        cc.emplace_back(std::move(c));
      }
    }
    return cc;
  }


  template <typename IT, typename IT1, typename Pred>
  bool are_connected(IT first, IT last, IT1 first1, IT1 last1, Pred pred)
  {
    bool any = false;
    for (; !any && (first != last); ++first)
    {
      for (auto it = first1; !any && (it != last1); ++it)
      {
        any = pred(*first, *it);
      }
    }
    return any;
  }


}

#endif

