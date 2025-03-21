#include "trigger/interfaces/linux/oneshot/trigger.hpp"

#include "sysfs/helpers.hpp"
#include "sysfs/interfaces/linux/sysfs.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <source_location>
#include <unordered_set>

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
        logif{std::get<0>(config)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>({path, logif})},
        id{getid()}
    {
        setup();
        log(logs::level::info,
            "Created oneshot trigger: " + (path / std::get<0>(names)).native() +
                " -> " + std::get<1>(names));
    }

    ~Handler()
    {
        release();
        log(logs::level::info,
            "Removed oneshot trigger: " + (path / std::get<0>(names)).native() +
                " -> " + std::get<1>(names));
    }

    bool getname(std::string& name) const
    {
        name = std::get<1>(names);
        return true;
    }

    bool run() const
    {
        if (sysfs->write(path / std::get<0>(names) / "trigger_now", str(1)))
        {
            log(logs::level::debug,
                "Trigger " + std::get<1>(names) + " activated");
            return true;
        }
        return false;
    }

  private:
    const std::shared_ptr<logs::LogIf> logif;
    const std::shared_ptr<sysfs::SysfsIf> sysfs;
    const uint32_t id;
    std::pair<std::string, std::string> names;

    bool setup()
    {
        sysfs->elevwrite(path / "add_trigger", str(id));
        names = getnames();
        sysfs->elevate(path / std::get<0>(names) / "trigger_now", "o", "w");
        return true;
    }

    bool release() const
    {
        return sysfs->elevwrite(path / "remove_trigger", str(id));
    }

    uint32_t getid() const
    {
        std::unordered_set<std::string> present;
        std::ranges::for_each(
            std::filesystem::directory_iterator(path),
            [this, &present](const auto& entry) {
                if (const auto& dir = entry.path().filename().native();
                    entry.is_directory() && dir.find("trigger") == 0)
                {
                    present.emplace(dir);
                    log(logs::level::debug, "Found existing os dir " + dir);
                }
            });

        auto seq = std::views::iota(0, 100);
        if (auto res = std::ranges::find_if_not(seq,
                                                [&present](auto id) {
                                                    return present.contains(
                                                        "trigger" + str(id));
                                                });
            res != seq.end())
        {
            log(logs::level::debug,
                "Found available os trigger id: " + str(*res));
            return (*res);
        }
        log(logs::level::critical, "Cannot find available os trigger id");
        throw std::runtime_error("Cannot find available os trigger id");
    }

    std::pair<std::string, std::string> getnames() const
    {
        std::string fsname, trigname = "sysfstrig" + str(id);
        const auto it = std::filesystem::directory_iterator(path);
        if (auto res = std::ranges::find_if(
                it,
                [this, &trigname](const auto& entry) {
                    if (entry.is_directory() &&
                        entry.path().filename().native().find("trigger") == 0)
                    {
                        std::string name;
                        sysfs->elevread(entry.path() / "name", name);
                        return name == trigname;
                    }
                    return false;
                });
            res != std::filesystem::end(it))
        {
            fsname = res->path().filename().native();
            log(logs::level::debug,
                "Found os trigger names: " + fsname + " -> " + trigname);
            return {fsname, trigname};
        }
        else
        {
            log(logs::level::error, "Cannot find created os trigger");
        }
        return {};
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
