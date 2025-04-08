#include <iostream>
#include <fstream>
#include "../json.hpp"
#include <vector>
#include <string_view>
#include "room.h"

//Define the locations of in/out files
#define OUTPUTS "../../shared/outputs.json"
#define SENSORS "../../shared/sensors.json"


using json=nlohmann::json;

// x defines which search mode to go into: string match or find substring
static void readJson(json& d,std::string_view searchstring, std::vector<Sensor>* u)
{ //search mode based on the full string
   for(auto reading:d[searchstring])
    {
        Sensor s(reading);
        u->emplace_back(s);
    }
}
static void readJson(json& d,std::string_view searchstring, std::vector<Control>* u)
{
//search mode using substring
   for(auto& [key,value]:d.items())
   {
       if(key.find(searchstring)!=std::string::npos)
        {
        Control s(value);
        u->emplace_back(s);
        }
   }

}

 //overload the << operator to easily output the Unit readings
std::ostream& operator<<(std::ostream& out, const Room& r)
{
    out<<"Temperatures: ";  //display the temperature readings
    for(const Unit& s:r.m_temp)
    {
        out<<s.getInput()<<" ";
    }
    out<<'\n';

    out<<"Humidity: ";      //display humidity readings
    for(const Unit& s:r.m_humidity)
    {
        out<<s.getInput()<<" ";
    }
    out<<'\n';

    out<<"CO2: ";  //display co2 readings
    for(const Unit& s:r.m_co2)
    {
        out<<s.getInput()<<" ";
    }
    out<<"\n\nCurrent control unit settings:\n";

    //display current
    out<<"Heaters: ";
    for(const Unit& s:r.m_heater)
    {
        out<<s.getInput()<<" ";
    }
    out<<'\n';

    out<<"Fans: ";
    for(const Unit& s:r.m_fan)
    {
        out<<s.getInput()<<" ";
    }
    out<<'\n';

    out<<"Humidifiers: ";
    for(const Unit& s:r.m_humidifier)
    {
        out<<s.getInput()<<" ";
    }
    out<<'\n';

    out<<"CO2 Injectors: ";
    for(const Unit& s:r.m_co2inject)
    {
        out<<s.getInput()<<" ";
    }


    return out;
}

Room* Room::readUnits()
    {
    //read the availble sensors
    std::ifstream sfile(SENSORS);
    if(!sfile)
    {
        throw std::runtime_error("Error: Failed to open Sensor Unit data file.");
    }
    json data;
    sfile>>data; //read the Unit file into the json object

    std::vector<Sensor> tunits{};
    readJson(data,"temperature",&tunits);
    std::vector<Sensor>hunits{};
    readJson(data,"humidity",&hunits);
    std::vector<Sensor>cunits{};
    readJson(data,"co2",&cunits);

    //read the availble control units
    std::ifstream ofile(OUTPUTS);
    if(!ofile)
    {
        throw std::runtime_error("Error: Failed to open Control Unit data file.");
    }
    json odata;
    ofile>>odata; //read the Unit file into the json object

    std::vector<Control> heatunits{};
    readJson(odata,"heater", &heatunits);
    std::vector<Control>fanunits{};
    readJson(odata,"fan",&fanunits);
    std::vector<Control>humidunits{};
    readJson(odata,"humidifier",&humidunits);
    std::vector<Control>co2inject{};
    readJson(odata,"co2_injector",&co2inject);

    return new Room(tunits,hunits,cunits,heatunits,fanunits,humidunits,co2inject);
    }
