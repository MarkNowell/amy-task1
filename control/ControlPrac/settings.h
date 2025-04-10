#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "../json.hpp"

//#pragma comment(lib, "Ws2_32.a")

using json = nlohmann::json;

class Settings
{
public:
    //set values
    static float TARGET_TEMP;
    static float TARGET_HUMIDITY;
    static float TARGET_CO2;

    static float MAXTEMP;
    static float MINCO2;

static json getSettingsFromServer();
static void currentSettings();

friend std::ostream& operator<<(std::ostream& out, const Settings&);
};


#endif // SETTINGS_H_INCLUDED
