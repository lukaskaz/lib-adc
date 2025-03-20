#include "trigger/interfaces/linux/periodic/trigger.hpp"

#include "sysfs/helpers.hpp"
#include "sysfs/interfaces/linux/sysfs.hpp"

#include <filesystem>
#include <fstream>
#include <source_location>

namespace trigger::lnx::periodic
{

using namespace sysfs;
using namespace sysfs::lnx;
using namespace std::string_literals;
using namespace std::chrono_literals;

std::filesystem::path pathcfs{"/sys/kernel/config/iio/triggers/hrtimer/"};
std::filesystem::path pathiio{"/sys/bus/iio/devices/"};

struct Trigger::Handler
{
  public:
    explicit Handler(const config_t& config) :
        logif{std::get<2>(config)}, id{std::get<0>(config)},
        freq{std::get<1>(config)}, sysfsname{"trigger" + std::to_string(id)},
        cnffsname{"cnffstrig" + std::to_string(id)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>({pathiio, logif})}
    {
        setup();
        log(logs::level::info,
            "Created periodic trigger: " + (pathiio / sysfsname).native() +
                " -> " + name);
    }

    ~Handler()
    {
        release();
        log(logs::level::info,
            "Removed periodic trigger: " + (pathiio / sysfsname).native() +
                " -> " + name);
    }

    bool getname(std::string& name) const
    {
        name = this->name;
        return true;
    }

    bool run() const
    {
        return true;
    }

  private:
    const std::shared_ptr<logs::LogIf> logif;
    const uint32_t id;
    const double freq;
    const std::string sysfsname;
    const std::string cnffsname;
    const std::shared_ptr<sysfs::SysfsIf> sysfs;
    std::string name;

    bool setup()
    {
        sysfs->elevate(pathcfs, "o", "w");
        std::filesystem::create_directories(pathcfs / cnffsname);
        sysfs->elevread(pathiio / sysfsname / "name", name);
        sysfs->elevwrite(pathiio / sysfsname / "sampling_frequency", str(freq));
        return true;
    }

    bool release() const
    {
        return std::filesystem::remove(pathcfs / cnffsname);
    }

    void log(
        logs::level level, const std::string& msg,
        const std::source_location loc = std::source_location::current()) const
    {
        if (logif)
            logif->log(level, std::string{loc.function_name()}, msg);
    }
};

Trigger::Trigger(const config_t& config) :
    handler{std::make_unique<Handler>(config)}
{}
Trigger::~Trigger() = default;

bool Trigger::name(std::string& name)
{
    return handler->getname(name);
}

bool Trigger::run()
{
    return handler->run();
}

} // namespace trigger::lnx::periodic
