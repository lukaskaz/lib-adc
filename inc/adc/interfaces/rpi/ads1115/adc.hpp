#pragma once

#include "adc/factory.hpp"
#include "logs/interfaces/logs.hpp"

#include <cstdint>
#include <tuple>

namespace adc::rpi::ads1115
{

enum class readtype
{
    standard,
    event_limit,
    event_window,
    event_dataready,
    trigger_oneshot,
    trigger_periodic
};

using config_t = std::tuple<std::string, readtype, uint32_t, double,
                            std::shared_ptr<logs::LogIf>>;

class Adc : public AdcIf
{
  public:
    ~Adc();
    bool observe(uint32_t, std::shared_ptr<Observer<AdcData>>) override;
    bool unobserve(uint32_t, std::shared_ptr<Observer<AdcData>>) override;
    bool trigger(uint32_t) override;
    bool read(uint32_t, double&) override;
    bool read(uint32_t, int32_t&) override;

  private:
    friend class adc::Factory;
    explicit Adc(const config_t&);

    struct Handler;
    std::unique_ptr<Handler> handler;
};

} // namespace adc::rpi::ads1115
