#include "trigger/interfaces/linux/periodic/trigger.hpp"

#include "sysfs/helpers.hpp"
#include "sysfs/interfaces/linux/sysfs.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <source_location>
#include <unordered_set>

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
        logif{std::get<1>(config)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>({pathiio, logif})},
        id{getid()}, freq{std::get<0>(config)}, cnffsname{"cnffstrig" + str(id)}

    {
        setup();
        log(logs::level::info, "Created periodic trigger: " +
                                   (pathiio / std::get<0>(names)).native() +
                                   " -> " + std::get<1>(names));
    }

    ~Handler()
    {
        release();
        log(logs::level::info, "Removed periodic trigger: " +
                                   (pathiio / std::get<0>(names)).native() +
                                   " -> " + std::get<1>(names));
    }

    bool getname(std::string& name) const
    {
        name = std::get<1>(names);
        return true;
    }

    bool run() const
    {
        return true;
    }

  private:
    const std::shared_ptr<logs::LogIf> logif;
    const std::shared_ptr<sysfs::SysfsIf> sysfs;
    const uint32_t id;
    const double freq;
    const std::string cnffsname;
    std::pair<std::string, std::string> names;

    bool setup()
    {
        sysfs->elevate(pathcfs, "o", "w");
        std::filesystem::create_directories(pathcfs / cnffsname);
        names = getnames();
        sysfs->elevwrite(pathiio / std::get<0>(names) / "sampling_frequency",
                         str(freq));
        return true;
    }

    bool release() const
    {
        return std::filesystem::remove(pathcfs / cnffsname);
    }

    uint32_t getid() const
    {
        std::unordered_set<std::string> present;
        std::ranges::for_each(
            std::filesystem::directory_iterator(pathcfs),
            [this, &present](const auto& entry) {
                if (const auto& dir = entry.path().filename().native();
                    entry.is_directory() && dir.find("cnffstrig") == 0)
                {
                    present.emplace(dir);
                    log(logs::level::debug, "Found existing hr dir " + dir);
                }
            });

        auto seq = std::views::iota(0, 100);
        if (auto res = std::ranges::find_if_not(seq,
                                                [&present](auto id) {
                                                    return present.contains(
                                                        "cnffstrig" + str(id));
                                                });
            res != seq.end())
        {
            log(logs::level::debug,
                "Found available hr trigger id: " + str(*res));
            return (*res);
        }
        log(logs::level::critical, "Cannot find available hr trigger id");
        throw std::runtime_error("Cannot find available hr trigger id");
    }

    std::pair<std::string, std::string> getnames() const
    {
        std::string fsname;
        const auto it = std::filesystem::directory_iterator(pathiio);
        if (auto res = std::ranges::find_if(
                it,
                [this](const auto& entry) {
                    if (entry.is_directory() &&
                        entry.path().filename().native().find("trigger") == 0)
                    {
                        std::string name;
                        sysfs->elevread(entry.path() / "name", name);
                        return name == cnffsname;
                    }
                    return false;
                });
            res != std::filesystem::end(it))
        {
            fsname = res->path().filename().native();
            log(logs::level::debug,
                "Found hr trigger names: " + fsname + " -> " + cnffsname);
            return {fsname, cnffsname};
        }
        else
        {
            log(logs::level::error, "Cannot find created hr trigger");
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

} // namespace trigger::lnx::periodic
