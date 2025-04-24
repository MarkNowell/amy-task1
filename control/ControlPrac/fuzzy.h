#ifndef FUZZY_H_INCLUDED
#define FUZZY_H_INCLUDED


enum class ControlLevel{Off,Low,Medium,High};
enum class TempLabel{Cold,Normal,Hot};
enum class HumidityLabel{Dry,Normal,Humid};
enum class Co2Label{Low,Normal,High};

template<typename LabelType>
class FuzzySet
{
    LabelType m_label;
    float m_start,m_peak,m_end;

public:
    FuzzySet(LabelType n, float s, float p,float e)
    :m_label(n),m_start(s),m_peak(p),m_end(e){}

    float membership(float x) const
    {
        if(x<=m_start || x>=m_end)return 0;
        if (x==m_peak)return 1;
        if(x<m_peak)return (x-m_start)/(m_peak-m_start);
        return (m_end-x)/(m_end-m_peak);
    }

    LabelType getLabel() const {return m_label;}
};

template<typename LabelType>
class FuzzyVariable
{
    std::vector<FuzzySet<LabelType>> m_sets;

public:
    FuzzyVariable()
    :m_sets(){}

    void addSet(const FuzzySet<LabelType>& s)
    {
        m_sets.emplace_back(s);
    }

    std::map<LabelType, float> fuzzify(float x) const
    {
        std::map<LabelType, float> results;
        for(const auto& set:m_sets)
        {
            results[set.getLabel()]=set.membership(x);
        }
        return results;
    }
};

template<typename InputLabel,typename OutputLabel>
struct FuzzyRule
{
    InputLabel input;
    OutputLabel output;

    FuzzyRule(InputLabel i, OutputLabel o)
    :input(i),output(o){}
};

struct TempFuzzyOutput
{
    ControlLevel heater;
    ControlLevel fan;

    TempFuzzyOutput(ControlLevel h,ControlLevel f)
    :heater(h),fan(f){};
};
struct TempOutPair
{
    float heaterVal;
    float fanVal;

    TempOutPair(float h,float f)
    :heaterVal(h),fanVal(f){}
};

class TempFuzzyControl
{
private:
    FuzzyVariable<TempLabel> m_temp;
    std::vector<FuzzyRule<TempLabel,TempFuzzyOutput>> m_rules;
    //integral gain values to try and make temp more acurate
    float m_integralError;
    float m_ki;
    float m_setTemp;

    float levelToValue(ControlLevel level) const
    {
        switch(level)
        {
            case ControlLevel::Off: return 0;
            case ControlLevel::Low: return 2;
            case ControlLevel::Medium: return 5;
            case ControlLevel::High: return 10;
        }
        return 0;
    }

public:
    TempFuzzyControl(float setTemp, float ki=0.1f)     //enhance this using templates to make it work for co2 and humidity as well as temp depending on inputs
    :m_temp(),m_rules(),m_integralError(0),m_ki(ki),m_setTemp(setTemp)
    {
        m_temp.addSet({TempLabel::Cold,0,setTemp-12,setTemp-2});
        m_temp.addSet({TempLabel::Normal,setTemp-4,setTemp,setTemp+4});
        m_temp.addSet({TempLabel::Hot,setTemp+2,setTemp+6,setTemp+14});

        m_rules.emplace_back(TempLabel::Cold,TempFuzzyOutput(ControlLevel::High,ControlLevel::Off));
        m_rules.emplace_back(TempLabel::Normal,TempFuzzyOutput(ControlLevel::Medium,ControlLevel::Low));
        m_rules.emplace_back(TempLabel::Hot,TempFuzzyOutput(ControlLevel::Off,ControlLevel::High));
    }

    void updateTarget(float newTarget)
    {
        m_setTemp=newTarget;
        m_temp=FuzzyVariable<TempLabel>();

        m_temp.addSet({TempLabel::Cold,0,newTarget-12,newTarget-2});
        m_temp.addSet({TempLabel::Normal,newTarget-4,newTarget,newTarget+4});
        m_temp.addSet({TempLabel::Hot,newTarget+2,newTarget+6,newTarget+14});
    }
    TempOutPair compute(float avt)
    {
        //error accumulation
        float error=m_setTemp-avt;
        m_integralError+=error;

        auto membership=m_temp.fuzzify(avt);
        float htotalWeight=0;
        float htotalValue=0;
        float ftotalWeight=0;
        float ftotalValue=0;

        for (const auto& rule:m_rules)
        {
            float hweight=membership[rule.input];
            float houtputVal=levelToValue(rule.output.heater);
            htotalValue+=hweight*houtputVal;
            htotalWeight+=hweight;

            float fweight=membership[rule.input];
            float foutputVal=levelToValue(rule.output.fan);
            ftotalValue+=fweight*foutputVal;
            ftotalWeight+=fweight;
        }

        float heaterOut=(htotalWeight>0)?(htotalValue/htotalWeight):0;
        float fanOut=(ftotalWeight>0)?(ftotalValue/ftotalWeight):0;

        //add integral correction to heater
        heaterOut+=m_ki*m_integralError;
        //limit heater output between 0 and 10
        if(heaterOut>10.0)heaterOut=10.0;
        else if (heaterOut<0.0)heaterOut=0;

        return TempOutPair(heaterOut,fanOut);
    }
};


#endif // FUZZY_H_INCLUDED
