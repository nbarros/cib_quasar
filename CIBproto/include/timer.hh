/*
 * timer.hh
 *
 *  Created on: Apr 13, 2023
 *      Author: nbarros
 */

#ifndef CIBPROTO_INCLUDE_TIMER_HH_
#define CIBPROTO_INCLUDE_TIMER_HH_

#include <iostream>
#include <chrono>

class Timer
{
public:
    Timer() : start(clock_t::now()) {}
    void reset() { start = clock_t::now(); }
    double elapsed() const {
        return std::chrono::duration_cast<second_t>(clock_t::now() - start).count();
    }

private:
    typedef std::chrono::high_resolution_clock clock_t;
    typedef std::chrono::duration<double, std::ratio<1> > second_t;
    std::chrono::time_point<clock_t> start;
};



#endif /* CIBPROTO_INCLUDE_TIMER_HH_ */
