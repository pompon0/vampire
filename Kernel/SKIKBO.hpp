
/*
 * File KBO.hpp.
 *
 * This file is part of the source code of the software program
 * Vampire. It is protected by applicable
 * copyright laws.
 *
 * This source code is distributed under the licence found here
 * https://vprover.github.io/license.html
 * and in the source directory
 *
 * In summary, you are allowed to use Vampire for non-commercial
 * purposes but not allowed to distribute, modify, copy, create derivatives,
 * or use in competitions. 
 * For other uses of Vampire please contact developers for a different
 * licence, which we will make an effort to provide. 
 */
/**
 * @file KBO.hpp
 * Defines class KBO for instances of the Knuth-Bendix ordering
 *
 * @since 30/04/2008 flight Brussels-Tel Aviv
 */

#ifndef __SKIKBO__
#define __SKIKBO__

#include "Forwards.hpp"

#include "Lib/DArray.hpp"

#include "Ordering.hpp"
#include "Signature.hpp"

namespace Kernel {

using namespace Lib;

/**
 * Class for instances of the Knuth-Bendix orderings
 * @since 30/04/2008 flight Brussels-Tel Aviv
 */
class SKIKBO
: public PrecedenceOrdering
{
public:
  CLASS_NAME(SKIKBO);
  USE_ALLOCATOR(SKIKBO);

  SKIKBO(Problem& prb, const Options& opt);
  virtual ~SKIKBO();

  using PrecedenceOrdering::compare;
  Result compare(TermList tl1, TermList tl2) const override;
  static unsigned maximumReductionLength(TermList term);

protected:
  Result comparePredicates(Literal* l1, Literal* l2) const override;

  class State;
  /** Weight of variables */
  int _variableWeight;
  /** Weight of function symbols not occurring in the
   * signature */
  int _defaultSymbolWeight;

  int functionSymbolWeight(unsigned fun) const;

  bool allConstantsHeavierThanVariables() const { return false; }
  bool existsZeroWeightUnaryFunction() const { return false; }

  static TermList reduce(TermStack& args, TermList& head);

  /**
   * State used for comparing terms and literals
   */
  mutable State* _state;
};

}
#endif
