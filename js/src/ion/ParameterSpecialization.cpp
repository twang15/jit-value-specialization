/*
 * ParameterSpecialization.cpp
 *
 *  Created on: Jun 6, 2012
 *      Author: igor
 */

#include "Ion.h"
#include "MIR.h"
#include "IonSpewer.h"
#include "ParameterSpecialization.h"


using namespace js;
using namespace js::ion;

ParameterSpecialization::ParameterSpecialization(JSContext *cx, JSScript *script)
{
    this->cx = cx;
    this->script = script;
    this->fp = cx->fp();
}

bool
ParameterSpecialization::extractArgs()
{
    if ((fp->hasArgs()) && (fp->isFunctionFrame())) {
        nargs = fp->numFormalArgs();
        args = fp->formals();

        return true;
    } else
        return false;
}

Value
ParameterSpecialization::getArg(unsigned i)
{
    JS_ASSERT(i < nargs);
    return args[i];
}

bool
ParameterSpecialization::canSpecialize(jsbytecode *osrPc)
{
    IonSpew(IonSpew_PS, "Beginning PS pass.");

    if (script->disabledForPS) {
        IonSpew(IonSpew_PS, "Skipped. The script has been disabled for parameter specialization.");
        return false;
    }

    if (!osrPc) {
        IonSpew(IonSpew_PS, "Skipped. The graph does not have an osr block.");
        
        script->disabledForPS = true;
        return false;
    }

    if (!extractArgs()) {
        IonSpew(IonSpew_PS, "Arguments retrieving failed. Maybe it is a global script.");
        
        script->disabledForPS = true;
        return false;
    }

    for (unsigned i = 0; i < nargs; i++) {
        if (getArg(i).isUndefined()) {
            IonSpew(IonSpew_PS, "Skipped. A parameter is undefined.");
            return false;
        }
    }

    return true;
}

bool
ParameterSpecialization::canSpecializeAtOsr()
{
    // If the parameters were replaced in the entry block, then they can
    // also be replaced in the OSR block.
    if (!(script->isParameterSpecialized)) {
        return false;
    }

    if (!extractArgs()) {
        IonSpew(IonSpew_PS, "[OSR] Arguments retrieving failed. Something bad happened during the graph construction.");
        
        script->disabledForPS = true;
        return false;
    }

    return true;
}

MConstant*
ParameterSpecialization::getConstantArg(unsigned i)
{
    Value value = getArg(i);
    MConstant *constant = MConstant::New(value);
    
    return constant;
}

MConstant*
ParameterSpecialization::getLocalValue(unsigned i)
{
    Value value = fp->slots()[i];
    MConstant *constant = MConstant::New(value);

    return constant;
}
