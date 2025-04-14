#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED

class Controller
{
public:
    virtual float compute(float setpoint, float actual)=0;
    virtual ~Controller(){}     //virtual destructor to make sure there are no leaks
};

class PController: public Controller
{
    float kp;

public:
    PController(float kpin)
    :kp(kpin){}

    virtual float compute(float setpoint, float actual) override
    {
        return kp*(setpoint-actual);
    }
};

class PIDController:public Controller
{
    float kp;
    float ki;
    float kd;
    float integral=0;
    float last_error=0;

public:
    PIDController(float kpin, float kiin, float kdin)
    :kp(kpin),ki(kiin),kd(kdin){}

    virtual float compute(float setpoint, float actual)override
    {
        float error=setpoint-actual;
        integral+=error;
        float derivative = error-last_error;

        return kp*error+ki*integral+kd*derivative;
    }
};

#endif // CONTROLLER_H_INCLUDED
