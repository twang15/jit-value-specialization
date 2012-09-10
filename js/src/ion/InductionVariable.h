/* Authors:
 * Igor Rafael [igor@dcc.ufmg.br]
 * Pericles Alves [periclesrafael@dcc.ufmg.br]
 */ 

#ifndef jsion_include_indvar_h__
#define jsion_include_indvar_h__

#include "MIR.h"
#include "MIRGraph.h"

namespace js {
namespace ion {

class InductionVariable
{
    // Basic integer range such that "variable >= lowerBound" and "variable <
    // upperBound".
    // The range is valid only outside the induction variable block.
    struct Range {
        int32 lowerBound;
        int32 upperBound;
   };
  private:
    MBasicBlock *block;
    
    MDefinition *variable_; // The actual induction variable.

    /**
      * Indicates if the variable is known to have fixed bounds. These
      * bounds are not valid either in the variable block or outside the
      * loop body.
      */
    bool hasLowerBound_;
    bool hasUpperBound_;
    bool isDoWhile_;

    Range range;

    /**
      * Look for induction variable patterns in the given block. All
      * patterns are integer ones, since it would not be worth it to handle
      * other types.
      * These functions also extract the induction variable range.
      */
    bool extractUBCSPattern(); // Upper Bounded Continuous Sum pattern.
    bool extractUBCSPatternDoWhile();
    
  public:
    InductionVariable(MBasicBlock *block);

    bool extractPattern();

    MDefinition *variable();

    // Verifies if the induction variable range is valid in the given block.
    // Since it relies on block IDs, this pass must run after Renumber
    // Blocks.
    bool rangeIsValid(MBasicBlock *block);

    bool hasLowerBound();
    bool hasUpperBound();
    int32 lowerBound();
    int32 upperBound();
    bool isDoWhile();
};

} // namespace ion
} // namespace js

#endif // jsion_include_indvar_h__

