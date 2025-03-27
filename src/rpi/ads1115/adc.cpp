#include "adc/interfaces/rpi/ads1115/adc.hpp"

#include "sysfs/helpers.hpp"
#include "sysfs/interfaces/linux/sysfs.hpp"
#include "trigger/interfaces/linux/oneshot/trigger.hpp"
#include "trigger/interfaces/linux/periodic/trigger.hpp"

#include <linux/iio/events.h>
#include <linux/iio/types.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <optional>
#include <source_location>
#include <unordered_map>

namespace adc::rpi::ads1115
{

using namespace sysfs;
using namespace sysfs::lnx;
using namespace std::string_literals;
using namespace std::chrono_literals;

std::filesystem::path iiodevices{"/sys/bus/iio/devices"};
constexpr uint32_t maxchannels{4};
constexpr uint32_t triggeredbytes{sizeof(int16_t)};
constexpr uint32_t eventbytes{sizeof(iio_event_data)};

enum class readtype
{
    standard,
    trigger,
    event,
};

static const std::unordered_map<iio_chan_type, std::string> iiochantype = {
    {IIO_VOLTAGE, "voltage"},
    {IIO_CURRENT, "current"},
    {IIO_POWER, "power"},
    {IIO_ACCEL, "accel"},
    {IIO_ANGL_VEL, "anglvel"},
    {IIO_MAGN, "magn"},
    {IIO_LIGHT, "illuminance"},
    {IIO_INTENSITY, "intensity"},
    {IIO_PROXIMITY, "proximity"},
    {IIO_TEMP, "temp"},
    {IIO_INCLI, "incli"},
    {IIO_ROT, "rot"},
    {IIO_ANGL, "angl"},
    {IIO_TIMESTAMP, "timestamp"},
    {IIO_CAPACITANCE, "capacitance"},
    {IIO_ALTVOLTAGE, "altvoltage"},
    {IIO_CCT, "cct"},
    {IIO_PRESSURE, "pressure"},
    {IIO_HUMIDITYRELATIVE, "humidityrelative"},
    {IIO_ACTIVITY, "activity"},
    {IIO_STEPS, "steps"},
    {IIO_ENERGY, "energy"},
    {IIO_DISTANCE, "distance"},
    {IIO_VELOCITY, "velocity"},
    {IIO_CONCENTRATION, "concentration"},
    {IIO_RESISTANCE, "resistance"},
    {IIO_PH, "ph"},
    {IIO_UVINDEX, "uvindex"},
    {IIO_GRAVITY, "gravity"},
    {IIO_POSITIONRELATIVE, "positionrelative"},
    {IIO_PHASE, "phase"},
    {IIO_MASSCONCENTRATION, "massconcentration"},
};

static const std::unordered_map<iio_event_type, std::string> iioevtype = {
    {IIO_EV_TYPE_THRESH, "thresh"},
    {IIO_EV_TYPE_MAG, "mag"},
    {IIO_EV_TYPE_ROC, "roc"},
    {IIO_EV_TYPE_THRESH_ADAPTIVE, "thresh_adaptive"},
    {IIO_EV_TYPE_MAG_ADAPTIVE, "mag_adaptive"},
    {IIO_EV_TYPE_CHANGE, "change"},
    {IIO_EV_TYPE_MAG_REFERENCED, "mag_referenced"},
    {IIO_EV_TYPE_GESTURE, "gesture"},
};

static const std::unordered_map<iio_event_direction, std::string> iioevdir = {
    {IIO_EV_DIR_EITHER, "either"},       {IIO_EV_DIR_RISING, "rising"},
    {IIO_EV_DIR_FALLING, "falling"},     {IIO_EV_DIR_SINGLETAP, "singletap"},
    {IIO_EV_DIR_DOUBLETAP, "doubletap"},
};

struct Adc::Handler : public helpers::Observable<ObsData>
{
  public:
    explicit Handler(const configstd_t& config) :
        device{std::get<0>(config)}, logif{std::get<3>(config)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>(
            {iiodevices / device, logif})},
        channel{std::get<1>(config)}, scale{[this]() {
            std::string scale;
            sysfs->read("in_voltage" + str(channel) + "_scale", scale);
            return std::atof(scale.c_str());
        }()},
        maxvalue{std::get<2>(config)}, reading{readtype::standard}
    {
        log(logs::level::info,
            "Created standard adc ads1115 [dev/cha/max]: " + device + "/" +
                str(channel) + "/" + str(maxvalue));
    }

    explicit Handler(const configtrig_t& config) :
        device{std::get<0>(config)}, logif{std::get<4>(config)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>(
            {iiodevices / device, logif})},
        trigger{[this](auto freq) {
            if (!freq)
                return trigger::Factory::create<
                    trigger::lnx::oneshot::Trigger,
                    trigger::lnx::oneshot::config_t>({logif});
            else
                return trigger::Factory::create<
                    trigger::lnx::periodic::Trigger,
                    trigger::lnx::periodic::config_t>({freq, logif});
        }(std::get<3>(config))},
        channel{std::get<1>(config)}, scale{[this]() {
            std::string scale;
            sysfs->read("in_voltage" + str(channel) + "_scale", scale);
            return std::atof(scale.c_str());
        }()},
        maxvalue{std::get<2>(config)}, reading{readtype::trigger}
    {
        triggersetup();
        triggermonitoring();

        std::string name;
        trigger->name(name);
        log(logs::level::info,
            "Created triggered adc ads1115 [dev/cha/trig/max]: " + device +
                "/" + str(channel) + "/" + name + "/" + str(maxvalue));
    }

    explicit Handler(const configevt_t& config) :
        device{std::get<0>(config)}, logif{std::get<4>(config)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>(
            {iiodevices / device, logif})},
        channel{std::get<1>(config)}, scale{[this]() {
            std::string scale;
            sysfs->read("in_voltage" + str(channel) + "_scale", scale);
            return std::atof(scale.c_str());
        }()},
        maxvalue{std::get<2>(config)}, reading{readtype::event}
    {
        auto [fallthd, risethd] = std::get<3>(config);
        std::string name;
        if (!fallthd && !risethd)
        {
            name = "data ready";
            eventdatareadysetup();
        }
        else if (fallthd && risethd)
        {
            name = "window";
            eventwindowsetup(fallthd, risethd);
        }
        else
        {
            name = "limit";
            auto thd = fallthd ? fallthd : risethd;
            eventlimitsetup(thd);
        }
        eventmonitoring();
        log(logs::level::info,
            "Created " + name +
                " events driven adc ads1115 [dev/cha/fall/rise/max]: " +
                device + "/" + str(channel) + "/" + str(fallthd) + "/" +
                str(risethd) + "/" + str(maxvalue));
    }

    ~Handler()
    {
        std::string name;
        switch (reading)
        {
            case readtype::standard:
                name = "standard";
                break;
            case readtype::trigger:
                name = "triggered";
                running.request_stop();
                triggerrelease();
                break;
            case readtype::event:
                name = "events driven";
                running.request_stop();
                eventrelease();
                break;
        }
        log(logs::level::info, "Removed " + name + " adc ads1115 [dev/cha]: " +
                                   device + "/" + str(channel));
    }

    bool observe(std::shared_ptr<helpers::Observer<ObsData>> obs)
    {
        std::ostringstream oss;
        oss << std::hex << obs.get();
        log(logs::level::debug, "Adding observer " + oss.str() + " for cha " +
                                    str(channel) + " and notifying client");
        subscribe(obs);
        return true;
    }

    bool unobserve(std::shared_ptr<helpers::Observer<ObsData>> obs)
    {
        std::ostringstream oss;
        oss << std::hex << obs.get();
        log(logs::level::debug,
            "Removing observer " + oss.str() + " for cha " + str(channel));
        unsubscribe(obs);
        return true;
    }

    bool runtrigger() const
    {
        return trigger->run();
    }

    bool read(AdcData& data)
    {
        auto volt{getvoltage()};
        data = {volt, getpercent(volt)};
        log(logs::level::debug, "Cha[" + str(channel) +
                                    "] adc read: " + str(std::get<0>(data)) +
                                    "/" + str(std::get<1>(data)));
        return true;
    }

  private:
    const std::string device;
    const std::shared_ptr<logs::LogIf> logif;
    const std::shared_ptr<sysfs::SysfsIf> sysfs;
    const std::shared_ptr<trigger::TriggerIf> trigger{};
    const uint32_t channel;
    const double scale;
    const double maxvalue;
    const readtype reading;
    std::future<void> async;
    std::stop_source running;
    const std::chrono::milliseconds monitorinterval{100ms};

    bool triggersetup() const
    {
        std::string triggername;
        trigger->name(triggername);
        sysfs->elevwrite(iiodevices / device / "trigger/current_trigger",
                         triggername);
        bufferenable();
        return true;
    }

    bool triggerrelease() const
    {
        bufferdisable();
        sysfs->elevwrite(iiodevices / device / "trigger/current_trigger", " ");
        return true;
    }

    bool bufferenable() const
    {
        sysfs->elevwrite(iiodevices / device / "buffer/length", str(100));
        auto scanned_item{"in_voltage" + str(channel) + "_en"};
        sysfs->elevwrite(iiodevices / device / "scan_elements" / scanned_item,
                         str(1));
        sysfs->elevwrite(iiodevices / device / "scan_elements/in_timestamp_en",
                         str(0));
        sysfs->elevwrite(iiodevices / device / "buffer/enable", str(1));
        return true;
    }

    bool bufferdisable() const
    {
        sysfs->elevwrite(iiodevices / device / "buffer/enable", str(0));
        auto scanned_item{"in_voltage" + str(channel) + "_en"};
        sysfs->elevwrite(iiodevices / device / "scan_elements" / scanned_item,
                         str(0));
        sysfs->elevwrite(iiodevices / device / "buffer/length", str(0));
        return true;
    }

    bool eventwindowsetup(double fallthresh, double risethresh) const
    {
        auto fallingvalue{"in_voltage" + str(channel) +
                          "_thresh_falling_value"};
        sysfs->elevwrite(iiodevices / device / "events" / fallingvalue,
                         str(getrawfromvolt(fallthresh)));

        auto risingvalue{"in_voltage" + str(channel) + "_thresh_rising_value"};
        sysfs->elevwrite(iiodevices / device / "events" / risingvalue,
                         str(getrawfromvolt(risethresh)));

        auto enablevalue{"in_voltage" + str(channel) + "_thresh_either_en"};
        sysfs->elevwrite(iiodevices / device / "events" / enablevalue, str(1));
        return true;
    }

    bool eventdatareadysetup() const
    {
        auto fallingvalue{"in_voltage" + str(channel) +
                          "_thresh_falling_value"};
        sysfs->elevwrite(iiodevices / device / "events" / fallingvalue,
                         str(0xFFFF));

        auto risingvalue{"in_voltage" + str(channel) + "_thresh_rising_value"};
        sysfs->elevwrite(iiodevices / device / "events" / risingvalue,
                         str(0x0000));

        auto enablevalue{"in_voltage" + str(channel) + "_thresh_either_en"};
        sysfs->elevwrite(iiodevices / device / "events" / enablevalue, str(1));
        return true;
    }

    bool eventlimitsetup(double thresh) const
    {
        auto risingvalue{"in_voltage" + str(channel) + "_thresh_rising_value"};
        sysfs->elevwrite(iiodevices / device / "events" / risingvalue,
                         str(getrawfromvolt(thresh)));

        auto enablevalue{"in_voltage" + str(channel) + "_thresh_rising_en"};
        sysfs->elevwrite(iiodevices / device / "events" / enablevalue, str(1));
        return true;
    }

    bool eventrelease() const
    {
        auto fallingvalue{"in_voltage" + str(channel) +
                          "_thresh_falling_value"};
        sysfs->elevwrite(iiodevices / device / "events" / fallingvalue,
                         str(-32768));
        auto risingvalue{"in_voltage" + str(channel) + "_thresh_rising_value"};
        sysfs->elevwrite(iiodevices / device / "events" / risingvalue,
                         str(32767));

        auto enablevalue = "in_voltage" + str(channel) + "_thresh_rising_en";
        sysfs->elevwrite(iiodevices / device / "events" / enablevalue, str(0));

        enablevalue = "in_voltage" + str(channel) + "_thresh_either_en";
        sysfs->elevwrite(iiodevices / device / "events" / enablevalue, str(0));
        return true;
    }

    bool triggermonitoring()
    {
        return useasync([this, running = running.get_token()]() {
            try
            {
                auto devnode = std::filesystem::path{"/dev"} / device;
                sysfs->elevate(devnode, "o", "r");
                log(logs::level::info, "Trigger monitoring started");
                auto ifs = fopen(devnode.c_str(), "r");
                if (!ifs)
                    throw std::runtime_error(
                        "Cannot open node of monitored device: " +
                        devnode.native());
                auto fd = fileno(ifs);
                std::vector<uint8_t> data(100);
                auto timeout = (int32_t)monitorinterval.count();
                double previous{};
                while (!running.stop_requested())
                    if (auto num = getdevicedata(fd, data, timeout); num > 0)
                        if (auto values = extractdevicevalues(num, data))
                            if (auto current = std::get<double>(*values);
                                ischanged(current, previous, 0.025))
                            {
                                previous = current;
                                notifyclients(channel, *values);
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

    bool eventmonitoring()
    {
        return useasync([this, running = running.get_token()]() {
            try
            {
                auto devnode = std::filesystem::path{"/dev"} / device;
                sysfs->elevate(devnode, "o", "r");
                log(logs::level::info, "Event monitoring started");
                auto ifs = fopen(devnode.c_str(), "r");
                if (!ifs)
                    throw std::runtime_error(
                        "Cannot open node of monitored device: " +
                        devnode.native());
                auto fd = fileno(ifs), evfd{-1};
                ioctl(fd, IIO_GET_EVENT_FD_IOCTL, &evfd);

                iio_event_data event;
                auto timeout = (int32_t)monitorinterval.count();
                double previous{};
                while (!running.stop_requested())
                    if (auto num = geteventdata(evfd, event, timeout); num > 0)
                        if (auto values = extracteventvalues(num, event))
                            if (auto current = std::get<double>(*values);
                                ischanged(current, previous, 0.025))
                            {
                                previous = current;
                                notifyclients(channel, *values);
                            }
                close(evfd);
                fclose(ifs);
            }
            catch (const std::exception& ex)
            {
                log(logs::level::error, ex.what());
                throw;
            }
        });
    }

    bool ischanged(double first, double second, double deadband)
    {
        return std::fabs(first - second) > deadband;
    }

    ssize_t getdevicedata(int32_t fd, std::vector<uint8_t>& data,
                          int32_t waittimems) const
    {
        ssize_t readbytes{};
        if (auto epollfd = epoll_create1(0); epollfd >= 0)
        {
            epoll_event event{.events = EPOLLIN, .data = {.fd = fd}}, revent{};
            epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
            if (0 < epoll_wait(epollfd, &revent, 1, waittimems) &&
                (revent.events & EPOLLIN))
            {
                readbytes = ::read(fd, &data[0], data.size());
                log(logs::level::debug,
                    "Received bytes num: " + str(readbytes));
            }
            close(epollfd);
        }
        return readbytes;
    }

    ssize_t geteventdata(int32_t fd, iio_event_data& iioevent,
                         int32_t waittimems) const
    {
        ssize_t readbytes{};
        if (auto epollfd = epoll_create1(0); epollfd >= 0)
        {
            epoll_event event{.events = EPOLLIN, .data = {.fd = fd}}, revent{};
            epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
            if (0 < epoll_wait(epollfd, &revent, 1, waittimems) &&
                (revent.events & EPOLLIN))
            {
                readbytes = ::read(fd, &iioevent, eventbytes);
                if (readbytes == eventbytes)
                    log(logs::level::debug,
                        "Received iio event, bytes num: " + str(readbytes));
                else
                    log(logs::level::warning,
                        "Cannot read iio event, sizes: " + str(readbytes) +
                            "/" + str(eventbytes));
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
                "Cha[" + str(channel) + "] clients notified");
        else
            log(logs::level::warning,
                "Cha[" + str(channel) + "] cannot notify clients");
        return ret;
    }

    std::optional<std::pair<double, int32_t>>
        extractdevicevalues(ssize_t readbytes,
                            const std::vector<uint8_t>& data) const
    {
        if (readbytes == triggeredbytes)
        {
            auto raw = (int16_t)((((int16_t)data[1] << 8) & 0xFF00) | data[0]);
            auto val = std::max(0., (double)raw), volt = getvoltfromraw(val);
            return std::make_pair(volt, getpercent(volt));
        }
        return {};
    }

    std::optional<std::pair<double, int32_t>>
        extracteventvalues(ssize_t readbytes, iio_event_data& event) const
    {
        if (readbytes == eventbytes)
        {
            auto chtype =
                (iio_chan_type)IIO_EVENT_CODE_EXTRACT_CHAN_TYPE(event.id);
            auto evtype = (iio_event_type)IIO_EVENT_CODE_EXTRACT_TYPE(event.id);
            auto dir =
                (iio_event_direction)IIO_EVENT_CODE_EXTRACT_DIR(event.id);

            if (iiochantype.contains(chtype) && iioevtype.contains(evtype) &&
                iioevdir.contains(dir))
                log(logs::level::debug,
                    "Decoded iio event: " + iiochantype.at(chtype) + "/" +
                        iioevtype.at(evtype) + "/" + iioevdir.at(dir));
            else
                log(logs::level::warning,
                    "Cannot decode event: " + str(chtype) + "/" + str(evtype) +
                        "/" + str(dir));

            if (chtype == IIO_VOLTAGE && evtype == IIO_EV_TYPE_THRESH &&
                (dir == IIO_EV_DIR_RISING || dir == IIO_EV_DIR_EITHER))
            {
                log(logs::level::debug, "Exposing adc values @ event");
                auto volt = getvoltage();
                return std::make_pair(volt, getpercent(volt));
            }
        }
        return {};
    }

    double getvoltage() const
    {
        std::string raw;
        sysfs->read("in_voltage" + str(channel) + "_raw", raw);
        return getvoltfromraw(std::atof(raw.c_str()));
    }

    double getvoltfromraw(double raw) const
    {
        raw = std::max(0., raw);
        auto voltage = raw * scale / 1000.;
        log(logs::level::debug,
            "Calculated voltage: " + str(raw) + " -> " + str(voltage));
        return voltage;
    }

    int32_t getpercent(double value) const
    {
        auto perc =
            (int32_t)std::min(100L, std::lround(100. * value / maxvalue));

        log(logs::level::debug, "Calculated percent: " + str(value) + " of " +
                                    str(maxvalue) + " -> " + str(perc));
        return perc;
    }

    int32_t getrawfromvolt(double volt) const
    {
        auto raw = (int32_t)std::round(volt * 1000. / scale);

        log(logs::level::debug,
            "Calculated raw: " + str(volt) + " -> " + str(raw));
        return raw;
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

Adc::Adc(const config_t& config)
{
    handler = std::visit(
        [](const auto& config) -> decltype(Adc::handler) {
            if constexpr (!std::is_same<const std::monostate&,
                                        decltype(config)>())
            {
                return std::make_unique<Adc::Handler>(config);
            }
            throw std::runtime_error(
                std::source_location::current().function_name() +
                "-> config not supported"s);
        },
        config);
}
Adc::~Adc() = default;

bool Adc::observe(std::shared_ptr<helpers::Observer<ObsData>> obs)
{
    return handler->observe(obs);
}

bool Adc::unobserve(std::shared_ptr<helpers::Observer<ObsData>> obs)
{
    return handler->unobserve(obs);
}

bool Adc::trigger()
{
    return handler->runtrigger();
}

bool Adc::read(AdcData& data)
{
    return handler->read(data);
}

} // namespace adc::rpi::ads1115
