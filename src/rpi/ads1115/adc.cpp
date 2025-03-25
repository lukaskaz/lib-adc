#include "adc/interfaces/rpi/ads1115/adc.hpp"

#include "sysfs/helpers.hpp"
#include "sysfs/interfaces/linux/sysfs.hpp"

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

struct Adc::Handler : public helpers::Observable<AdcData>
{
  public:
    explicit Handler(const config_t& config) :
        device{std::get<0>(config)}, logif{std::get<5>(config)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>(
            {iiodevices / device, logif})},
        trigger{std::get<4>(config)}, reading{std::get<1>(config)},
        channel{std::get<2>(config)}, scale{[this]() {
            std::string scale;
            sysfs->read("in_voltage" + str(channel) + "_scale", scale);
            return std::atof(scale.c_str());
        }()},
        maxvalue{std::get<3>(config)}
    {
        switch (reading)
        {
            case readtype::standard:
                break;
            case readtype::trigger_oneshot:
                [[fallthrough]];
            case readtype::trigger_periodic:
                triggersetup();
                triggermonitoring();
                break;
            case readtype::event_dataready:
                break;
            case readtype::event_limit:
                break;
            case readtype::event_window:
                eventsetup();
                eventmonitoring();
                break;
        }

        log(logs::level::info, "Created adc ads1115 [dev/mode/cha/max]: " +
                                   device + "/" + str((int32_t)reading) + "/" +
                                   str(channel) + "/" + str(maxvalue));
    }

    ~Handler()
    {
        switch (reading)
        {
            case readtype::standard:
                break;
            case readtype::trigger_oneshot:
                [[fallthrough]];
            case readtype::trigger_periodic:
                running.request_stop();
                triggerrelease();
                break;
            case readtype::event_dataready:
                break;
            case readtype::event_limit:
                break;
            case readtype::event_window:
                running.request_stop();
                eventrelease();
                break;
        }

        log(logs::level::info, "Removed adc ads1115 [dev/mode/cha]: " + device +
                                   "/" + str((int32_t)reading) + "/" +
                                   str(channel));
    }

    bool observe(std::shared_ptr<helpers::Observer<AdcData>> obs)
    {
        std::ostringstream oss;
        oss << std::hex << obs.get();
        log(logs::level::debug, "Adding observer " + oss.str() + " for cha " +
                                    str(channel) + " and notifying client");
        subscribe(obs);
        return true;
    }

    bool unobserve(std::shared_ptr<helpers::Observer<AdcData>> obs)
    {
        std::ostringstream oss;
        oss << std::hex << obs.get();
        log(logs::level::debug,
            "Removing observer " + oss.str() + " for cha " + str(channel));
        unsubscribe(obs);
        return true;
    }

    bool runtrigger([[maybe_unused]] uint32_t channel) const
    {
        return trigger->run();
    }

    bool read(double& val)
    {
        val = getreadout(channel);
        log(logs::level::debug,
            "Cha[" + str(channel) + "] value read: " + str(val));
        return true;
    }

    bool read(int32_t& perc)
    {
        perc = getpercent(getreadout(channel));
        log(logs::level::debug,
            "Cha[" + str(channel) + "] percent read: " + str(perc));
        return true;
    }

  private:
    const std::string device;
    const std::shared_ptr<logs::LogIf> logif;
    const std::shared_ptr<sysfs::SysfsIf> sysfs;
    std::shared_ptr<trigger::TriggerIf> trigger;
    const readtype reading;
    const uint32_t channel;
    const double scale;
    const double maxvalue;
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
        usleep(100 * 1000);
        auto scanned_item{"in_voltage" + str(channel) + "_en"};
        sysfs->elevwrite(iiodevices / device / "scan_elements" / scanned_item,
                         str(0));
        sysfs->elevwrite(iiodevices / device / "buffer/length", str(0));
        return true;
    }

    bool eventsetup() const
    {
        auto eventitem{"in_voltage" + str(channel) + "_thresh_falling_value"};
        sysfs->elevwrite(iiodevices / device / "events" / eventitem, str(5000));
        eventitem = "in_voltage" + str(channel) + "_thresh_rising_value";
        sysfs->elevwrite(iiodevices / device / "events" / eventitem,
                         str(10000));
        eventitem = "in_voltage" + str(channel) + "_thresh_either_en";
        sysfs->elevwrite(iiodevices / device / "events" / eventitem, str(1));
        return true;
    }

    bool eventrelease() const
    {
        auto eventitem{"in_voltage" + str(channel) + "_thresh_falling_value"};
        sysfs->elevwrite(iiodevices / device / "events" / eventitem,
                         str(-32768));
        eventitem = "in_voltage" + str(channel) + "_thresh_rising_value";
        sysfs->elevwrite(iiodevices / device / "events" / eventitem,
                         str(32767));
        eventitem = "in_voltage" + str(channel) + "_thresh_either_en";
        sysfs->elevwrite(iiodevices / device / "events" / eventitem, str(0));
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
                while (!running.stop_requested())
                    if (auto num = getfddata(fd, data, timeout); num > 0)
                        if (auto values = extractfdvalues(num, data))
                            notifyclients(channel, *values);
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
                while (!running.stop_requested())
                    if (auto num = getevfddata(evfd, event, timeout); num > 0)
                        if (auto values = extractevfdvalues(num, event))
                            notifyclients(channel, *values);
                fclose(ifs);
            }
            catch (const std::exception& ex)
            {
                log(logs::level::error, ex.what());
                throw;
            }
        });
    }

    ssize_t getfddata(int32_t devfd, std::vector<uint8_t>& data,
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
                        "Received bytes num: " + str(readbytes));
                }
            close(epollfd);
        }
        return readbytes;
    }

    ssize_t getevfddata(int32_t evfd, iio_event_data& iioevent,
                        int32_t waittimems) const
    {
        ssize_t readbytes{};
        auto epollfd = epoll_create1(0);
        if (epollfd >= 0)
        {
            epoll_event event{.events = EPOLLIN, .data = {.fd = evfd}},
                revent{};
            epoll_ctl(epollfd, EPOLL_CTL_ADD, evfd, &event);
            if (0 < epoll_wait(epollfd, &revent, 1, waittimems))
                if (revent.events & EPOLLIN)
                {
                    readbytes = ::read(evfd, &iioevent, eventbytes);
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
        extractfdvalues(ssize_t readbytes,
                        const std::vector<uint8_t>& data) const
    {
        if (readbytes == triggeredbytes)
        {
            auto raw = (int16_t)((((int16_t)data[1] << 8) & 0xFF00) | data[0]);
            auto val = std::max(0., (double)raw),
                 volt = getvalue(val, scale, 3);
            return std::make_pair(volt, getpercent(volt));
        }
        return {};
    }

    std::optional<std::pair<double, int32_t>>
        extractevfdvalues(ssize_t readbytes, iio_event_data& event) const
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
                dir == IIO_EV_DIR_EITHER)
            {
                log(logs::level::debug, "Exposing adc values @ event");
                auto volt = getreadout(channel, 4);
                return std::make_pair(volt, getpercent(volt));
            }
        }
        return {};
    }

    double getreadout(uint32_t channel, uint32_t prec = 2) const
    {
        std::string raw;
        sysfs->read("in_voltage" + str(channel) + "_raw", raw);
        return getvalue(std::atof(raw.c_str()), scale, prec);
    }

    double getvalue(double raw, double scale, uint32_t prec) const
    {
        raw = std::max(0., raw);
        auto value = raw * scale / 1000.;
        auto ratio = helpers::tr::pow(10, prec);
        auto volt = std::round(ratio * value) / ratio;

        log(logs::level::debug,
            "Calculated voltage: " + str(raw) + " -> " + str(volt));
        return volt;
    }

    int32_t getpercent(double value) const
    {
        auto perc =
            (int32_t)std::min(100L, std::lround(100. * value / maxvalue));

        log(logs::level::debug, "Calculated percent: " + str(value) + " of " +
                                    str(maxvalue) + " -> " + str(perc));
        return perc;
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

bool Adc::observe(std::shared_ptr<helpers::Observer<AdcData>> obs)
{
    return handler->observe(obs);
}

bool Adc::unobserve(std::shared_ptr<helpers::Observer<AdcData>> obs)
{
    return handler->unobserve(obs);
}

bool Adc::trigger(uint32_t channel)
{
    return handler->runtrigger(channel);
}

bool Adc::read(double& val)
{
    return handler->read(val);
}

bool Adc::read(int32_t& val)
{
    return handler->read(val);
}

} // namespace adc::rpi::ads1115
