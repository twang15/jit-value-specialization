/*
 * OverflowTestElimination.h
 *
 *  Created on: Jun 21, 2012
 *      Author: igor
 */

#ifndef OVERFLOWTESTELIMINATION_H_
#define OVERFLOWTESTELIMINATION_H_

// This file represents Loop Inversion optimization pass
namespace js {
namespace ion {


class MIRGraph;

class OverflowTestElimination {
	MIRGraph &graph;
public:
	OverflowTestElimination(MIRGraph &graph);
	virtual ~OverflowTestElimination();
	void eliminateOverflowTests();
};

}
}

#endif /* OVERFLOWTESTELIMINATION_H_ */
