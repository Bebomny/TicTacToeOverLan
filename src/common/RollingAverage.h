#ifndef TICTACTOEOVERLAN_ROLLINGAVERAGE_H
#define TICTACTOEOVERLAN_ROLLINGAVERAGE_H

/**
 * @brief A container for aggregated statistical data.
 * <br> Used to retrieve a snapshot of the current performance metrics in a single call.
 */
struct PackagedValues {
    double average;
    double min;
    double max;
};

/**
 * @brief Interface for calculating moving statistics over a stream of values.
 * <br> Primarily used for performance telemetry, such as server tick rates over a specific window of time.
 */
class RollingAverage {
public:
    RollingAverage() = default;

    virtual ~RollingAverage() = default;

    /**
     * @brief Calculates the mean of the values currently in the rolling window.
     *
     * @return The average value.
     */
    virtual double average() = 0;

    /**
     * @brief Retrieves the smallest value currently in the rolling window.
     *
     * @return The minimum value.
     */
    virtual double min() = 0;

    /**
     * @brief Retrieves the largest value currently in the rolling window.
     *
     * @return The maximum value.
     */
    virtual double max() = 0;

    /**
     * @brief Retrieves all statistical metrics in a single structure.
     * <br> This is preferred over calling average/min/max individually if you need
     * to display all of them (e.g., in a debug overlay), as it ensures the values
     * come from the exact same calculation state.
     *
     * @return A PackagedValues struct containing average, min, and max.
     */
    virtual PackagedValues getPackagedValues() = 0;
};

#endif //TICTACTOEOVERLAN_ROLLINGAVERAGE_H
