#include <iostream>
#include <fstream>
#include "../json.hpp"
#include <vector>
#include <string_view>
#include <chrono>
#include <thread>

#include "room.h"
#include "settings.h"
#include "controller.h"
#include "fuzzy.h"

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

        //PIDController tcontrol(2,0.5,1);
        PController hcontrol(0.5);
        PIDController ccontrol(0.05,0.03,0.02);

        r->tempControl(Settings::TARGET_TEMP);                  //using Fuzzy
        r->humidControl(Settings::TARGET_HUMIDITY,hcontrol);    //using P
        r->co2Control(Settings::TARGET_CO2,ccontrol);           //using PID

        std::cout<<*r;

        r->Room::writeUnits();
        std::cout<<"-------------------------------------\n";
        std::this_thread::sleep_for(2000ms);
    }

    return 0;
}
