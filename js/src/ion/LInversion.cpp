/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=79:
 *
 * Loop Inversion optimization pass
 *
 * autor: Igor Rafael <igor@dcc.ufmg.br>
 *
 */


#include "MIR.h"
#include "MIRGraph.h"
#include "IonSpewer.h"
#include "LInversion.h"
#include <fstream>

using namespace js;
using namespace js::ion;
using namespace std;

LInversion::LInversion(MIRGraph &graph): isLIR(false){
    this->graph.mir = &graph;
}

LInversion::LInversion(LIRGraph &graph): isLIR(true){
    this->graph.lir = &graph;
}


/*
bool
LInversion::checkLoopCondition(MInstruction *cond){
    JS_ASSERT(cond->isCompare() || cond->isConstant());
    return false;
}
*/
void
LInversion::analyzeEmptyBlocks(){
    fstream fp_in("/tmp/ionlinv", ios::out | ios::app);
    MBasicBlockIterator end = graph.mir->end();
    for(MBasicBlockIterator it = graph.mir->begin(); it != end; it++){
        MBasicBlock *block = *it;
        MInstructionIterator iItEnd = block->end();
        int numInst = 0;
        for(MInstructionIterator iIt = block->begin(); iIt != iItEnd; iIt++){
            numInst++;
        }
        fp_in << graph.mir->begin()->info().filename() << ":" << graph.mir->begin()->info().lineno()
            << "," << block->id() << "," << numInst << "," << block->numPredecessors()
            << "," << block->numSuccessors() << "\n";
    }

    fp_in.close();
}

/*
 * Try to ensure that the loop will be executed at least one.
 * If so eliminate the surrounding if.
 * only enabled with --ion-ps
 *
 */
void
LInversion::eliminateUnnecessaryIf(){

}

/*
void
LInversion::eliminateEmptyLIRBlocks(){
    //graph.lir->
}

void
LInversion::eliminateEmptyMIRBlocks(){
    MBasicBlockIterator end = graph.mir->end();
    FixedList<MBasicBlock*> blocksToDelete;
    int numBlocksToDelete=0;
    blocksToDelete.init(graph.mir->numBlockIds());
    for(MBasicBlockIterator it = graph.mir->begin(); it != end; it++){
        MBasicBlock *block = *it;
        IonSpew(IonSpew_LInv, "Analyzing block %d", block->id());
        MInstructionIterator iIt = block->begin();
        MPhiIterator itPhis = block->phisBegin();
        //if block has only one instruction
        iIt++;
        if(iIt == block->end() && itPhis == block->phisEnd()){
            IonSpew(IonSpew_LInv, "block %d has only one instruction and no phis", block->id());
            if(block->lastIns()->isGoto()){
                for(uint32 i = 0; i < block->numPredecessors(); i++){
                    MBasicBlock *pred = block->getPredecessor(i);
                    MBasicBlock *succ = block->getSuccessor(0);
                    MInstruction *lastIns = pred->lastIns();
                    if(lastIns->isGoto()){
                        //replace last instruction with a goto to succ
                        pred->discardLastIns();
                        pred->end(MGoto::New(succ));
                    }else if(lastIns->isTest()){
                        //replace last instruction with a copy that replace block with succ
                        MTest *oldLastIns = lastIns->toTest();
                        MTest *newLastIns;// = MTest::New(NULL, oldLastIns->if)
                        if(oldLastIns->ifTrue() == block) {
                            newLastIns = MTest::New(oldLastIns->getOperand(0), succ ,oldLastIns->ifFalse());
                        } else {
                            JS_ASSERT(oldLastIns->ifFalse() == block);
                            newLastIns = MTest::New(oldLastIns->getOperand(0), oldLastIns->ifTrue(), succ);
                        }
                        pred->discardLastIns();
                        pred->end(newLastIns);
                    }
                    //replace block by pred in succ list of predecessors
                    succ->replacePredecessor(block,pred);
                    blocksToDelete[numBlocksToDelete++] = block;
                }
            }else
                JS_NOT_REACHED("FIXME: unexpected behavior");
        }
    }

    for(int i = 0; i < numBlocksToDelete; i++) {
        MBasicBlock *toDelete = blocksToDelete[i];
        IonSpew(IonSpew_LInv, "block %d was discarded", toDelete->id());
        graph.mir->removeBlock(toDelete);
    }
}

void
LInversion::eliminateEmptyBlocks() {
    if(isLIR)
        eliminateEmptyLIRBlocks();
    else
        eliminateEmptyMIRBlocks();
}*/
