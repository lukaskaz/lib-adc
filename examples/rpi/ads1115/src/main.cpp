#include "adc/interfaces/rpi/ads1115/adc.hpp"
#include "logs/interfaces/console/logs.hpp"
#include "logs/interfaces/group/logs.hpp"
#include "logs/interfaces/storage/logs.hpp"

#include <iostream>

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
                {loglvl, logs::time::show, logs::tags::hide});
            auto logstorage = logs::Factory::create<logs::storage::Log,
                                                    logs::storage::config_t>(
                {loglvl, logs::time::show, logs::tags::show, {}});
            auto logif =
                logs::Factory::create<logs::group::Log, logs::group::config_t>(
                    {logconsole, logstorage});

            {
                std::cout << "First scenario -> ADCs standard read\n";

                using namespace adc::rpi::ads1115;
                auto adc0 = adc::Factory::create<Adc, config_t>(
                    {device, readtype::standard, channel, maxvalue, freq,
                     logif});

                std::cout << "ADCs initiated\n";
                std::cout << "To read press [enter]" << std::flush;
                getchar();

                double value{};
                adc0->read(0, value);
                std::cout << "ADCs voltage: " << value << "\n";

                int percent{};
                adc0->read(0, percent);
                std::cout << "ADCs percent: " << percent << "\n";

                std::cout << "To exit press [enter]" << std::flush;
                getchar();

                std::cout << "First scenario DONE -> ADCs released\n";
            }
            {
                std::cout
                    << "Second scenario -> ADCs observed @ one shot trigger\n";
                using namespace adc::rpi::ads1115;
                auto adc0 = adc::Factory::create<Adc, config_t>(
                    {device, readtype::trigger_oneshot, channel, maxvalue, freq,
                     logif});

                auto readingfunc = Observer<adc::AdcData>::create(
                    [](const adc::AdcData& data) {
                        auto [volt, perc] = std::get<1>(data);
                        std::cout << "Observer of cha: " << std::get<0>(data)
                                  << " got data voltage/percent: " << volt
                                  << "/" << perc << std::endl;
                    });

                adc0->observe(0, readingfunc);

                std::cout << "ADCs initiated, to trigger press [enter]"
                          << std::flush;
                getchar();

                adc0->trigger(0);
                usleep(100 * 1000);

                adc0->unobserve(0, readingfunc);
                adc0->trigger(0);

                std::cout << "To exit press [enter]" << std::flush;
                getchar();

                std::cout << "Second scenario DONE -> ADCs released\n";
            }
            {
                std::cout
                    << "Third scenario -> ADCs observed @ periodic trigger\n";
                using namespace adc::rpi::ads1115;
                auto adc0 = adc::Factory::create<Adc, config_t>(
                    {device, readtype::trigger_periodic, channel, maxvalue,
                     freq, logif});

                auto readingfunc = Observer<adc::AdcData>::create(
                    [](const adc::AdcData& data) {
                        auto [volt, perc] = std::get<1>(data);
                        std::cout << "Observer of cha: " << std::get<0>(data)
                                  << " got data voltage/percent: " << volt
                                  << "/" << perc << std::endl;
                    });

                adc0->observe(0, readingfunc);

                std::cout << "ADCs initiated, trigger ongoing, to interrupt "
                             "press [enter]"
                          << std::flush;
                getchar();
                std::cout << "Third scenario DONE -> ADCs released\n";
            }
        }
    }
    catch (std::exception& err)
    {
        std::cerr << "[ERROR] " << err.what() << '\n';
    }
    return 0;
}
