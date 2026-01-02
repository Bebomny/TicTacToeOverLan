#include "LongLongRollingAverage.h"

#include <deque>
#include <limits.h>

LongLongRollingAverage::LongLongRollingAverage(int sampleSize) : sampleSize(sampleSize) {
    this->sampleSize = sampleSize;
    this->samples = std::deque<long long>();
}

void LongLongRollingAverage::add(long long value) {
    std::lock_guard<std::mutex> lock(this->mtx);

    this->total += value;
    this->samples.push_back(value);
    if (this->samples.size() > this->sampleSize) {
        this->total -= this->samples.front();
        this->samples.pop_front();
    }
}

double LongLongRollingAverage::average() {
    std::lock_guard<std::mutex> lock(this->mtx);

    if (this->samples.empty()) {
        return 0.0;
    }
    return this->total / this->samples.size();
}

double LongLongRollingAverage::min() {
    std::lock_guard<std::mutex> lock(this->mtx);

    double min = INT_MAX;
    for (long long &sample: samples) {
        min = std::min(static_cast<double>(sample), min);
    }
    return min;
}

double LongLongRollingAverage::max() {
    std::lock_guard<std::mutex> lock(this->mtx);

    double max = 0.0;
    for (long long &sample: samples) {
        max = std::max(static_cast<double>(sample), max);
    }
    return max;
}

PackagedValues LongLongRollingAverage::getPackagedValues() {
    std::lock_guard<std::mutex> lock(this->mtx);

    double average = 0.0, min = INT_MAX, max = INT_MIN;
    for (long long &sample: samples) {
        min = std::min(static_cast<double>(sample), min);
        max = std::max(static_cast<double>(sample), max);
    }
    average = this->total / this->samples.size();

    return {average, min, max};
}

