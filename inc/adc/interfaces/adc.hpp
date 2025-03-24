#pragma once

#include "adc/helpers.hpp"

#include <chrono>
#include <cstdint>

namespace adc
{

using AdcData = std::tuple<uint32_t, std::pair<double, int32_t>>;

class AdcIf
{
  public:
    virtual ~AdcIf() = default;
    virtual bool observe(uint32_t,
                         std::shared_ptr<helpers::Observer<AdcData>>) = 0;
    virtual bool unobserve(uint32_t,
                           std::shared_ptr<helpers::Observer<AdcData>>) = 0;
    virtual bool trigger(uint32_t) = 0;
    virtual bool read(uint32_t, double&) = 0;
    virtual bool read(uint32_t, int32_t&) = 0;
};

} // namespace adc
