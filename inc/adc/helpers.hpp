#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_set>

namespace helpers
{

template <typename T>
class Observer
{
  public:
    using Func = std::function<void(const T&)>;
    static std::shared_ptr<Observer<T>> create(const Func& func)
    {
        return std::shared_ptr<Observer<T>>(new Observer<T>(func));
    }

    void operator()(const T& param)
    {
        func(param);
    }

  private:
    Observer(const Func& func) : func{func}
    {}
    Func func;
};

template <typename T>
class Observable
{
  public:
    bool notify(const T& param)
    {
        if (!observers.empty())
        {
            std::ranges::for_each(observers,
                                  [&param](auto obs) { (*obs)(param); });
            return true;
        }
        return false;
    }

    void subscribe(std::shared_ptr<Observer<T>> obs)
    {
        if (!observers.insert(obs).second)
        {
            throw std::runtime_error(
                "Trying to subscribe already existing observer");
        }
    }

    void unsubscribe(std::shared_ptr<Observer<T>> obs)
    {
        if (!observers.erase(obs))
        {
            throw std::runtime_error(
                "Trying to unsubscribe not existing observer");
        }
    }

  private:
    std::unordered_set<std::shared_ptr<Observer<T>>> observers;
};

namespace tr
{
// tail recursion variant
template <class T>
requires requires(T a)
{
    a* a;
}
inline auto pow(T base, uint32_t exp, T res = 1) -> T
{
    if (!exp)
        return res;
    if (!base)
        return 0;
    return pow(base, exp - 1, res * base);
}
} // namespace tr

} // namespace helpers
