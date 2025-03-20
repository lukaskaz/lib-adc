#include "trigger/interfaces/linux/oneshot/trigger.hpp"

#include "sysfs/helpers.hpp"
#include "sysfs/interfaces/linux/sysfs.hpp"

#include <filesystem>
#include <fstream>
#include <source_location>

namespace trigger::lnx::oneshot
{

using namespace sysfs;
using namespace sysfs::lnx;
using namespace std::string_literals;
using namespace std::chrono_literals;

std::filesystem::path path{"/sys/bus/iio/devices/iio_sysfs_trigger"};

struct Trigger::Handler
{
  public:
    explicit Handler(const config_t& config) :
        logif{std::get<1>(config)}, id{std::get<0>(config)},
        fsname{"trigger" + std::to_string(id)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>({path, logif})}
    {
        setup();
        log(logs::level::info, "Created oneshot trigger: " +
                                   (path / fsname).native() + " -> " + name);
    }

    ~Handler()
    {
        release();
        log(logs::level::info, "Removed oneshot trigger: " +
                                   (path / fsname).native() + " -> " + name);
    }

    bool getname(std::string& name) const
    {
        name = this->name;
        return true;
    }

    bool run() const
    {
        if (sysfs->write(path / fsname / "trigger_now", str(1)))
        {
            log(logs::level::debug, "Trigger " + name + " activated");
            return true;
        }
        return false;
    }

  private:
    const std::shared_ptr<logs::LogIf> logif;
    const uint32_t id;
    const std::string fsname;
    const std::shared_ptr<sysfs::SysfsIf> sysfs;
    std::string name;

    bool setup()
    {
        sysfs->elevwrite(path / "add_trigger", str(id));
        sysfs->elevread(path / fsname / "name", name);
        sysfs->elevate(path / fsname / "trigger_now", "o", "w");
        return true;
    }

    bool release() const
    {
        return sysfs->elevwrite(path / "remove_trigger", str(id));
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

} // namespace trigger::lnx::oneshot
