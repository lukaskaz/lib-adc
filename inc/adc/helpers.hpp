#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_set>

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
