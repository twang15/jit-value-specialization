/* Author:
 * Pericles Alves [periclesrafael@dcc.ufmg.br]
 */

#ifdef MOZ_PROFILING

#include <algorithm>

#include "IonSpewer.h"
#include "IonProfiler.h"

using namespace js;
using namespace js::ion;

static IonProfiler ionprofiler; // Singleton instance.

void
ion::IonProfileNewScript(JSScript *script)
{
    ionprofiler.setCurrentScript(script);
}

void
ion::IonProfileStartTimer()
{
    ionprofiler.startTimer();
}

void
ion::IonProfileStopTimer()
{
    ionprofiler.stopTimer();
}

void
ion::IonProfileSpewTimer(const char *message)
{
    ionprofiler.spewTimer(message);
}

IonProfiler::IonProfiler()
{
    outputFile.open("/tmp/ionprofile.out");
    currentScript = NULL;
}

IonProfiler::~IonProfiler()
{
    outputFile.close();
}

void
IonProfiler::setCurrentScript(JSScript *script)
{
    currentScript = script;

    outputFile << "Script";
    outputFile << " ";
    outputFile << currentScript->filename << ":" << currentScript->lineno << std::endl;
}

void
IonProfiler::spewTimer(const char *message)
{
    double elapsedTime = stopTime.tv_sec - startTime.tv_sec + 1e-6 * (stopTime.tv_usec - startTime.tv_usec);

    std::string strMessage = message;

    replace(strMessage.begin(), strMessage.end(), ' ', '_'); // Eliminate spaces from the message.

    outputFile << strMessage;
    outputFile << " ";
    outputFile << std::fixed << elapsedTime << std::endl;
}

#endif /* MOZ_PROFILING */
