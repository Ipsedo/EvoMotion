//
// https://stackoverflow.com/questions/728068/how-to-calculate-a-time-difference-in-c
//

#include <sstream>
#include "timer.h"

Timer::Timer() : beg(clock::now()) {}

void Timer::reset() { beg = clock::now(); }

double Timer::elapsed() const {
        return std::chrono::duration_cast<second>(clock::now() - beg).count();
}
