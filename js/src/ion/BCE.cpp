/* Authors:
 * Igor Rafael [igor@dcc.ufmg.br]
 * Pericles Alves [periclesrafael@dcc.ufmg.br]
 */

#include "Ion.h"
#include "IonSpewer.h"
#include "BCE.h"
#include "MIR.h"
#include "MIRGraph.h"

using namespace js;
using namespace js::ion;

BCE::BCE(MIRGraph &graph)
  : graph(graph)
{
}

bool
BCE::analyze()
{
    IonSpew(IonSpew_BCE, "Beginning BCE pass.");

    // Looks for all the unneed bounds checks in the graph.
    for (MBasicBlockIterator blockIt(graph.begin()); blockIt != graph.end(); blockIt++) {
        MBasicBlock *block = *blockIt;

        InductionVariable indVar(block);

        if (!indVar.extractPattern())
            continue;

        IonSpew(IonSpew_BCE, "Induction variable found! id = %d.", indVar.variable()->id());

        // Looks for unneeded bounds checks among the uses of induction
        // variable.
        for (MUseIterator useIt = indVar.variable()->usesBegin(); useIt != indVar.variable()->usesEnd(); useIt++) {
            MUse *use = *useIt;

            if (!use->node()->isDefinition())
                continue;

            MDefinition *useNode = use->node()->toDefinition();

            // Tries to eliminate the use.
            if (tryElimination(&indVar, useNode))
                continue;

            // The use may be an integer specialization of a bounds check index.
            if (useNode->isToInt32()) {
                for (MUseIterator toInt32UseIt = useNode->usesBegin(); toInt32UseIt != useNode->usesEnd(); toInt32UseIt++) {
                    MUse *toInt32Use = *toInt32UseIt;

                    if (!(toInt32Use->node()->isDefinition()))
                        continue;

                    tryElimination(&indVar, toInt32Use->node()->toDefinition());
                }
            }

            // The use may be an unbox of a bounds check index.
            else if (useNode->isUnbox()) {
                for (MUseIterator unboxUseIt = useNode->usesBegin(); unboxUseIt != useNode->usesEnd(); unboxUseIt++) {
                    MUse *unboxUse = *unboxUseIt;

                    if (!unboxUse->node()->isDefinition())
                        continue;

                    tryElimination(&indVar, unboxUse->node()->toDefinition());
                }
            }
        }
    }

    eliminateUnneededChecks();

    return true;
}

bool
BCE::tryElimination(InductionVariable *indVar, MDefinition *def)
{
    // The induction variable range must be valid in the definition block.
    if (!indVar->rangeIsValid(def->block()))
        return false;

    if (def->isBoundsCheck()) {
        MBoundsCheck *bCheck = def->toBoundsCheck();

        if ((bCheck->minimum() == 0) && (bCheck->maximum() == 0)) {
            if (eliminateBoundsCheck(indVar, bCheck))
                return true;
        } else if (eliminateBoundsCheckRange(indVar, bCheck))
            return true;

    } else if (def->isBoundsCheckLower()) {
        if (eliminateBoundsCheckLower(indVar, def->toBoundsCheckLower()))
            return true;
    }

    return false;
}

bool
BCE::eliminateBoundsCheck(InductionVariable *indVar, MBoundsCheck *bCheck)
{
    JS_ASSERT((bCheck->minimum() == 0) && (bCheck->maximum() == 0));

    if (!bCheck->length()->isInitializedLength())
        return false;

    if (!indVar->hasUpperBound())
        return false;

    // If the length is known to be a constant, we can try to eliminate
    // the bounds check in a easier way.
    if (evaluateConstantLength(indVar->upperBound(), bCheck)) {
        unneededChecks.insert(bCheck);
        return true;
    }

    // If the bounds check target is not a constant, we cannot retrieve the
    // array initialized length.
    if (!bCheck->length()->toInitializedLength()->elements()->toElements()->object()->isConstant())
        return false;

    MConstant *object = bCheck->length()->toInitializedLength()->elements()->toElements()->object()->toConstant();

    if (!object->value().isObject())
        return false;

    if (!object->value().toObject().isDenseArray())
        return false;

    // We need to guarantee that the array initialized length will not
    // change during the scrip execution.
    if (!bCheck->length()->isInitializedLength() || !hasUnchangeableLength(object))
        return false;

    uint32_t initializedLength = object->value().toObject().getDenseArrayInitializedLength();

    // If index < initialized_length, the bounds check can be eliminated.
    if (indVar->upperBound() <= initializedLength) {
        unneededChecks.insert(bCheck);
        return true;
    }

    return false;
}

bool
BCE::eliminateBoundsCheckRange(InductionVariable *indVar, MBoundsCheck *bCheck)
{
    JS_ASSERT((bCheck->minimum() != 0) || (bCheck->maximum() != 0));

    // The induction variable must have a fixed range.
    if (!indVar->hasLowerBound() || !indVar->hasUpperBound())
        return false;

    // If the bounds check target is not a constant, we cannot retrieve the
    // array initialized length.
    if (!bCheck->length()->toInitializedLength()->elements()->toElements()->object()->isConstant())
        return false;

    MConstant *object = bCheck->length()->toInitializedLength()->elements()->toElements()->object()->toConstant();

    if (!object->value().isObject())
        return false;

    if (!object->value().toObject().isDenseArray())
        return false;

    // We need to guarantee that the array initialized length will not
    // change during the scrip execution.
    if (!hasUnchangeableLength(object))
        return false;

    uint32_t initializedLength = object->value().toObject().getDenseArrayInitializedLength();

    // If index + minimum >= 0 and index + maximum < initialized_length, the bounds check can be eliminated.
    if (((indVar->lowerBound() + bCheck->minimum()) >= 0) && ((indVar->upperBound() + bCheck->maximum()) <= initializedLength)) {
        unneededChecks.insert(bCheck);
        return true;
    }

    return false;
}

bool
BCE::eliminateBoundsCheckLower(InductionVariable *indVar, MBoundsCheckLower *bCheckLower)
{
    if (!indVar->hasLowerBound())
        return false;

    // If index >= minimum, the bounds check can be eliminated.
    if (indVar->lowerBound() >= bCheckLower->minimum()) {
        unneededChecks.insert(bCheckLower);
        return true;
    }

    return false;
}

bool 
BCE::evaluateConstantLength(int32 upperBound, MBoundsCheck *bCheck)
{
    if (!bCheck->length()->isConstant())
        return false;

    MConstant *length = bCheck->length()->toConstant();

    if (!length->value().isInt32())
        return false;

    if (upperBound <= length->value().toInt32())
        return true;

    return false;
}

bool
BCE::hasUnchangeableLength(MConstant *array)
{
    // FIXME: 
    // - It does not detect stores of diferent constants that represent the same array.
    // - Since there is no alias analysis, if we have a store in a global object, there
    // is no way to know if this object aliases the array.

    // Simply verifies if there is no store instruction using this array as its operand.
    for (MUseIterator useIt = array->usesBegin(); useIt != array->usesEnd(); useIt++) {
        MUse *use = *useIt;

        if (!use->node()->isDefinition())
            continue;

        MDefinition *def = use->node()->toDefinition();

        if (def->getAliasSet().isStore())
            return false;
    }

    return true;
}

void
BCE::eliminateUnneededChecks()
{
    while (!unneededChecks.empty()) {
        // Gets a new element from the  list.
        MInstruction *bCheck = *(unneededChecks.begin());

        bool canDelete = true;
        for (MUseIterator itUse = bCheck->usesBegin(); itUse != bCheck->usesEnd(); ) { // itUse++) {
            MDefinition *useDef = itUse->node()->toDefinition();
            if (useDef->isLoadElement()) {
                itUse = itUse->node()->replaceOperand(itUse, bCheck->getOperand(0));
            } else {
                canDelete = false;
                itUse++;
            }
        }

        unneededChecks.erase(unneededChecks.begin());

        if (!canDelete) 
            continue;

        bCheck->block()->discard(bCheck);

        IonSpew(IonSpew_BCE, "Bounds check %d eliminated.", bCheck->id());
    }
}
