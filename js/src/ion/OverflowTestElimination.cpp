/*
 * OverflowTestElimination.cpp
 *
 *  Created on: Jun 21, 2012
 *      Author: igor
 */

#include "MIR.h"
#include "MIRGraph.h"
#include "IonSpewer.h"
#include "OverflowTestElimination.h"
#include <fstream>

using namespace js;
using namespace js::ion;
using namespace std;

OverflowTestElimination::OverflowTestElimination(MIRGraph &graph) : graph(graph) {
	// TODO Auto-generated constructor stub

}

OverflowTestElimination::~OverflowTestElimination() {
	// TODO Auto-generated destructor stub
}

void OverflowTestElimination::eliminateOverflowTests() {
	fstream fp_in("/tmp/ionote", ios::out | ios::app);
	MBasicBlockIterator end = graph.end();
	for(MBasicBlockIterator it = graph.begin(); it != end; it++){
		MBasicBlock *block = *it;
		MInstructionIterator iItEnd = block->end();
		for(MInstructionIterator iIt = block->begin(); iIt != iItEnd; iIt++){
			MInstruction *inst = *iIt;
			//FIXME: actually IM only analyzes overflow in this two operations
			if(inst->isUrsh()){
				MUrsh* ursh = inst->toUrsh();
				if(ursh->canOverflow()){

				}
			}else if(inst->isMul()){
				MMul* mul = inst->toMul();
				if(mul->canOverflow()){

				}
			}
		}
	}
	fp_in << graph.begin()->info().filename() << ":"<<graph.begin()->info().lineno();
}
