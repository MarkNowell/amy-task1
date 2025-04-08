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

    return 0;
}
