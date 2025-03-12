#pragma once

#include <chrono>
#include <cstdint>

namespace adc
{

class AdcIf
{
  public:
    virtual ~AdcIf() = default;
    virtual bool read(double&) = 0;
};

} // namespace adc
