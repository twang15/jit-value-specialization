/* Authors:
 * Igor Rafael [igor@dcc.ufmg.br]
 * Pericles Alves [periclesrafael@dcc.ufmg.br]
 */ 

#include "InductionVariable.h"

using namespace js;
using namespace js::ion;

InductionVariable::InductionVariable(MBasicBlock *block)
{
    this->block = block;
    variable_ = NULL;

    hasLowerBound_ = false;
    hasUpperBound_ = false;
}

bool
InductionVariable::extractPattern()
{
    bool match = false;

    // The patterns are arranged from the most common to the less common.

    if (extractUBCSPattern() || extractUBCSPatternDoWhile())
        match = true;
    
    if (!match)
        return false;

    if (!variable_)
        return false;

    return true;
}

bool
InductionVariable::extractUBCSPattern()
{
    // Upper Bounded Continuous Sum pattern.
    // e.g.: "for (i = <lower_bound>; i </<= <upper_bound>; i += <something>)".
    // e.g.: "i = <lower_bound>; while (i </<= <upper_bound>) { ... i += <something>; }"

    if (!(block->isLoopHeader()))
        return false;
 
    // The last instruction of the block must be a boolean test (MTest).
    if (!block->lastIns()->isTest())
        return false;
        
    MTest *test = block->lastIns()->toTest();
    
    if (!test->getOperand(0)->isCompare())
        return false;
        
    MCompare *compare = test->getOperand(0)->toCompare();

    // The boolean test must be a "lt" or "le" comparision. 
    if ((compare->jsop() != JSOP_LT) && (compare->jsop() != JSOP_LE))
        return false;
        
    // The left comparision operand must be one of the following kinds of
    // instructions:
    // - a phi.
    // - an integer specialization (toInt32) of a phi.
    // - an unbox of a phi.
    // - a box of a phi.
    MDefinition *compareLhs = compare->lhs();
    MPhi *phi = NULL;

    // In case it is a phi.
    if (compareLhs->isPhi())
        phi = compareLhs->toPhi();

    // In case it is an integer specialization of a phi.
    else if (compareLhs->isToInt32()) {
        if (compareLhs->toToInt32()->input()->isPhi())
            phi = compareLhs->toToInt32()->input()->toPhi();

    // in case it is an unbox of a phi.
    } else if (compareLhs->isUnbox()) {
        if (compareLhs->toUnbox()->input()->isPhi())
            phi = compareLhs->toUnbox()->input()->toPhi();

    // In case it is an box of a phi.
    } else if (compareLhs->isBox()) {
        if (compareLhs->toBox()->getOperand(0)->isPhi())
            phi = compareLhs->toBox()->getOperand(0)->toPhi();
    }

    if (!phi)
        return false;

    if (phi->numOperands() != 2)
        return false;   
        
    // One of the phi operands must be one of the following kinds of
    // instructions:
    // - an add.
    // - an integer specialization (toInt32) of an add.
    // - an unbox of an add.
    // - a box of an add.
    // and its block must be a loop back edge.
    MDefinition *phiOperand0 = phi->getOperand(0);
    MDefinition *phiOperand1 = phi->getOperand(1);
    MAdd *add = NULL; // The add instruction used to increase the induction variable value.
    MDefinition *mayBeLowerBound = NULL; // Possible lower bound for the induction variable.

    // In case it is an add.
    if (phiOperand0->isAdd() && phiOperand0->block()->isLoopBackedge()) {
        add = phiOperand0->toAdd();
        mayBeLowerBound = phiOperand1;
    } else if (phiOperand1->isAdd() && phiOperand1->block()->isLoopBackedge()) {
        add = phiOperand1->toAdd();
        mayBeLowerBound = phiOperand0;

    // In case it is an integer specialization of an add.
    } else if(phiOperand0->isToInt32()) {
        if ((phiOperand0->toToInt32()->input()->isAdd()) && (phiOperand0->block()->isLoopBackedge())) {
            add = phiOperand0->toToInt32()->input()->toAdd();
            mayBeLowerBound = phiOperand1;
        }
    } else if(phiOperand1->isToInt32()) {
        if ((phiOperand1->toToInt32()->input()->isAdd()) && (phiOperand1->block()->isLoopBackedge())) {
            add = phiOperand1->toToInt32()->input()->toAdd();
            mayBeLowerBound = phiOperand0;
        }

    // In case it is an unbox of an add.
    } else if(phiOperand0->isUnbox()) {
        if ((phiOperand0->toUnbox()->input()->isAdd()) && (phiOperand0->block()->isLoopBackedge())) {
            add = phiOperand0->toUnbox()->input()->toAdd();
            mayBeLowerBound = phiOperand1;
        }
    } else if(phiOperand1->isUnbox()) {
        if ((phiOperand1->toUnbox()->input()->isAdd()) && (phiOperand1->block()->isLoopBackedge())) {
            add = phiOperand1->toUnbox()->input()->toAdd();
            mayBeLowerBound = phiOperand0;
        }

    // In case it is a box of an add.
    } else if(phiOperand0->isBox()) {
        if ((phiOperand0->toBox()->getOperand(0)->isAdd()) && (phiOperand0->block()->isLoopBackedge())) {
            add = phiOperand0->toBox()->getOperand(0)->toAdd();
            mayBeLowerBound = phiOperand1;
        }
    } else if(phiOperand1->isBox()) {
        if ((phiOperand1->toBox()->getOperand(0)->isAdd()) && (phiOperand1->block()->isLoopBackedge())) {
            add = phiOperand1->toBox()->getOperand(0)->toAdd();
            mayBeLowerBound = phiOperand0;
        }
    }

    if (!add)
        return false;

    // The add must ba a sum of the induction variable with a positive
    // integer constant.
    MConstant *increment = NULL;

    if (add->lhs()->isConstant())
        increment = add->lhs()->toConstant();
    else if (add->rhs()->isConstant())
        increment = add->rhs()->toConstant();

    if (!increment)
        return false;

    if (!increment->value().isInt32())
        return false;

    if (increment->value().toInt32() < 0)
        return false;

    // If all conditions are satisfied, the phi is the induction variable.
    variable_ = phi;

    // Extracts the lower bound.
    if (mayBeLowerBound) {
        Value value = UndefinedValue(); // The actual lower bound.

        if (mayBeLowerBound->isConstant())
            value = mayBeLowerBound->toConstant()->value();

        // The lower bound can be extracted even if we have a phi of two
        // constants.
        else if (mayBeLowerBound->isPhi() && (mayBeLowerBound->numOperands() == 2)) {
            MPhi *lowerPhi = mayBeLowerBound->toPhi();

            if (lowerPhi->getOperand(0)->isConstant() && lowerPhi->getOperand(1)->isConstant()) {
                Value lPhiLhs = lowerPhi->getOperand(0)->toConstant()->value();
                Value lPhiRhs = lowerPhi->getOperand(1)->toConstant()->value();

                if (lPhiLhs.isInt32() && lPhiRhs.isInt32()) {
                    if (lPhiLhs.toInt32() <= lPhiRhs.toInt32())
                        value = lPhiLhs;
                    else
                        value = lPhiRhs;
                }
            }

        }

        if (value.isInt32()) {
            int32 lowerBound = value.toInt32();
            
            if (lowerBound >= 0) {
                hasLowerBound_ = true;
                range.lowerBound = lowerBound;
            }
        }
    }

    // Extracts the upper bound.
    if (compare->rhs()->isConstant()) {
        Value value = compare->rhs()->toConstant()->value();

        if (value.isInt32()) {
            int32 upperBound = value.toInt32();

            if (upperBound >= 0) {
                hasUpperBound_ = true;

                // The upper bound must guarantee the LT relation.
                if (compare->jsop() == JSOP_LE)
                    range.upperBound = (upperBound - 1);
                else
                    range.upperBound = upperBound;
            }
        }
    }

    return true;
}

bool
InductionVariable::extractUBCSPatternDoWhile() {
    // Upper Bounded Continuous Sum pattern.
    // e.g: "i = <lower_bound>; do { ... i += <something> } while (i </<= <upper_bound);

    if (!(block->isLoopHeader()))
        return false;

    // Save the header an work on the its backedge.
    MBasicBlock *header = block;
    block = block->backedge()->getPredecessor(0);
 
    // The last instruction if the block must be a boolean test (MTest).
    if (!block->lastIns()->isTest())
        return false;
        
    MTest *test = block->lastIns()->toTest();
    
    if (!test->getOperand(0)->isCompare())
        return false;
        
    MCompare *compare = test->getOperand(0)->toCompare();

    // The boolean test must be a "lt" or "le" comparision. 
    if ((compare->jsop() != JSOP_LT) && (compare->jsop() != JSOP_LE))
        return false;

    // The left comparision operand must be an add. 

    MDefinition *compareLhs = compare->lhs();

    if (!compare->rhs()->isConstant())
       return false;

    if (!compareLhs->isAdd())
        return false;

    MAdd *add = compareLhs->toAdd();

    // The left operand of the add must be:
    // - a phi.
    // - an integer specialization (toInt32) of a phi.
    // - an unbox of a phi.
    // - a box of a phi.
    MDefinition *addLhs = add->getOperand(0); 
    MPhi *phi = NULL;

    if (addLhs->isPhi()) {
        phi = addLhs->toPhi();
    } else if (addLhs->isToInt32()) {
        if (addLhs->toToInt32()->input()->isPhi())
            phi = addLhs->toToInt32()->input()->toPhi();
    } else if (addLhs->isUnbox()) {
        if (addLhs->toUnbox()->input()->isPhi())
            phi = addLhs->toUnbox()->input()->toPhi();
    } else if (addLhs->isBox()) {
        if (addLhs->toBox()->getOperand(0)->isPhi())
            phi = addLhs->toBox()->getOperand(0)->toPhi();
    }

    if (!phi)
        return false;

    if (phi->numOperands() != 2)
        return false;   

    // The phi's first operand can be:
    // - a phi with the same conditions except for this one. 
    // - a box of a constant.
    // - an unbox of a constant.
    // - a constant.

    MPhi *otherPhi;
    if (phi->getOperand(0)->isPhi())
        otherPhi = phi->getOperand(0)->toPhi();
    else
        otherPhi = phi;

    MDefinition *phiOper0 = otherPhi->getOperand(0);
    MConstant *lowerBound = NULL;

    if (phiOper0->isBox() && phiOper0->getOperand(0)->isConstant())
        lowerBound = phiOper0->getOperand(0)->toConstant();
    else if (phiOper0->isConstant())
        lowerBound = phiOper0->toConstant();

    if (!lowerBound)
        return false;

    // The add must ba a sum of the induction variable with a positive
    // integer constant.
    MConstant *increment = NULL;

    if (add->lhs()->isConstant())
        increment = add->lhs()->toConstant();
    else if (add->rhs()->isConstant())
        increment = add->rhs()->toConstant();

    if (!increment)
        return false;

    if (!increment->value().isInt32())
        return false;

    if (increment->value().toInt32() < 0)
        return false;

    // If all conditions are satisfied, the phi is the induction variable.
    variable_ = phi;
 
    Value value = lowerBound->toConstant()->value();
    if (value.isInt32()) {
        int32 lowerBound = value.toInt32();
        if (lowerBound >= 0) {
            hasLowerBound_ = true;
            range.lowerBound = lowerBound;
        }
    }

    if (compare->rhs()->isConstant()) {
        Value value = compare->rhs()->toConstant()->value();

        if (value.isInt32()) {
            int32 upperBound = value.toInt32();

            if (upperBound >= 0) {
                hasUpperBound_ = true;

                // The upper bound must guarantee the LT relation.
                if (compare->jsop() == JSOP_LE)
                    range.upperBound = (upperBound - 1);
                else
                    range.upperBound = upperBound;
            }
        }
    }
    isDoWhile_ = true;
        
    return true;
}

MDefinition*
InductionVariable::variable()
{
    return variable_;
}

bool
InductionVariable::rangeIsValid(MBasicBlock *block)
{
    if (!variable_)
        return false;

    // The induction variable range is valid only within the loop body,
    // therefore, we must have: headerId < blockId <= backEdgeId.

    if (!variable_->block()->isLoopHeader())
        return false;

    uint32 blockId = block->id();
    uint32 headerId = variable_->block()->id();
    uint32 backEdgeId = variable_->block()->backedge()->id();

    if ((headerId < blockId) && (blockId <= backEdgeId))
        return true;
    if (isDoWhile() && (headerId <= blockId) && (blockId <= backEdgeId))
        return true;

    return false;
}

bool
InductionVariable::hasLowerBound()
{
    return hasLowerBound_;
}

bool
InductionVariable::hasUpperBound()
{
    return hasUpperBound_;
}

int32
InductionVariable::lowerBound()
{
    JS_ASSERT(hasLowerBound_);

    return range.lowerBound;
}

int32
InductionVariable::upperBound()
{
    JS_ASSERT(hasUpperBound_);

    return range.upperBound;
}

bool
InductionVariable::isDoWhile() {
    return isDoWhile_;
}
