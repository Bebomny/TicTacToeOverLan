#ifndef TICTACTOEOVERLAN_DOUBLEROLLINGAVERAGE_H
#define TICTACTOEOVERLAN_DOUBLEROLLINGAVERAGE_H
#include <deque>
#include <mutex>

#include "RollingAverage.h"


class LongLongRollingAverage final : RollingAverage {
private:
    std::deque<long long> samples;
    int sampleSize;
    long long total = 0.0;
    mutable std::mutex mtx;

public:
    LongLongRollingAverage(int sampleSize);
    ~LongLongRollingAverage() override {}

    void add(long long value);

    double average() override;

    double min() override;

    double max() override;

    PackagedValues getPackagedValues() override;
};


#endif //TICTACTOEOVERLAN_DOUBLEROLLINGAVERAGE_H
