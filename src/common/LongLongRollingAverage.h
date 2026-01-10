#ifndef TICTACTOEOVERLAN_DOUBLEROLLINGAVERAGE_H
#define TICTACTOEOVERLAN_DOUBLEROLLINGAVERAGE_H
#include <deque>
#include <mutex>

#include "RollingAverage.h"

/**
 * @brief A thread-safe implementation of RollingAverage for `long long` integer types.
 * <br> Uses a deque (double-ended queue) to maintain a sliding window of the most recent samples.
 * <br> This implementation is  designed for timing values in nanoseconds stored as `long long`.
 */
class LongLongRollingAverage final : RollingAverage {
private:
    std::deque<long long> samples;
    int sampleSize;
    long long total = 0.0;
    mutable std::mutex mtx;

public:
    /**
     * @brief Constructs a rolling average tracker.
     *
     * @param sampleSize The number of historical data points to keep (window size).
     * Larger sizes give smoother averages but react slower to changes.
     */
    LongLongRollingAverage(int sampleSize);

    ~LongLongRollingAverage() override {}

    /**
     * @brief Adds a new sample to the window.
     * <br> If the window is full, the oldest sample is removed and subtracted from the `total`.
     * <br> Thread-safe: Locks the mutex during operation.
     *
     * @param value The new data point to record.
     */
    void add(long long value);

    /**
     * @brief Calculates the mean of the current samples.
     * <br> Thread-safe: Locks the mutex.
     *
     * @return The average value as a double, or 0.0 if no samples exist.
     */
    double average() override;

    /**
     * @brief Finds the minimum value in the current window.
     * <br> Thread-safe: Locks the mutex.
     *
     * @return The minimum value, or 0.0 if empty.
     */
    double min() override;

    /**
     * @brief Finds the maximum value in the current window.
     * <br> Thread-safe: Locks the mutex.
     *
     * @return The maximum value, or 0.0 if empty.
     */
    double max() override;

    /**
     * @brief Retrieves Average, Min, and Max atomically.
     * <br> Acquires the lock once and computes all three statistics to ensure consistency.
     *
     * @return A filled PackagedValues struct.
     */
    PackagedValues getPackagedValues() override;
};


#endif //TICTACTOEOVERLAN_DOUBLEROLLINGAVERAGE_H
