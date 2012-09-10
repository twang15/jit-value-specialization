/* Authors:
 * Igor Rafael [igor@dcc.ufmg.br]
 * Pericles Alves [periclesrafael@dcc.ufmg.br]
 */

#include "Ion.h"
#include "IonSpewer.h"
#include "CP.h"
#include "MIR.h"
#include "MIRGraph.h"

using namespace js;
using namespace js::ion;
using namespace js::ion::cp;

CP::CP(MIRGraph &graph)
  : graph(graph)
{
#ifdef DEBUG
    actualScript = NULL;
    numFolded = 0;
    failsDueToType = 0;
    totalInst = 0;
#endif
}

#ifdef DEBUG
#define PERCENT(n,d) (d==0 ? 0 : n/d),(d == 0 || n == d ? 0 : (int)(100*((double)n)/d))
inline void CP::printStats(){
    JS_ASSERT(actualScript != NULL);
    SAVE_STATS(actualScript,numFolded,totalInst);
}
#endif

bool
CP::isBlacklisted(MInstruction *ins)
{
    if(ins->isPassArg()
        || ins->isCall()
        || ins->isCreateThis())
        return true;
    return false;
}

bool
CP::initWorkList()
{
    //Default implementation: Adds uses of constant instructions in the graph to the work list
    for (MBasicBlockIterator block(graph.begin()); block != graph.end(); block++) {
#ifdef DEBUG
        for (MPhiIterator phi(block->phisBegin()); phi != block->phisEnd(); phi++)
            countInst();
#endif
        for (MInstructionIterator instIt(block->begin()); instIt != block->end(); instIt++) {
            countInst();
        
            MInstruction *instruction = *instIt;

            if (instruction->isConstant() && !isBlacklisted(instruction)) {
                addUsesToWorkList(instruction);
            }
        }
    }

    return true;
}

bool
CP::analyze()
{
    bool updated = false;
    IonSpew(IonSpew_CP, "Beginning optimization pass.");
    if(!initWorkList())
        return false;

    //Definitions are propagated until there are no more phi cycles to be solved
    do {
        while (!workList.empty()) {
            //Gets a new element from the work list
            MDefinition *def = *(workList.begin());
            workList.erase(workList.begin());

            if (IonSpewEnabled(IonSpew_CP)) {
                IonSpewHeader(IonSpew_CP);
                fprintf(IonSpewFile, "Analyzing [");
                def->printOpcode(IonSpewFile);
                fprintf(IonSpewFile, "]\n");
            }

            MDefinition *foldResult = def->foldsToConstant();

            //Propagates a folded definition
            if ((foldResult != def) && (foldResult->isInstruction())) {
                MInstruction *resultInstruction = foldResult->toInstruction();
                //Uses may be constant propagable too
                addUsesToWorkList(def);

                if (def->isPhi()) {
                    MPhi *phi = def->toPhi();
                    MBasicBlock *block = def->block();
                    uint32 slot = phi->slot();

                    def->replaceWithInstruction(resultInstruction);

                    for (MBasicBlockIterator itBlock = graph.begin(block); itBlock != graph.end(); itBlock++)
                        if (slot < itBlock->stackDepth() && itBlock->getSlot(slot) == def)
                            itBlock->initSlot(slot, resultInstruction);
                } else {
                    def->replaceWithInstruction(resultInstruction);
                }
                updated = true;
                IonSpew(IonSpew_CP, "Definition %d folded.", def->id());

//                if (IonSpewEnabled(IonSpew_CP)) {
//                    IonSpewHeader(IonSpew_CP);
//                    def->printOpcode(IonSpewFile);
//                    fprintf(IonSpewFile, " --> ");
//                    resultInstruction->printOpcode(IonSpewFile);
//                    fprintf(IonSpewFile, "\n");
//                }

                countNumFolded();

            //A non-propagated phi can be part of a phi cycle
            } else if (def->isPhi()) {
                phiMap.insert(std::make_pair(def->toPhi(), PhiItem()));
            }
        }

        solvePhiCycles();
    } while (!workList.empty());

    printStats();
    IonSpew(IonSpew_CP, "Constant Propagation END.");
    return updated;
}

void 
CP::addUsesToWorkList(MDefinition *def)
{
    for (MUseIterator useIt(def->usesBegin()); useIt != def->usesEnd(); useIt++) {
        MUse *use = *useIt;
        
        if (use->node()->isDefinition()) {
            MDefinition *def = use->node()->toDefinition();
            if(!def->isInstruction() || !isBlacklisted(def->toInstruction()))
                workList.insert(use->node()->toDefinition());
        }
    } 
}

void
CP::addPhiUsesToMap(MPhi *phi)
{
    for (MUseIterator useIt(phi->usesBegin()); useIt != phi->usesEnd(); useIt++) {
        MUse *use = *useIt;

        if (use->node()->isDefinition()) {
            MDefinition *def = use->node()->toDefinition();
            if (def->isPhi()) {
                phiMap.insert(std::make_pair(def->toPhi(), PhiItem()));
            }
        }
    }
}

bool
CP::analyzePhi(MPhi *phi, PhiItem &pi) {
    bool updated = false;
    for (size_t i = 0; i < phi->numOperands(); ++i) {
        MDefinition* operand = phi->getOperand(i);
//        IonSpew(IonSpew_CP, "\tAnalyzing operand[%d]=def%d:", i, operand->id());
        if (operand->isConstant()) {
//            IonSpew(IonSpew_CP, "\t\tisConstant");
            PhiItem opr = PhiItem(Constant, operand->toConstant());
            updated = propagate(pi, opr) || updated;
        } else if(operand->isPhi()) {
//            IonSpew(IonSpew_CP, "\t\tisPhi");
            MPhi *operandToPhi = operand->toPhi();
            std::pair<std::map<MPhi*, PhiItem>::iterator, bool>
                phiOperandInMap = phiMap.insert(std::make_pair(operandToPhi, PhiItem()));
            PhiItem & piOperand = phiOperandInMap.first->second;
            updated = propagate(pi, piOperand) || updated;
        } else {
            PhiItem opr = PhiItem(Top, NULL);
            updated = propagate(pi, opr) || updated;
        }
    }
    return updated;
}

void
CP::solvePhiCycles(){

    bool updated;
    do {
        updated = false;
        std::map<MPhi*, PhiItem>::iterator it;
        for (it = phiMap.begin(); it != phiMap.end(); it++) {
            PhiItem& pi = it->second;
            MPhi *phi = it->first;

//            State lastState = pi.s;
//            if (pi.s == Bottom) {
//                addPhiUsesToMap(phi);
//                updated = true;
//            }

            updated |= analyzePhi(phi, pi);
  //          JS_ASSERT(lastState != Bottom || pi.s != Top);
       }
    } while (updated);

    //only for debug
    int b = 0, c = 0, t = 0;
    std::map<MPhi*, PhiItem>::iterator it, itEnd = phiMap.end();
    for(it = phiMap.begin(); it != itEnd; it++){
        MPhi *phi = it->first;
        PhiItem &pi = it->second;

        if (pi.s == Constant) {
//            if (IonSpewEnabled(IonSpew_CP)) {
//                IonSpew(IonSpew_CP, "\tphi%d is constant!", phi->id());
//                phi->printOpcode(IonSpewFile);
//                fprintf(IonSpewFile, "\n");
//             }
            JS_ASSERT(pi.c != NULL);
            for (size_t i = 0; i < phi->numOperands(); ++i) {
                phi->replaceOperand(i, pi.c);
            }
            this->workList.insert(phi);
        }
        ANALYSE_STATE(pi.s,b,c,t);
    }
    IonSpew(IonSpew_CP, "\tPhi Cycles [Bottom,Constant,Top] = [%d,%d,%d]", b, c, t);

    phiMap.clear();
}

bool 
CP::propagate(cp::PhiItem &to, cp::PhiItem &from)
{
//    if (IonSpewEnabled(IonSpew_CP)) {
//        fprintf(IonSpewFile, "\n");
//        IonSpewHeader(IonSpew_CP);
//        fprintf(IonSpewFile, "\tto: ");
//        switch (to.s) {
//            case Bottom: fprintf(IonSpewFile, "B");    break;
//            case Constant: fprintf(IonSpewFile, "C");    break;
//            case Top: fprintf(IonSpewFile, "T");    break;
//        }
//        fprintf(IonSpewFile, "\tfrom: ");
//        switch (from.s) {
//            case Bottom: fprintf(IonSpewFile, "B");    break;
//            case Constant: fprintf(IonSpewFile, "C");    break;
//            case Top: fprintf(IonSpewFile, "T");    break;
//        }
//    }
    switch (to.s) {
      case Bottom:
//        if (IonSpewEnabled(IonSpew_CP)) {
//            fprintf(IonSpewFile, "\n");
//            IonSpewHeader(IonSpew_CP);
//            switch (from.s) {
//                case Bottom: fprintf(IonSpewFile, "\tB -> B");    break;
//                case Constant: fprintf(IonSpewFile, "\tB -> C");    break;
//                case Top: fprintf(IonSpewFile, "\tB -> T");    break;
//            }
//        }
        if(from.s != Bottom){
            to.s = from.s;
            to.c = from.c;
            return true;
        }
        break;
      case Constant:
        JS_ASSERT(to.c != NULL);
        if(from.s == Top ||
          (from.s == Constant && from.c->value() != to.c->value()))
        {
            to.s = Top;
            to.c = NULL;
            return true;
        }
        break;
      case Top:
        break;
    }
    return false;
}
