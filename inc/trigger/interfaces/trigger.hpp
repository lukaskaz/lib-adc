#pragma once

#include "adc/helpers.hpp"

#include <cstdint>
#include <string>

namespace trigger
{

class TriggerIf
{
  public:
    virtual ~TriggerIf() = default;
    virtual bool name(std::string&) = 0;
    virtual bool run() = 0;
};

} // namespace trigger
