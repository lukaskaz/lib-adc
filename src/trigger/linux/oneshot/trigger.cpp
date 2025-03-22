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

std::filesystem::path pathiio{"/sys/bus/iio/devices/iio_sysfs_trigger"};
static const auto triggernameprefix{"sysfstrig"s};
static const auto triggerdirprefix{"trigger"s};

struct Trigger::Handler
{
  public:
    explicit Handler(const config_t& config) :
        logif{std::get<0>(config)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>({pathiio, logif})},
        id{getid()}, trigname{triggernameprefix + str(id)}, dirname{create()}
    {
        setup();
        log(logs::level::info,
            "Created oneshot trigger: " + (pathiio / dirname).native() +
                " -> " + trigname);
    }

    ~Handler()
    {
        release();
        log(logs::level::info,
            "Removed oneshot trigger: " + (pathiio / dirname).native() +
                " -> " + trigname);
    }

    bool getname(std::string& name) const
    {
        name = trigname;
        return true;
    }

    bool run() const
    {
        if (sysfs->write(pathiio / dirname / "trigger_now", str(1)))
        {
            log(logs::level::debug, "Trigger " + trigname + " activated");
            return true;
        }
        return false;
    }

  private:
    const std::shared_ptr<logs::LogIf> logif;
    const std::shared_ptr<sysfs::SysfsIf> sysfs;
    const int32_t minid{0}, maxid{100};
    const uint32_t id;
    const std::string trigname;
    const std::string dirname;

    bool setup() const
    {
        return sysfs->elevate(pathiio / dirname / "trigger_now", "o", "w");
    }

    bool release() const
    {
        return sysfs->elevwrite(pathiio / "remove_trigger", str(id));
    }

    std::string create() const
    {
        sysfs->elevwrite(pathiio / "add_trigger", str(id));
        return getdirname(pathiio);
    }

    uint32_t getid() const
    {
        std::unordered_set<std::string> existing;
        std::ranges::for_each(
            std::filesystem::directory_iterator(pathiio),
            [this, &existing](const auto& entry) {
                if (const auto& dir = entry.path().filename().native();
                    entry.is_directory() && dir.find(triggerdirprefix) == 0)
                {
                    std::string name;
                    sysfs->elevread(entry.path() / "name", name);
                    existing.emplace(name);
                    log(logs::level::debug,
                        "Found existing os timer " + dir + ":" + name);
                }
            });

        const auto seq = std::views::iota(minid, maxid);
        if (auto res = std::ranges::find_if_not(
                seq,
                [&existing](auto id) {
                    return existing.contains(triggernameprefix + str(id));
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

    std::string getdirname(const std::filesystem::path& path) const
    {
        const auto it = std::filesystem::directory_iterator(path);
        if (auto res = std::ranges::find_if(
                it,
                [this](const auto& entry) {
                    if (entry.is_directory() &&
                        entry.path().filename().native().find(
                            triggerdirprefix) == 0)
                    {
                        std::string name;
                        sysfs->elevread(entry.path() / "name", name);
                        return name == trigname;
                    }
                    return false;
                });
            res != std::filesystem::end(it))
        {
            auto dirname = res->path().filename().native();
            log(logs::level::debug,
                "Found os trigger directory name: " + dirname);
            return dirname;
        }
        else
            log(logs::level::error, "Cannot find created os trigger");
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
