#pragma once

#include "adc/interfaces/adc.hpp"

#include <memory>

namespace adc
{

class Factory
{
  public:
    template <typename T, typename C>
    static std::shared_ptr<AdcIf> create(const C& config)
    {
        return std::shared_ptr<T>(new T(config));
    }
};

} // namespace adc
