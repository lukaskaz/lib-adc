#include "adc/interfaces/rpi/ads1115/adc.hpp"

#include "sysfs/interfaces/linux/sysfs.hpp"

#include <sys/epoll.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <source_location>

namespace adc::rpi::ads1115
{

using namespace sysfs::lnx;
using namespace std::string_literals;
using namespace std::chrono_literals;

std::filesystem::path iiodevices{"/sys/bus/iio/devices"};

struct Adc::Handler : public Observable<AdcData>
{
  public:
    explicit Handler(const config_t& config) :
        logif{std::get<4>(config)}, device{std::get<0>(config)},
        reading{std::get<1>(config)}, channel{std::get<2>(config)},
        maxvalue{std::get<3>(config)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>(
            {iiodevices / device, logif})}
    {

        if (reading == readtype::trigger_oneshot)
        {
            setuptriggeros();
            useasync([this, running = running.get_token()]() {
                try
                {
                    auto devnode = std::filesystem::path{"/dev"} / device;
                    log(logs::level::info, "Iio device monitoring started");
                    auto ifs = fopen(devnode.c_str(), "r");
                    if (!ifs)
                        throw std::runtime_error("Cannot open monitored node " +
                                                 devnode.native());
                    std::vector<uint8_t> data(100);
                    while (!running.stop_requested())
                    {
                        auto epollfd = epoll_create1(0);
                        if (epollfd >= 0)
                        {
                            epoll_event event{.events = EPOLLIN,
                                              .data = {.fd = fileno(ifs)}},
                                revent{};
                            epoll_ctl(epollfd, EPOLL_CTL_ADD, fileno(ifs),
                                      &event);
                            if (0 <
                                epoll_wait(epollfd, &revent, 1,
                                           (int32_t)monitorinterval.count()))
                            {
                                if (revent.events & EPOLLIN)
                                {
                                    auto ret =
                                        ::read(fileno(ifs), &data[0], 100);
                                    if (ret == 2)
                                    {
                                        int32_t val =
                                            (((int32_t)data[1] << 8) & 0xFF00) |
                                            data[0];
                                        auto volt =
                                            (double)val * 125. / 1000000.;
                                        auto perc = (int32_t)std::min(
                                            100L, std::lround(100. * volt /
                                                              maxvalue));
                                        notify({0, volt, perc});
                                    }
                                }
                            }
                            close(epollfd);
                        }
                    }
                    fclose(ifs);
                }
                catch (const std::exception& ex)
                {
                    log(logs::level::error, ex.what());
                    throw;
                }
            });
        }

        log(logs::level::info, "Created adc ads1115 [dev/cha/max]: " + device +
                                   "/" + std::to_string(channel) + "/" +
                                   std::to_string(maxvalue));
    }

    ~Handler()
    {
        if (reading == readtype::trigger_oneshot)
        {
            running.request_stop();
            releasetriggeros();
        }
        log(logs::level::info, "Removed adc ads1115 [dev/cha]: " + device +
                                   "/" + std::to_string(channel));
    }

    bool observe([[maybe_unused]] uint32_t channel,
                 std::shared_ptr<Observer<AdcData>> obs)
    {
        subscribe(obs);
        return true;
    }

    bool unobserve([[maybe_unused]] uint32_t channel,
                   std::shared_ptr<Observer<AdcData>> obs)
    {
        unsubscribe(obs);
        return true;
    }

    bool trigger([[maybe_unused]] uint32_t channel) const
    {
        auto triggeros{"trigger" + std::to_string(trigid)};
        auto triggerpath =
            std::filesystem::path{iiodevices / triggeros / "trigger_now"};
        if (std::ofstream ofs{triggerpath}; ofs.is_open())
        {
            static const auto runtriggercmd{1u};
            ofs << runtriggercmd << std::flush;
            return true;
        }
        throw std::runtime_error("Cannot use one shot trigger: " +
                                 triggerpath.native());
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
    const std::shared_ptr<sysfs::SysfsIf> sysfs;
    const uint32_t trigid{0};
    std::future<void> async;
    std::stop_source running;
    const std::chrono::milliseconds monitorinterval{100ms};

    bool setuptriggeros()
    {
        if (std::ofstream ofs{iiodevices / "iio_sysfs_trigger/add_trigger"};
            ofs.is_open())
        {
            ofs << trigid << std::flush;
            auto triggeros{"trigger" + std::to_string(trigid)};
            if (std::ifstream ifs{iiodevices / triggeros / "name"};
                ifs.is_open())
            {
                std::string triggername;
                ifs >> triggername;
                log(logs::level::debug, "One shot trigger[" +
                                            std::to_string(trigid) +
                                            "] created: " + triggername);

                if (std::ofstream ofs{iiodevices / device /
                                      "trigger/current_trigger"};
                    ofs.is_open())
                {
                    ofs << triggername << std::flush;
                    log(logs::level::debug,
                        "One shot trigger[" + std::to_string(trigid) + "] " +
                            triggername + " set for device: " + device);

                    if (std::ofstream ofs{iiodevices / device /
                                          "buffer/length"};
                        ofs.is_open())
                    {
                        ofs << 100 << std::flush;
                        log(logs::level::debug, "Buffer length set");
                    }

                    auto scanned_channel{"in_voltage" +
                                         std::to_string(channel) + "_en"};
                    if (std::ofstream ofs{iiodevices / device /
                                          "scan_elements" / scanned_channel};
                        ofs.is_open())
                    {
                        ofs << 1 << std::flush;
                        log(logs::level::debug,
                            "Scan enabled for: " + scanned_channel);
                    }
                    if (std::ofstream ofs{iiodevices / device /
                                          "buffer/enable"};
                        ofs.is_open())
                    {
                        ofs << 1 << std::flush;
                        log(logs::level::debug, "Buffering enabled");
                    }

                    auto cmd{"../scripts/setup_trigger.sh " +
                             std::to_string(trigid)};
                    std::system(cmd.c_str());

                    return true;
                }
            }
        }
        return false;
    }

    bool releasetriggeros()
    {
        if (std::ofstream ofs{iiodevices / device / "buffer/length"};
            ofs.is_open())
        {
            ofs << 0 << std::flush;
            log(logs::level::debug, "Buffer length reset");
        }

        auto scanned_channel{"in_voltage" + std::to_string(channel) + "_en"};
        if (std::ofstream ofs{iiodevices / device / "scan_elements" /
                              scanned_channel};
            ofs.is_open())
        {
            ofs << 0 << std::flush;
            log(logs::level::debug, "Scan disabled for: " + scanned_channel);
        }

        if (std::ofstream ofs{iiodevices / device / "buffer/enable"};
            ofs.is_open())
        {
            ofs << 0 << std::flush;
            log(logs::level::debug, "Buffering disabled");
        }

        if (std::ofstream ofs{iiodevices / "iio_sysfs_trigger/remove_trigger"};
            ofs.is_open())
        {
            ofs << trigid << std::flush;
            log(logs::level::debug,
                "Trig[" + std::to_string(trigid) + "] removed");
            return true;
        }

        return false;
    }

    double getreadout() const
    {
        std::string raw, scale;
        sysfs->read("in_voltage" + std::to_string(channel) + "_raw", raw);
        sysfs->read("in_voltage" + std::to_string(channel) + "_scale", scale);

        double rawval = std::atof(raw.c_str()),
               scaleval = std::atof(scale.c_str());
        return rawval * scaleval / 1000.;
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

    void log(logs::level level, const std::string& msg,
             const std::source_location loc = std::source_location::current())
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
    return handler->trigger(channel);
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
