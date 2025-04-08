#ifndef ROOM_H_INCLUDED
#define ROOM_H_INCLUDED

//create a Unit class that is generic to any type of Unit we use
class Unit
{
protected:
    float m_input{};

public:
    Unit(float in=0.0)
    :m_input{in}{}

    float getInput()const {return m_input;}
};

class Sensor:public Unit
{
public:
    Sensor(float in=0.0)
    :Unit(in){}

};

class Control:public Unit
{
public:
    Control(float in=0.0)
    :Unit(in){}

};


//each room is made up of a number of Units
class Room
{
    //sensors
    std::vector<Sensor> m_temp{10};
    std::vector<Sensor> m_humidity{10};
    std::vector<Sensor> m_co2{10};
    //control units
    std::vector<Control> m_heater{10};
    std::vector<Control> m_fan{10};
    std::vector<Control> m_humidifier{10};
    std::vector<Control> m_co2inject{10};

public:
    Room(std::vector<Sensor> t,std::vector<Sensor> h,std::vector<Sensor> c,
         std::vector<Control> heat,std::vector<Control> fan,std::vector<Control> humid,std::vector<Control> co2inject)
    :m_temp{t},m_humidity{h},m_co2{c},
    m_heater{heat},m_fan{fan},m_humidifier{humid},m_co2inject{co2inject}
    {}

    static Room* readUnits();
    friend std::ostream& operator<<(std::ostream& out, const Room& r);
};


#endif // ROOM_H_INCLUDED
