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

static const std::filesystem::path pathcfs{
    "/sys/kernel/config/iio/triggers/hrtimer/"};
static const std::filesystem::path pathiio{"/sys/bus/iio/devices/"};
static const auto cnffstriggerprefix{"cnffstrig"s};
static const auto sysfstriggerprefix{"trigger"s};

struct Trigger::Handler
{
  public:
    explicit Handler(const config_t& config) :
        logif{std::get<1>(config)},
        sysfs{sysfs::Factory::create<Sysfs, configrw_t>({pathiio, logif})},
        id{getid()}, freq{std::get<0>(config)},
        trigname{cnffstriggerprefix + str(id)}, dirname{create()}

    {
        setup();
        log(logs::level::info,
            "Created periodic trigger: " + (pathiio / dirname).native() +
                " -> " + trigname);
    }

    ~Handler()
    {
        release();
        log(logs::level::info,
            "Removed periodic trigger: " + (pathiio / dirname).native() +
                " -> " + trigname);
    }

    bool getname(std::string& name) const
    {
        name = trigname;
        return true;
    }

    bool run() const
    {
        return true;
    }

  private:
    const std::shared_ptr<logs::LogIf> logif;
    const std::shared_ptr<sysfs::SysfsIf> sysfs;
    const int32_t minid{0}, maxid{100};
    const uint32_t id;
    const double freq;
    const std::string trigname;
    const std::string dirname;

    bool setup() const
    {
        return sysfs->elevwrite(pathiio / dirname / "sampling_frequency",
                                str(freq));
    }

    bool release() const
    {
        return std::filesystem::remove(pathcfs / trigname);
    }

    std::string create() const
    {
        if (sysfs->elevate(pathcfs, "o", "w") &&
            std::filesystem::create_directories(pathcfs / trigname))
            return getdirname(pathiio);
        log(logs::level::critical, "Cannot create trigger " + trigname);
        return {};
    }

    uint32_t getid() const
    {
        std::unordered_set<std::string> existing;
        std::ranges::for_each(
            std::filesystem::directory_iterator(pathcfs),
            [this, &existing](const auto& entry) {
                if (const auto& dir = entry.path().filename().native();
                    entry.is_directory() && dir.find(cnffstriggerprefix) == 0)
                {
                    existing.emplace(dir);
                    log(logs::level::debug,
                        "Found existing hr timer dir " + dir);
                }
            });

        const auto seq = std::views::iota(minid, maxid);
        if (auto res = std::ranges::find_if_not(
                seq,
                [&existing](auto id) {
                    return existing.contains(cnffstriggerprefix + str(id));
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

    std::string getdirname(const std::filesystem::path& path) const
    {
        const auto it = std::filesystem::directory_iterator{path};
        if (auto res = std::ranges::find_if(
                it,
                [this](const auto& entry) {
                    if (entry.is_directory() &&
                        entry.path().filename().native().find(
                            sysfstriggerprefix) == 0)
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
                "Found hr trigger directory name: " + dirname);
            return dirname;
        }
        else
            log(logs::level::error, "Cannot find created hr trigger");
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
