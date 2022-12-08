//
// https://stackoverflow.com/questions/728068/how-to-calculate-a-time-difference-in-c
//

#ifndef EVOMOTION_TIMER_H
#define EVOMOTION_TIMER_H

#include <chrono>
#include <string>

class Timer {
public:
    Timer();
    void reset();
    double elapsed() const;

private:
    typedef std::chrono::high_resolution_clock clock;
    typedef std::chrono::duration<double, std::ratio<1>> second;
    std::chrono::time_point<clock> beg;
};

#endif //EVOMOTION_TIMER_H
