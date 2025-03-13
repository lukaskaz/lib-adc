#include "adc/interfaces/rpi/ads1115/adc.hpp"

#include "sysfs/interfaces/linux/sysfs.hpp"

#include <chrono>
#include <source_location>

namespace adc::rpi::ads1115
{

using namespace sysfs::lnx;
using namespace std::chrono_literals;

struct Adc::Handler
{
  public:
    explicit Handler(const config_t& config) :
        logif{std::get<2>(config)}, channel{std::get<0>(config)},
        floatprecision{std::get<1>(config)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>({"adc", logif})}
    {
        log(logs::level::info, "Created adc ads1115 [channel/precision]: " +
                                   std::to_string(channel) + "/" +
                                   std::to_string(floatprecision));
    }

    ~Handler()
    {
        log(logs::level::info,
            "Removed adc ads1115/channel num: " + std::to_string(channel));
    }

    bool read(double& val) const
    {
        val = 0.;
        return true;
    }

  private:
    const std::shared_ptr<logs::LogIf> logif;
    const uint32_t channel;
    const uint32_t floatprecision;
    const std::shared_ptr<sysfs::SysfsIf> sysfs;

    void log(
        logs::level level, const std::string& msg,
        const std::source_location loc = std::source_location::current()) const
    {
        if (logif)
            logif->log(level, std::string{loc.function_name()}, msg);
    }
};

Adc::Adc(const config_t& config) : handler{std::make_unique<Handler>(config)}
{}
Adc::~Adc() = default;

bool Adc::read(double& val)
{
    return handler->read(val);
}

} // namespace adc::rpi::ads1115
