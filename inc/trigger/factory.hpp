#pragma once

#include "trigger/interfaces/trigger.hpp"

#include <memory>

namespace trigger
{

class Factory
{
  public:
    template <typename T, typename C>
    static std::shared_ptr<TriggerIf> create(const C& config)
    {
        return std::shared_ptr<T>(new T(config));
    }
};

} // namespace trigger
