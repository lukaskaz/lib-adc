#pragma once

#include "adc/helpers.hpp"

#include <chrono>
#include <cstdint>

namespace adc
{

using AdcData = std::pair<double, int32_t>;
using ObsData = std::tuple<uint32_t, AdcData>;

class AdcIf
{
  public:
    virtual ~AdcIf() = default;
    virtual bool observe(std::shared_ptr<helpers::Observer<ObsData>>) = 0;
    virtual bool unobserve(std::shared_ptr<helpers::Observer<ObsData>>) = 0;
    virtual bool trigger() = 0;
    virtual bool read(AdcData&) = 0;
};

} // namespace adc
