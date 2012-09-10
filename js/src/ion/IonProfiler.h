/* Author:
 * Pericles Alves [periclesrafael@dcc.ufmg.br]
 */

#ifndef jsion_profiler_h__
#define jsion_profiler_h__

#include <sys/time.h>
#include <time.h>

#include <iostream>
#include <fstream>

#include "../jsscript.h"

namespace js {
namespace ion {

/*
 * Very simple time profiler for IonMonkey (only available in debug mode).
 * Results are located at /tmp/ionprofile.out
 * Example of usage:
 *
 *     IonProfileStartTimer();
 *
 *     myInterestingFunction();
 *
 *     IonProfileStopTimer();
 *     IonProfileSpewTimer("Time spent in myInterestingFunction");
 */

#ifdef MOZ_PROFILING

/**
  * Ion Profiler main class.
  */
class IonProfiler
{
  private:
    std::ofstream outputFile;

    JSScript *currentScript; // Script being analyzed.

    // Time difference between two execution points.
    struct timeval startTime;
    struct timeval stopTime;

  public:
    IonProfiler();
    ~IonProfiler();

    void setCurrentScript(JSScript *script);

    // Starts execution time logging.
    inline void startTimer()
    {
        gettimeofday(&startTime, NULL);
    }
    
    // Stops execution time loggin.
    inline void stopTimer()
    {
        gettimeofday(&stopTime, NULL);
    }
    
    void spewTimer(const char *message); // Sends execution time info to the compiler spewer.
};

void IonProfileNewScript(JSScript *script);
void IonProfileStartTimer();
void IonProfileStopTimer();
void IonProfileSpewTimer(const char *message);

#else

static inline void IonProfileNewScript(JSScript *script)
{}
static inline void IonProfileStartTimer()
{}
static inline void IonProfileStopTimer()
{}
static inline void IonProfileSpewTimer(const char *message)
{}

#endif /* MOZ_PROFILING */

} // namespace ion
} // namespace js

#endif // jsion_profiler_h__
