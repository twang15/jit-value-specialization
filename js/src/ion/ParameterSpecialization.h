/*
 * ParameterSpecialization.h
 *
 *  Created on: Jun 6, 2012
 *      Author: igor
 */

#ifndef PARAMETERSPECIALIZATION_H_
#define PARAMETERSPECIALIZATION_H_

namespace js {
namespace ion{

class MConstant;

class ParameterSpecialization{
  private:
    JSContext *cx;
    JSScript *script;

    StackFrame *fp;
    unsigned nargs;
    Value* args;

    bool extractArgs();
    Value getArg(unsigned i);

  public:
    ParameterSpecialization(JSContext *cx, JSScript *script);
    
    // Indicates if parameter based specialization can be applied to the
    // current function.
    bool canSpecialize(jsbytecode *osrPc);
    bool canSpecializeAtOsr();

    void disable() {
        script->disabledForPS = true;
    }

    // Retrieves the constant instruction corresponding to the argument
    // value in the interpreter stack.
    MConstant *getConstantArg(unsigned i);
    MConstant *getLocalValue(unsigned i);
};

}
}
#endif /* PARAMETERSPECIALIZATION_H_ */
