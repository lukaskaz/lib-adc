#pragma once

#include "adc/factory.hpp"
#include "logs/interfaces/logs.hpp"
#include "trigger/interfaces/trigger.hpp"

#include <cstdint>
#include <tuple>
#include <vector>

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
                            std::shared_ptr<trigger::TriggerIf>,
                            std::shared_ptr<logs::LogIf>>;

class Adc : public AdcIf
{
  public:
    ~Adc();
    bool observe(std::shared_ptr<helpers::Observer<AdcData>>) override;
    bool unobserve(std::shared_ptr<helpers::Observer<AdcData>>) override;
    bool trigger(uint32_t) override;
    bool read(double&) override;
    bool read(int32_t&) override;

  private:
    friend class adc::Factory;
    explicit Adc(const config_t&);

    struct Handler;
    std::unique_ptr<Handler> handler;
};

} // namespace adc::rpi::ads1115
