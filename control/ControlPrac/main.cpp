#include <iostream>
#include <fstream>
#include "../json.hpp"
#include <vector>
#include <string_view>
#include <chrono>
#include <thread>

#include "room.h"

using namespace std::chrono_literals;

int main()
{
    Room* r=Room::readUnits();
    std::cout<<"Created room:\n"<<*r;
    //Target values
    const float TARGET_TEMP{22.5};
    const float TARGET_HUMIDITY{55.0};
    const float TARGET_CO2{400.0};

    const float MAXTEMP=TARGET_TEMP+3;
    const float MINCO2=TARGET_CO2-50;

    while(true)
    {
        r->Room::tempControl(TARGET_TEMP,MAXTEMP);
        r->Room::humidControl(TARGET_HUMIDITY);
        r->Room::co2Control(TARGET_CO2,MINCO2);

        std::cout<<*r;

        r->Room::writeUnits();
        std::this_thread::sleep_for(5000ms);
    }

    return 0;
}
