#ifndef TICTACTOEOVERLAN_ROLLINGAVERAGE_H
#define TICTACTOEOVERLAN_ROLLINGAVERAGE_H

struct PackagedValues {
    double average;
    double min;
    double max;
};

class RollingAverage {
public:
    RollingAverage() = default;
    virtual ~RollingAverage() = default;

    virtual double average() = 0;

    virtual double min() = 0;

    virtual double max() = 0;

    virtual PackagedValues getPackagedValues() = 0;
};

#endif //TICTACTOEOVERLAN_ROLLINGAVERAGE_H
