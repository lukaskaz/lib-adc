#pragma once

#include "adc/factory.hpp"
#include "logs/interfaces/logs.hpp"

#include <cstdint>
#include <tuple>
#include <variant>

namespace adc::rpi::ads1115
{

using configstd_t =
    std::tuple<std::string, uint32_t, double, std::shared_ptr<logs::LogIf>>;
using configtrig_t = std::tuple<std::string, uint32_t, double, double,
                                std::shared_ptr<logs::LogIf>>;
using configevt_t =
    std::tuple<std::string, uint32_t, double, std::tuple<double, double>,
               std::shared_ptr<logs::LogIf>>;
using config_t =
    std::variant<std::monostate, configstd_t, configtrig_t, configevt_t>;

class Adc : public AdcIf
{
  public:
    ~Adc();
    bool observe(std::shared_ptr<helpers::Observer<ObsData>>) override;
    bool unobserve(std::shared_ptr<helpers::Observer<ObsData>>) override;
    bool trigger() override;
    bool read(AdcData&) override;

  private:
    friend class adc::Factory;
    explicit Adc(const config_t&);

    struct Handler;
    std::unique_ptr<Handler> handler;
};

} // namespace adc::rpi::ads1115
