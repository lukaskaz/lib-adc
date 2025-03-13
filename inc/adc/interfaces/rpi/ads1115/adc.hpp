#pragma once

#include "adc/factory.hpp"
#include "logs/interfaces/logs.hpp"

#include <cstdint>
#include <tuple>

namespace adc::rpi::ads1115
{

using config_t = std::tuple<uint32_t, uint32_t, std::shared_ptr<logs::LogIf>>;

class Adc : public AdcIf
{
  public:
    ~Adc();
    bool read(double&) override;

  private:
    friend class adc::Factory;
    explicit Adc(const config_t&);

    struct Handler;
    std::unique_ptr<Handler> handler;
};

} // namespace adc::rpi::ads1115
