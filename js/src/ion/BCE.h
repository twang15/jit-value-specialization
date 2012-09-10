/* Authors:
 * Igor Rafael [igor@dcc.ufmg.br]
 * Pericles Alves [periclesrafael@dcc.ufmg.br]
 */

#ifndef jsion_bce_h__
#define jsion_bce_h__

#include "InductionVariable.h"
#include <set>

// This file represents the Array Bounds Check Elimination optimization pass

namespace js {
namespace ion {

class MIRGraph;

/**
  * Array bounds check elimination main class.
  */
class BCE
{
  public:
    BCE(MIRGraph &graph);

    // As GVN may change bounds check range, this pass must run afetr it.
    bool analyze();

  private:
    MIRGraph &graph;
    std::set<MInstruction*> unneededChecks; // List of bounds checks that can be eliminated.

    // Verifies if the given definition is a known kind of bounds check. If so, tries
    // to eliminate it based on the range of the given induction variable.
    bool tryElimination(InductionVariable *indVar,  MDefinition *def);

    /**
      * Given a specific kind of array bounds check and the induction variable
      * used as its index, this set of functions try to eliminate the bounds
      * check by proving that the index is always within the array limits and
      * the array length is never changed inside the function.
      */

    // Attempts to eliminate the basic kind of bounds check: index < length.
    bool eliminateBoundsCheck(InductionVariable *indVar, MBoundsCheck *bCheck);

    // Attempts to eliminate a bounds check in a specifc range: index +
    // minimum >= 0 and index + maximum < length.
    bool eliminateBoundsCheckRange(InductionVariable *indVar, MBoundsCheck *bCheck);

    // Attempts to eliminate an lower bounds check: index >= minimum.
    bool eliminateBoundsCheckLower(InductionVariable *indVar, MBoundsCheckLower *bCheckLower);


    bool evaluateConstantLength(int32 upperBound, MBoundsCheck *bCheck);

    // Verifies if the length of a dense array does not change during the
    // script execution.
    bool hasUnchangeableLength(MConstant *array);

    // Since we may be iterating over lists of instructions, the unneeded
    // bounds checks can be eliminated only at the end of the analysis.
    void eliminateUnneededChecks();
};

} // namespace ion
} // namespace js

#endif // jsion_bce_h__
