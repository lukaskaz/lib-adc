#include "adc/interfaces/rpi/ads1115/adc.hpp"

#include "sysfs/helpers.hpp"
#include "sysfs/interfaces/linux/sysfs.hpp"

#include <sys/epoll.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <optional>
#include <source_location>

namespace adc::rpi::ads1115
{

using namespace sysfs;
using namespace sysfs::lnx;
using namespace std::string_literals;
using namespace std::chrono_literals;

std::filesystem::path iiodevices{"/sys/bus/iio/devices"};

struct Adc::Handler : public Observable<AdcData>
{
  public:
    explicit Handler(const config_t& config) :
        logif{std::get<5>(config)}, device{std::get<0>(config)},
        reading{std::get<1>(config)}, channel{std::get<2>(config)},
        maxvalue{std::get<3>(config)}, trigger{std::get<4>(config)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>(
            {iiodevices / device, logif})}
    {

        if (reading == readtype::trigger_oneshot ||
            reading == readtype::trigger_periodic)
        {
            setuptrigmon();
            runtrigmon();
        }

        log(logs::level::info, "Created adc ads1115 [dev/cha/max]: " + device +
                                   "/" + std::to_string(channel) + "/" +
                                   std::to_string(maxvalue));
    }

    ~Handler()
    {
        if (reading == readtype::trigger_oneshot ||
            reading == readtype::trigger_periodic)
        {
            running.request_stop();
            releasetrigmon();
        }
        log(logs::level::info, "Removed adc ads1115 [dev/cha]: " + device +
                                   "/" + std::to_string(channel));
    }

    bool observe([[maybe_unused]] uint32_t channel,
                 std::shared_ptr<Observer<AdcData>> obs)
    {
        std::ostringstream oss;
        oss << std::hex << obs.get();
        log(logs::level::debug, "Adding observer "s + oss.str() + " for cha " +
                                    std::to_string(channel));
        subscribe(obs);
        return true;
    }

    bool unobserve([[maybe_unused]] uint32_t channel,
                   std::shared_ptr<Observer<AdcData>> obs)
    {
        std::ostringstream oss;
        oss << std::hex << obs.get();
        log(logs::level::debug, "Removing observer "s + oss.str() +
                                    " for cha " + std::to_string(channel));
        unsubscribe(obs);
        return true;
    }

    bool runtrigger([[maybe_unused]] uint32_t channel) const
    {
        return trigger->run();
    }

    bool read(uint32_t channel, double& val)
    {
        val = getreadout();
        log(logs::level::debug, "Cha[" + std::to_string(channel) +
                                    "] value read: " + std::to_string(val));
        return true;
    }

    bool read(uint32_t channel, int32_t& perc)
    {
        perc = getpercent();
        log(logs::level::debug, "Cha[" + std::to_string(channel) +
                                    "] percent read: " + std::to_string(perc));
        return true;
    }

  private:
    const std::shared_ptr<logs::LogIf> logif;
    const std::string device;
    const readtype reading;
    const uint32_t channel;
    const double maxvalue;
    std::shared_ptr<trigger::TriggerIf> trigger;
    const std::shared_ptr<sysfs::SysfsIf> sysfs;
    const uint32_t trigid{0};
    std::future<void> async;
    std::stop_source running;
    const std::chrono::milliseconds monitorinterval{100ms};

    bool setuptrigmon()
    {
        std::string triggername;
        trigger->name(triggername);
        sysfs->elevwrite(iiodevices / device / "trigger/current_trigger",
                         triggername);
        sysfs->elevwrite(iiodevices / device / "buffer/length", str(100));
        auto scanned_item{"in_voltage" + std::to_string(channel) + "_en"};
        sysfs->elevwrite(iiodevices / device / "scan_elements" / scanned_item,
                         str(1));
        sysfs->elevwrite(iiodevices / device / "buffer/enable", str(1));
        return true;
    }

    bool releasetrigmon() const
    {
        sysfs->elevwrite(iiodevices / device / "trigger/current_trigger", {});
        sysfs->elevwrite(iiodevices / device / "buffer/length", str(0));
        auto scanned_item{"in_voltage" + std::to_string(channel) + "_en"};
        sysfs->elevwrite(iiodevices / device / "scan_elements" / scanned_item,
                         str(0));
        sysfs->elevwrite(iiodevices / device / "buffer/enable", str(0));
        return true;
    }

    bool runtrigmon()
    {
        return useasync([this, running = running.get_token()]() {
            try
            {
                auto devnode = std::filesystem::path{"/dev"} / device;
                sysfs->elevate(devnode, "o", "r");
                log(logs::level::info, "Iio device monitoring started");
                auto ifs = fopen(devnode.c_str(), "r");
                if (!ifs)
                    throw std::runtime_error(
                        "Cannot open node of monitored device: " +
                        devnode.native());
                auto fd = fileno(ifs);
                std::vector<uint8_t> data(100);
                auto timeout = (int32_t)monitorinterval.count();
                while (!running.stop_requested())
                    if (auto num = getdevdata(fd, data, timeout); num > 0)
                        if (auto values = extractvalues(num, data))
                            notifyclients(0, *values);
                fclose(ifs);
            }
            catch (const std::exception& ex)
            {
                log(logs::level::error, ex.what());
                throw;
            }
        });
    }

    ssize_t getdevdata(int32_t devfd, std::vector<uint8_t>& data,
                       int32_t waittimems) const
    {
        ssize_t readbytes{};
        auto epollfd = epoll_create1(0);
        if (epollfd >= 0)
        {
            epoll_event event{.events = EPOLLIN, .data = {.fd = devfd}},
                revent{};
            epoll_ctl(epollfd, EPOLL_CTL_ADD, devfd, &event);
            if (0 < epoll_wait(epollfd, &revent, 1, waittimems))
                if (revent.events & EPOLLIN)
                {
                    readbytes = ::read(devfd, &data[0], data.size());
                    log(logs::level::debug,
                        "Received bytes num: " + std::to_string(readbytes));
                }
            close(epollfd);
        }
        return readbytes;
    }

    bool notifyclients(uint32_t channel, const std::pair<double, int32_t> data)
    {
        bool ret{};
        if ((ret = notify({channel, data})))
            log(logs::level::debug,
                "Cha[" + std::to_string(channel) + "] clients notified");
        else
            log(logs::level::warning,
                "Cha[" + std::to_string(channel) + "] cannot notify clients");
        return ret;
    }

    std::optional<std::pair<double, int32_t>>
        extractvalues(ssize_t readbytes, const std::vector<uint8_t>& data) const
    {
        if (readbytes >= 2)
        {
            auto val = (int16_t)((((int16_t)data[1] << 8) & 0xFF00) | data[0]);
            auto volt =
                std::round(100. * std::max(0., (double)val) * 125. / 1000000.) /
                100.;
            auto perc =
                (int32_t)std::min(100L, std::lround(100. * volt / maxvalue));
            return std::make_pair(volt, perc);
        }
        return {};
    }

    double getreadout() const
    {
        std::string raw, scale;
        sysfs->read("in_voltage" + std::to_string(channel) + "_raw", raw);
        sysfs->read("in_voltage" + std::to_string(channel) + "_scale", scale);

        double rawval = std::max(0., std::atof(raw.c_str())),
               scaleval = std::atof(scale.c_str());
        return std::round(100. * rawval * scaleval / 1000.) / 100.;
    }

    int32_t getpercent() const
    {
        return (int32_t)std::min(100L,
                                 std::lround(100. * getreadout() / maxvalue));
    }

    bool useasync(std::function<void()>&& func)
    {
        if (async.valid())
            async.wait();
        async = std::async(std::launch::async, std::move(func));
        return true;
    };

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

bool Adc::observe(uint32_t channel, std::shared_ptr<Observer<AdcData>> obs)
{
    return handler->observe(channel, obs);
}

bool Adc::unobserve(uint32_t channel, std::shared_ptr<Observer<AdcData>> obs)
{
    return handler->unobserve(channel, obs);
}

bool Adc::trigger(uint32_t channel)
{
    return handler->runtrigger(channel);
}

bool Adc::read(uint32_t channel, double& val)
{
    return handler->read(channel, val);
}

bool Adc::read(uint32_t channel, int32_t& val)
{
    return handler->read(channel, val);
}

} // namespace adc::rpi::ads1115
