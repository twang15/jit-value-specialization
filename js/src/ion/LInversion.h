/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=79:
 *
 * Loop Inversion optimization pass
 *
 * autor: Igor Rafael <igor@dcc.ufmg.br>
 *
 */
#ifndef jsion_linversion_h__
#define jsion_linversion_h__

// This file represents Loop Inversion optimization pass
namespace js {
namespace ion {

class MIRGraph;
class LIRGraph;
class MBasicBlock;

class LInversion {
    union{
        MIRGraph *mir;
        LIRGraph *lir;
    }graph;

    bool isLIR;
    /*
    void eliminateEmptyLIRBlocks();
    void eliminateEmptyMIRBlocks();
    */
public:
    LInversion(MIRGraph &graph);
    LInversion(LIRGraph &graph);
//  static bool checkLoopCondition(MInstruction *cond);
//  void eliminateEmptyBlocks();
    void eliminateUnnecessaryIf();
    void analyzeEmptyBlocks();
};

} //namespace js
} //namespace ion

#endif // jsion_linversion_h__
