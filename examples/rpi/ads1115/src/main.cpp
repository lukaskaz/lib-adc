#include "adc/interfaces/rpi/ads1115/adc.hpp"
#include "logs/interfaces/console/logs.hpp"
#include "logs/interfaces/group/logs.hpp"
#include "logs/interfaces/storage/logs.hpp"

#include <iostream>

using namespace std::chrono_literals;

int main(int argc, char** argv)
{
    try
    {
        if (argc == 6)
        {
            auto device = std::string{argv[1]};
            auto channel = (uint32_t)atoi(argv[2]);
            auto maxvalue = (double)atof(argv[3]);
            auto freq = (double)atof(argv[4]);
            auto loglvl =
                (bool)atoi(argv[5]) ? logs::level::debug : logs::level::info;
            auto logconsole = logs::Factory::create<logs::console::Log,
                                                    logs::console::config_t>(
                {loglvl, logs::time::hide, logs::tags::hide});
            auto logstorage = logs::Factory::create<logs::storage::Log,
                                                    logs::storage::config_t>(
                {loglvl, logs::time::show, logs::tags::show, {}});
            auto logif =
                logs::Factory::create<logs::group::Log, logs::group::config_t>(
                    {logconsole, logstorage});

            {
                std::cout << "First scenario -> ADCs standard read\n";

                using namespace adc::rpi::ads1115;
                auto adc = adc::Factory::create<Adc, configstd_t>(
                    {device, channel, maxvalue, logif});

                std::cout << "ADCs initiated\n";
                std::cout << "To read press [enter]" << std::flush;
                getchar();

                adc::AdcData data;
                adc->read(data);
                std::cout << "ADCs direct read voltage/percent: "
                          << std::get<0>(data) << "/" << std::get<1>(data)
                          << "\n";

                std::cout << "To exit press [enter]" << std::flush;
                getchar();

                std::cout << "First scenario DONE -> ADCs released\n";
            }
            {
                std::cout
                    << "Second scenario -> ADCs observed @ one shot trigger\n";

                using namespace adc::rpi::ads1115;
                auto adc = adc::Factory::create<Adc, configtrig_t>(
                    {device, channel, maxvalue, {}, logif});

                auto readingfunc = helpers::Observer<adc::ObsData>::create(
                    [](const adc::ObsData& data) {
                        auto [volt, perc] = std::get<1>(data);
                        std::cout << "Observer of cha: " << std::get<0>(data)
                                  << " got data voltage/percent: " << volt
                                  << "/" << perc << std::endl;
                    });

                adc->observe(readingfunc);

                std::cout << "ADCs initiated, to trigger press [enter]"
                          << std::flush;
                getchar();

                adc->trigger();
                usleep(std::chrono::microseconds(100ms).count());
                adc->unobserve(readingfunc);
                adc->trigger();

                std::cout << "To exit press [enter]" << std::flush;
                getchar();

                std::cout << "Second scenario DONE -> ADCs released\n";
            }
            {
                std::cout
                    << "Third scenario -> ADCs observed @ periodic trigger\n";

                using namespace adc::rpi::ads1115;
                auto adc = adc::Factory::create<Adc, configtrig_t>(
                    {device, channel, maxvalue, freq, logif});

                auto readingfunc = helpers::Observer<adc::ObsData>::create(
                    [](const adc::ObsData& data) {
                        auto [volt, perc] = std::get<1>(data);
                        std::cout << "Observer of cha: " << std::get<0>(data)
                                  << " got data voltage/percent: "
                                  << helpers::fp::trim(volt, 3) << "/" << perc
                                  << std::endl;
                    });

                adc->observe(readingfunc);

                std::cout << "ADCs initiated, trigger ongoing, to interrupt "
                             "press [enter]"
                          << std::flush;
                getchar();
                std::cout << "Third scenario DONE -> ADCs released\n";
            }
            {
                std::cout
                    << "Forth scenario -> ADCs observed @ data ready events\n";

                using namespace adc::rpi::ads1115;
                auto adc = adc::Factory::create<Adc, configevt_t>(
                    {device, channel, maxvalue, {}, logif});

                std::cout << "ADCs initiated, now... waiting for events\n";

                auto readingfunc = helpers::Observer<adc::ObsData>::create(
                    [](const adc::ObsData& data) {
                        auto [volt, perc] = std::get<1>(data);
                        std::cout << "Observer of cha: " << std::get<0>(data)
                                  << " got data voltage/percent: "
                                  << helpers::fp::trim(volt, 3) << "/" << perc
                                  << std::endl;
                    });
                adc->observe(readingfunc);

                std::cout << "To exit press [enter]" << std::flush;
                getchar();

                std::cout << "Forth scenario DONE -> ADCs released\n";
            }
            {
                std::cout << "Fifth scenario -> ADCs observed @ limit events\n";

                using namespace adc::rpi::ads1115;
                auto adc = adc::Factory::create<Adc, configevt_t>(
                    {device, channel, maxvalue, {1.75, {}}, logif});

                std::cout << "ADCs initiated, now... waiting for events\n";

                auto readingfunc = helpers::Observer<adc::ObsData>::create(
                    [](const adc::ObsData& data) {
                        auto [volt, perc] = std::get<1>(data);
                        std::cout << "Observer of cha: " << std::get<0>(data)
                                  << " got data voltage/percent: "
                                  << helpers::fp::trim(volt, 3) << "/" << perc
                                  << std::endl;
                    });
                adc->observe(readingfunc);

                std::cout << "To exit press [enter]" << std::flush;
                getchar();

                std::cout << "Fifth scenario DONE -> ADCs released\n";
            }
            {
                std::cout
                    << "Sixth scenario -> ADCs observed @ window events\n";

                using namespace adc::rpi::ads1115;
                auto adc = adc::Factory::create<Adc, configevt_t>(
                    {device, channel, maxvalue, {1.9, 2.55}, logif});

                std::cout << "ADCs initiated, now... waiting for events\n";

                auto readingfunc = helpers::Observer<adc::ObsData>::create(
                    [](const adc::ObsData& data) {
                        auto [volt, perc] = std::get<1>(data);
                        std::cout << "Observer of cha: " << std::get<0>(data)
                                  << " got data voltage/percent: "
                                  << helpers::fp::trim(volt, 3) << "/" << perc
                                  << std::endl;
                    });
                adc->observe(readingfunc);

                std::cout << "To exit press [enter]" << std::flush;
                getchar();

                std::cout << "Sixth scenario DONE -> ADCs released\n";
            }
        }
    }
    catch (std::exception& err)
    {
        std::cerr << "[ERROR] " << err.what() << '\n';
    }
    return 0;
}
