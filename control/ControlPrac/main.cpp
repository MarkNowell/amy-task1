#include <iostream>
#include <fstream>
#include "../json.hpp"
#include <vector>
#include <string_view>

#include "room.h"


int main()
{
    Room* r=Room::readUnits();
    std::cout<<"Created room:\n"<<*r;

    //Target values
    const float TARGET_TEMP{22.5};
    const float TARGET_HUMIDITY{55.0};
    const float TARGET_CO{400.0};

    const float MAXTEMP=TARGET_TEMP+3;




    return 0;
}
