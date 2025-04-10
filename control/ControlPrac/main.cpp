#include <iostream>
#include <fstream>
#include "../json.hpp"
#include <vector>
#include <string_view>
#include <chrono>
#include <thread>

#include "room.h"
#include "settings.h"

using namespace std::chrono_literals;

int main()
{
    Room* r=Room::readAllUnits();
    std::cout<<"Created room:\n"<<*r;

    Settings::currentSettings();
    std::cout<<Settings();

    //add in a time of day to determine when we check for updated settings
    auto settings_updated = std::chrono::steady_clock::now();


    while(true)
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = now-settings_updated;

        if(elapsed.count()>=10.0)
        {
            Settings::currentSettings();
            std::cout<<Settings();
            settings_updated=now;
        }
        r->readSensors();

        r->Room::tempControl(Settings::TARGET_TEMP,Settings::MAXTEMP);
        r->Room::humidControl(Settings::TARGET_HUMIDITY);
        r->Room::co2Control(Settings::TARGET_CO2,Settings::MINCO2);

        std::cout<<*r;

        r->Room::writeUnits();
        std::cout<<"-------------------------------------\n";
        std::this_thread::sleep_for(2000ms);
    }

    return 0;
}
