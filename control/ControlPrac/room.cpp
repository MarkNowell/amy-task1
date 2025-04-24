#include <iostream>
#include <fstream>
#include "../json.hpp"
#include <vector>
#include <string_view>
#include "room.h"
#include "controller.h"
#include "fuzzy.h"

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
float Room::avSensor(std::vector<Sensor> stype)
{
    float av{};
    int n{};
    for (const Sensor& s:stype)
    {
        av+=s.getInput();
        n++;
    }
    return av/n;
}

void Room::tempControl(float target, Controller& c)
{
    float avTemp=this->avSensor(m_temp);
    float updateVoltage = c.compute(target, avTemp);
    if(avTemp<target)
    {
       for(Control& f:m_fan)f.updateInput(0);
       for(Control& h:m_heater)h.updateInput(updateVoltage);
    }
    if(avTemp>target)
    {
        for(Control& h:m_heater)h.updateInput(0);
        for(Control& f:m_fan)f.updateInput(-updateVoltage); //make this negative to negate the negative from (avTemp-target) in compute
    }

}
//fuzzy temp control
void Room::tempControl(TempFuzzyControl& c)
{
    float avTemp=this->avSensor(m_temp);
    TempOutPair output=c.compute(avTemp);
    for(Control& h:m_heater)h.updateInput(output.heaterVal);
    for(Control& f:m_fan)f.updateInput(output.fanVal);
}

void Room::humidControl(float target,Controller& c)
{
    float avHumid{};
    avHumid=this->avSensor(m_humidity);

    float updateVoltage=c.compute(target,avHumid);
    for(Control& h:m_humidifier) h.updateInput(updateVoltage);
}

void Room::co2Control(float target,Controller& c)
{
    float avCo2{};
    avCo2=this->avSensor(m_co2);

    float updateVoltage=c.compute(target,avCo2);

    for(Control& injector:m_co2inject)injector.updateInput(updateVoltage);
}

 //overload the << operator to easily output the Unit readings
std::ostream& operator<<(std::ostream& out, const Room& r)
{
    out<<"Temperatures: ";  //display the temperature readings
    for(const Unit& s:r.m_temp){out<<s.getInput()<<" ";}
    out<<'\n';

    out<<"Humidity: ";      //display humidity readings
    for(const Unit& s:r.m_humidity){out<<s.getInput()<<" ";}
    out<<'\n';

    out<<"CO2: ";  //display co2 readings
    for(const Unit& s:r.m_co2){out<<s.getInput()<<" ";}
    out<<"\n\nCurrent control unit settings:\n";

    //display current
    out<<"Heaters: ";
    for(const Unit& s:r.m_heater){out<<s.getInput()<<" ";}
    out<<'\n';

    out<<"Fans: ";
    for(const Unit& s:r.m_fan){out<<s.getInput()<<" ";}
    out<<'\n';

    out<<"Humidifiers: ";
    for(const Unit& s:r.m_humidifier){out<<s.getInput()<<" ";}
    out<<'\n';

    out<<"CO2 Injectors: ";
    for(const Unit& s:r.m_co2inject){out<<s.getInput()<<" ";}
    out<<'\n';
    return out;
}

Room* Room::readAllUnits()
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

void Room::readSensors()
{
    //read the availble sensors
    std::ifstream sfile(SENSORS);
    if(!sfile)
    {
        throw std::runtime_error("Error: Failed to open Sensor Unit data file.");
    }
    json data;
    sfile>>data; //read the Unit file into the json object

    //lambda function to avoid doing the same thing for each sensor type
    auto updateSensors =[](std::vector<Sensor>& s, const json& j)
    {
        if(j.is_array())
        {
            for(size_t i=0;i<s.size() && i< j.size();++i)
            {
                s[i].updateInput(j[i]);
            }
        }
        else s[0].updateInput(j.get<float>());
    };

    updateSensors(m_temp,data["temperature"]);
    updateSensors(m_humidity,data["humidity"]);
    updateSensors(m_co2,data["co2"]);
}

void Room::writeUnits()
{
    std::ofstream ofile(OUTPUTS);   //open the file and automatically clear it
    if(!ofile)
    {
        throw std::runtime_error("Error: failed to open Control Unit data file.");
    }
    json data;
    int  n{1};

    if(m_heater.size()>1)
    {
        for(const Control& c:m_heater)
        {
            data["heater"+std::to_string(n)]=c.getInput();
            n++;
        }
    }
    else data["heater"]=m_heater[0].getInput();

    n=1;
    if(m_fan.size()>1)
    {
        for(const Control& c:m_fan)
        {
            data["fan"+std::to_string(n)]=c.getInput();
            n++;
        }
    }
    else data["fan"]=m_fan[0].getInput();

    n=1;
    if(m_humidifier.size()>1)
    {
        for(const Control& c:m_humidifier)
        {
            data["humidifier"+std::to_string(n)]=c.getInput();
            n++;
        }
    }
    else data["humidifier"]=m_humidifier[0].getInput();

    n=1;
    if(m_co2inject.size()>1)
    {
        for(const Control& c:m_co2inject)
        {
            data["co2_injector"+std::to_string(n)]=c.getInput();
            n++;
        }
    }
    else data["co2_injector"]=m_co2inject[0].getInput();

    ofile<<data.dump(4);
    ofile.close();

    std::cout<<"New controller settings updated\n\n";
}
