/**
 * @file ClauseContainer.hpp
 * Defines class ClauseContainer
 *
 */

#ifndef __ClauseContainer__
#define __ClauseContainer__

#include "../Forwards.hpp"

#include "../Lib/Event.hpp"
#include "../Lib/VirtualIterator.hpp"
#include "../Lib/Stack.hpp"

#include "Limits.hpp"

#define OUTPUT_LRS_DETAILS 0

namespace Saturation
{

using namespace Lib;
using namespace Kernel;

class ClauseContainer
{
public:
  virtual ~ClauseContainer() {}
  ClauseEvent addedEvent;
  /**
   * This event fires when a clause is removed from the
   * container because it is no longer needed, e.g. it was
   * backward-simplified, or the container is destroyed.
   * It does not fire for clauses that are removed from the
   * container because they are selected to be further
   * processed by the saturation algorithm (e.g. activated).
   */
  ClauseEvent removedEvent;
  /**
   * This event fires when a clause is removed from the
   * container to be further processed by the saturation
   * algorithm (e.g. activated).
   */
  ClauseEvent selectedEvent;
  virtual void add(Clause* c) = 0;
  void addClauses(ClauseIterator cit);
};

class RandomAccessClauseContainer
: public ClauseContainer
{
public:
  virtual void attach(SaturationAlgorithm* salg);
  virtual void detach();

  virtual unsigned size() = 0;
  virtual void remove(Clause* c) = 0;
  void removeClauses(ClauseIterator cit);

protected:
  RandomAccessClauseContainer() :_salg(0) {}
  SaturationAlgorithm* getSaturationAlgorithm() { return _salg; }

  virtual void onLimitsUpdated(LimitsChangeType change) {}
private:
  SaturationAlgorithm* _salg;
  SubscriptionData _limitChangeSData;
};

class UnprocessedClauseContainer
: public ClauseContainer
{
public:
  ~UnprocessedClauseContainer();
  UnprocessedClauseContainer() : _data(64) {}
  void add(Clause* c);
  Clause* pop();
  bool isEmpty() const
  { return _data.isEmpty(); }
private:
  Stack<Clause*> _data;
};

class PassiveClauseContainer
: public RandomAccessClauseContainer
{
public:

  virtual bool isEmpty() const = 0;
  virtual Clause* popSelected() = 0;

  virtual void updateLimits(long estReachableCnt) {}
};

class ActiveClauseContainer
: public RandomAccessClauseContainer
{
public:
  ActiveClauseContainer() : _size(0) {}

  void add(Clause* c);
  void remove(Clause* c);

  unsigned size() { return _size; }

protected:
  void onLimitsUpdated(LimitsChangeType change);
private:
  unsigned _size;
};


};

#endif /*__ClauseContainer__*/
