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

class TempFuzzyControl
{
private:
    FuzzyVariable<TempLabel> m_temp;
    std::vector<FuzzyRule<TempLabel,ControlLevel>> m_rules;

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
    TempFuzzyControl(float setTemp)     //enhance this using templates to make it work for co2 and humidity as well as temp depending on inputs
    :m_temp(),m_rules()
    {
        m_temp.addSet({TempLabel::Cold,0,setTemp-12,setTemp-2});
        m_temp.addSet({TempLabel::Normal,setTemp-4,setTemp,setTemp+4});
        m_temp.addSet({TempLabel::Hot,setTemp+2,setTemp+6,setTemp+14});

        m_rules.emplace_back(TempLabel::Cold,ControlLevel::High);
        m_rules.emplace_back(TempLabel::Normal,ControlLevel::Medium);
        m_rules.emplace_back(TempLabel::Hot,ControlLevel::Off);
    }

    float compute(float avt)
    {
        auto membership=m_temp.fuzzify(avt);
        float totalWeight=0;
        float totalValue=0;

        for (const auto& rule:m_rules)
        {
            float weight=membership[rule.input];
            float outputVal=levelToValue(rule.output);
            totalValue+=weight*outputVal;
            totalWeight+=weight;
        }
        return (totalWeight>0)?(totalValue/totalWeight):0;
    }
};


#endif // FUZZY_H_INCLUDED
