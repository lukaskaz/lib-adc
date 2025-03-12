#include "adc/interfaces/rpi/ads1x15/adc.hpp"
#include "logs/interfaces/console/logs.hpp"
#include "logs/interfaces/group/logs.hpp"
#include "logs/interfaces/storage/logs.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    try
    {
        if (argc == 4)
        {
            std::cout << "ADCs scenario started\n";
            auto channel = (uint32_t)atoi(argv[1]);
            auto precision = (uint32_t)atoi(argv[2]);
            auto loglvl =
                (bool)atoi(argv[3]) ? logs::level::debug : logs::level::info;

            auto logconsole = logs::Factory::create<logs::console::Log,
                                                    logs::console::config_t>(
                {loglvl, logs::tags::hide});
            auto logstorage = logs::Factory::create<logs::storage::Log,
                                                    logs::storage::config_t>(
                {loglvl, logs::tags::show, {}});
            auto logif =
                logs::Factory::create<logs::group::Log, logs::group::config_t>(
                    {logconsole, logstorage});

            using namespace adc::rpi::ads1x15;
            auto adc0 = adc::Factory::create<Adc, config_t>(
                {channel, precision, logif});

            std::cout << "ADCs initiated\n";
            std::cout << "To read press [enter]" << std::flush;

            double value{};
            adc0->read(value);

            std::cout << "To exit press [enter]" << std::flush;
            getchar();
            std::cout << "ADCs released\n";
        }
    }
    catch (std::exception& err)
    {
        std::cerr << "[ERROR] " << err.what() << '\n';
    }
    return 0;
}
