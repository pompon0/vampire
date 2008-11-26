/**
 * @file SubstitutionTree.cpp
 * Implements class SubstitutionTree.
 *
 * @since 16/08/2008 flight Sydney-San Francisco
 */

#include "../Kernel/Clause.hpp"
#include "../Kernel/Term.hpp"
#include "../Kernel/Renaming.hpp"
#include "../Lib/BinaryHeap.hpp"

#ifdef VDEBUG
#include <iostream>
#include "../Kernel/Signature.hpp"
#include "../Lib/Environment.hpp"
#include "../Lib/Int.hpp"
#include "../Test/Output.hpp"

string SingleTermListToString(const TermList* ts);

#endif

#include "SubstitutionTree.hpp"

using namespace Indexing;




/**
 * Initialise the substitution tree.
 * @since 16/08/2008 flight Sydney-San Francisco
 */
SubstitutionTree::SubstitutionTree(int nodes)
  : _numberOfTopLevelNodes(nodes),
    _nextVar(0)
{
  CALL("SubstitutionTree::SubstitutionTree");
  ASS(nodes > 0);

  _nodes = new(ALLOC_KNOWN(nodes*sizeof(Node*),"SubstitutionTree::Node"))
                Node*[nodes];
  for (int i = nodes-1;i >= 0;i--) {
    _nodes[i] = 0;
  }
} // SubstitutionTree::SubstitutionTree

/**
 * Destroy the substitution tree.
 * @warning does not destroy nodes yet
 * @since 16/08/2008 flight Sydney-San Francisco
 */
SubstitutionTree::~SubstitutionTree()
{
  CALL("SubstitutionTree::~SubstitutionTree");

  for (int i = _numberOfTopLevelNodes-1; i>=0; i--) {
    if(_nodes[i]!=0) {
      delete _nodes[i];
    }
  }

  DEALLOC_KNOWN(_nodes,
		_numberOfTopLevelNodes*sizeof(Node*),
		"SubstitutionTree::Node");
} // SubstitutionTree::~SubstitutionTree


void SubstitutionTree::getBindings(Term* t, BindingQueue& bq)
{
  Binding bind;
  TermList* args=t->args();

  int nextVar = 0;
  while (! args->isEmpty()) {
    if (_nextVar <= nextVar) {
      _nextVar = nextVar+1;
    }
    bind.var = nextVar++;
    bind.term = args;
    bq.insert(bind);
    args = args->next();
  }
}

void SubstitutionTree::insert(Literal* lit, Clause* cls)
{
  Renaming normalizer;
  Renaming::normalizeVariables(lit,normalizer);
  Literal* normLit=normalizer.apply(lit);

  BindingQueue bq;
  getBindings(normLit, bq);
  insert(_nodes+getRootNodeIndex(lit), bq, LeafData(cls, lit));
}
void SubstitutionTree::remove(Literal* lit, Clause* cls)
{
  Renaming normalizer;
  Renaming::normalizeVariables(lit,normalizer);
  Literal* normLit=normalizer.apply(lit);

  BindingQueue bq;
  getBindings(normLit, bq);
  remove(_nodes+getRootNodeIndex(lit), bq, LeafData(cls, lit));
}

void SubstitutionTree::insert(TermList* term, Clause* cls)
{
  ASS(!term->isEmpty());
  ASS(!term->isSpecialVar());

  BindingQueue bq;
  getBindings(term, bq);

  insert(_nodes+getRootNodeIndex(term), bq, LeafData(cls, reinterpret_cast<void*&>(*term)));
}

void SubstitutionTree::remove(TermList* term, Clause* cls)
{
  ASS(!term->isEmpty());
  ASS(!term->isSpecialVar());

  BindingQueue bq;
  getBindings(term, bq);

  remove(_nodes+getRootNodeIndex(term), bq, LeafData(cls, reinterpret_cast<void*&>(*term)));
}

/**
 * Auxiliary function for substitution tree insertion.
 * @since 16/08/2008 flight Sydney-San Francisco
 */
void SubstitutionTree::insert(Node** pnode,BindingQueue& bh,LeafData ld)
{
  CALL("SubstitutionTree::insert/3");

  if(*pnode == 0) {
    if(bh.isEmpty()) {
      *pnode=createLeaf();
    } else {
      *pnode=createIntermediateNode();
    }
  }
  if(bh.isEmpty()) {
    ASS((*pnode)->isLeaf());
    ensureLeafEfficiency(reinterpret_cast<Leaf**>(pnode));
    static_cast<Leaf*>(*pnode)->insert(ld);
    return;
  }

  start:
  Binding bind=bh.pop();
  TermList* term=bind.term;

  ASS(! (*pnode)->isLeaf());
  IntermediateNode* inode = static_cast<IntermediateNode*>(*pnode);

  //Into pparent we store the node, we might be inserting into.
  //So in the case we do insert, we might check whether this node
  //needs expansion.
  Node** pparent=pnode;
  pnode=inode->childByTop(term,true);

  if (*pnode == 0) {
    while (!bh.isEmpty()) {
      IntermediateNode* inode = createIntermediateNode(term);
      *pnode = inode;

      bind = bh.pop();
      term=bind.term;
      pnode = inode->childByTop(term,true);
    }
    Leaf* lnode=createLeaf(term);
    *pnode=lnode;
    lnode->insert(ld);

    ensureIntermediateNodeEfficiency(reinterpret_cast<IntermediateNode**>(pparent));
    return;
  }


  TermList* tt = term;
  TermList* ss = &(*pnode)->term;

  ASS(TermList::sameTop(ss, tt));

  if (tt->sameContent(ss)) {
    if (bh.isEmpty()) {
      ASS((*pnode)->isLeaf());
      ensureLeafEfficiency(reinterpret_cast<Leaf**>(pnode));
      Leaf* leaf = static_cast<Leaf*>(*pnode);
      leaf->insert(ld);
      return;
    }
    goto start;
  }

  // ss is the term in node, tt is the term to be inserted
  // ss and tt have the same top symbols but are not equal
  // create the common subterm of ss,tt and an alternative node
  Stack<TermList*> subterms(64);
  for (;;) {
    if (!ss->sameContent(tt) && TermList::sameTop(ss,tt)) {
      // ss and tt have the same tops and are different, so must be non-variables
      ASS(! ss->isVar());
      ASS(! tt->isVar());

      Term* s = ss->term();
      Term* t = tt->term();

      ASS(s->arity() > 0);
      ASS(s->functor() == t->functor());

      if (s->shared()) {
	// create a shallow copy of s
	s = Term::cloneNonShared(s);
	ss->setTerm(s);
      }

      ss = s->args();
      tt = t->args();
      if (ss->next()->isEmpty()) {
	continue;
      }
      subterms.push(ss->next());
      subterms.push(tt->next());
    } else {
      if (! TermList::sameTop(ss,tt)) {
	int x;
	if(!ss->isSpecialVar()) {
	  x = _nextVar++;
	  Node::split(pnode, ss,x);
	} else {
	  x=ss->var();
	}
	Binding bind(x,tt);
	bh.insert(bind);
      }

      if (subterms.isEmpty()) {
	break;
      }
      tt = subterms.pop();
      ss = subterms.pop();
      if (! ss->next()->isEmpty()) {
	subterms.push(ss->next());
	subterms.push(tt->next());
      }
    }
  }

  goto start;
} // // SubstitutionTree::insert

/*
 * Remove a clause from the substitution tree.
 * @since 16/08/2008 flight San Francisco-Seattle
 */
void SubstitutionTree::remove(Node** pnode,BindingQueue& bh,LeafData ld)
{
  CALL("SubstitutionTree::remove-2");

  ASS(*pnode);

  Stack<Node**> history(1000);

  while (! bh.isEmpty()) {
    history.push(pnode);

    ASS (! (*pnode)->isLeaf());
    IntermediateNode* inode=static_cast<IntermediateNode*>(*pnode);

    Binding bind = bh.pop();
    TermList* t = bind.term;

    pnode=inode->childByTop(t,false);
    ASS(pnode);

    TermList* s = &(*pnode)->term;
    ASS(TermList::sameTop(s,t));

    if (s->content() == t->content()) {
      continue;
    }

    ASS(! s->isVar());
    const TermList* ss = s->term()->args();
    ASS(!ss->isEmpty());

    // computing the disagreement set of the two terms
    TermStack subterms(120);

    subterms.push(ss);
    subterms.push(t->term()->args());
    while (! subterms.isEmpty()) {
      const TermList* tt = subterms.pop();
      ss = subterms.pop();
      if (tt->next()->isEmpty()) {
	ASS(ss->next()->isEmpty());
      }
      else {
	subterms.push(ss->next());
	subterms.push(tt->next());
      }
      if (ss->content() == tt->content()) {
	continue;
      }
      if (ss->isVar()) {
	ASS(ss->isSpecialVar());
	Binding b(ss->var(),const_cast<TermList*>(tt));
	bh.insert(b);
	continue;
      }
      ASS(! t->isVar());
      ASS(ss->term()->functor() == tt->term()->functor());
      ss = ss->term()->args();
      if (! ss->isEmpty()) {
	ASS(! tt->term()->args()->isEmpty());
	subterms.push(ss);
	subterms.push(tt->term()->args());
      }
    }
  }

  ASS ((*pnode)->isLeaf());


  Leaf* lnode = static_cast<Leaf*>(*pnode);
  lnode->remove(ld);
  ensureLeafEfficiency(reinterpret_cast<Leaf**>(pnode));

  while( (*pnode)->isEmpty() ) {
    TermList term=(*pnode)->term;
    if(history.isEmpty()) {
      delete *pnode;
      *pnode=0;
      return;
    } else {
      Node* node=*pnode;
      IntermediateNode* parent=static_cast<IntermediateNode*>(*history.top());
      parent->remove(&term);
      delete node;
      pnode=history.pop();
      ensureIntermediateNodeEfficiency(reinterpret_cast<IntermediateNode**>(pnode));
    }
  }
} // SubstitutionTree::remove


#if VDEBUG

string getIndentStr(int n)
{
  string res;
  for(int indCnt=0;indCnt<n;indCnt++) {
	res+="  ";
  }
  return res;
}

string SubstitutionTree::toString() const
{
  CALL("SubstitutionTree::toString");

  string res;

  for(int tli=0;tli<_numberOfTopLevelNodes;tli++) {
    res+=Int::toString(tli);
    res+=":\n";

    Stack<int> indentStack(10);
    Stack<Node*> stack(10);

    stack.push(_nodes[tli]);
    indentStack.push(1);

    while(stack.isNonEmpty()) {
      Node* node=stack.pop();
      int indent=indentStack.pop();

      if(!node) {
	continue;
      }
      if(!node->term.isEmpty()) {
	res+=getIndentStr(indent)+Test::Output::singleTermListToString(node->term)+"\n";
      }

      if(node->isLeaf()) {
	Leaf* lnode = static_cast<Leaf*>(node);
	LDIterator ldi(lnode->allChildren());

	while(ldi.hasNext()) {
	  res+=getIndentStr(indent) + "Lit: " + Test::Output::toString((Literal*)ldi.next().data) + "\n";
	}
      } else {
	IntermediateNode* inode = static_cast<IntermediateNode*>(node);
	NodeIterator noi(inode->allChildren());
	while(noi.hasNext()) {
	  stack.push(*noi.next());
	  indentStack.push(indent+1);
	}
      }
    }
  }
  return res;
}

#endif

SubstitutionTree::Node::~Node()
{
  CALL("SubstitutionTree::Node::~Node");

  if(term.isTerm()) {
    term.term()->destroyNonShared();
  }
}


void SubstitutionTree::Node::split(Node** pnode, TermList* where, int var)
{
  CALL("SubstitutionTree::Node::split");

  Node* node=*pnode;

  IntermediateNode* newNode = createIntermediateNode(&node->term);
  node->term=*where;
  *pnode=newNode;

  where->makeSpecialVar(var);

  Node** nodePosition=newNode->childByTop(&node->term, true);
  ASS(!*nodePosition);
  *nodePosition=node;
}

void SubstitutionTree::IntermediateNode::loadChildren(NodeIterator children)
{
  CALL("SubstitutionTree::IntermediateNode::loadChildren");

  while(children.hasNext()) {
    Node* ext=*children.next();
    Node** own=childByTop(&ext->term, true);
    ASS(! *own);
    *own=ext;
  }
}

void SubstitutionTree::Leaf::loadChildren(LDIterator children)
{
  CALL("SubstitutionTree::Leaf::loadClauses");

  while(children.hasNext()) {
    LeafData ld=children.next();
    insert(ld);
  }
}

void SubstitutionTree::UnificationsIterator::init(SubstitutionTree* t,
	Literal* query, bool complementary)
{
  CALL("SubstitutionTree::UnificationsIterator::init");
  int rootIndex=t->getRootNodeIndex(query, complementary);
  Node* root=t->_nodes[rootIndex];

  if(!root) {
	return;
  }

  Renaming::normalizeVariables(query, queryNormalizer);
  Literal* queryNorm=queryNormalizer.apply(query);

  createInitialBindings(queryNorm);

  BacktrackData bd;
  enter(root, bd);
  bd.drop();
}

void SubstitutionTree::UnificationsIterator::createInitialBindings(Term* t)
{
  TermList* args=t->args();
  int nextVar = 0;
  while (! args->isEmpty()) {
    unsigned var = nextVar++;
	subst.bindSpecialVar(var,*args,NORM_QUERY_BANK);
    svQueue.insert(var);
    args = args->next();
  }
}

bool SubstitutionTree::UnificationsIterator::hasNext()
{
  CALL("SubstitutionTree::UnificationsIterator::hasNext");

  while(!ldIterator.hasNext() && findNextLeaf()) {}
  return ldIterator.hasNext();
}

SLQueryResult SubstitutionTree::UnificationsIterator::next()
{
  CALL("SubstitutionTree::UnificationsIterator::next");

  while(!ldIterator.hasNext() && findNextLeaf()) {}
  ASS(ldIterator.hasNext());

  if(clientBDRecording) {
    subst.bdDone();
    clientBDRecording=false;
    clientBacktrackData.backtrack();
  }

  LeafData ld=ldIterator.next();

  Literal* lit=static_cast<Literal*>(ld.data);
  Renaming normalizer;
  Renaming::normalizeVariables(lit,normalizer);

  ASS(clientBacktrackData.isEmpty());
  subst.bdRecord(clientBacktrackData);
  clientBDRecording=true;

  subst.denormalize(normalizer,NORM_RESULT_BANK,RESULT_BANK);
  subst.denormalize(queryNormalizer,NORM_QUERY_BANK,QUERY_BANK);

  return SLQueryResult(lit, ld.clause, &subst);
}


bool SubstitutionTree::UnificationsIterator::findNextLeaf()
{
  CALL("SubstitutionTree::UnificationsIterator::findNextLeaf");

  if(nodeIterators.isEmpty()) {
    //There are no node iterators in the stack, so there's nowhere
    //to look for the next leaf.
    //This shouldn't hapen during the regular retrieval process, but it
    //can happen when there are no literals inserted for a predicate,
    //or when predicates with zero arity are encountered.
    return false;
  }

  if(inLeaf) {
    if(clientBDRecording) {
      subst.bdDone();
      clientBDRecording=false;
      clientBacktrackData.backtrack();
    }
    //Leave the current leaf
    bdStack.pop().backtrack();
    inLeaf=false;
  }

  ASS(!clientBDRecording);
  ASS(bdStack.length()+1==nodeIterators.length());

  do {
    while(!nodeIterators.top().hasNext() && !bdStack.isEmpty()) {
      //backtrack undos everything that enter(...) method has done,
      //so it also pops one item out of the nodeIterators stack
      bdStack.pop().backtrack();
    }
    if(!nodeIterators.top().hasNext()) {
      return false;
    }
    Node* n=*nodeIterators.top().next();
    BacktrackData bd;
    bool success=enter(n,bd);
    if(!success) {
      bd.backtrack();
      continue;
    } else {
      bdStack.push(bd);
    }
  } while(!inLeaf);
  return true;
}

bool SubstitutionTree::UnificationsIterator::enter(Node* n, BacktrackData& bd)
{
  CALL("SubstitutionTree::UnificationsIterator::enter");

  if(!n->term.isEmpty()) {
    //n is proper node, not a root

    unsigned specVar=svQueue.top();
    TermList qt;
    qt.makeSpecialVar(specVar);

    subst.bdRecord(bd);
    bool success=associate(qt,n->term);
    subst.bdDone();

    if(!success) {
      return false;
    }
    svQueue.backtrackablePop(bd);
    extractSpecialVariables(n->term, bd);
  }
  if(n->isLeaf()) {
    ldIterator=static_cast<Leaf*>(n)->allChildren();
    inLeaf=true;
  } else {
    ASS(!svQueue.isEmpty());
    NodeIterator nit=getNodeIterator(static_cast<IntermediateNode*>(n));
    nodeIterators.backtrackablePush(nit, bd);
  }
  return true;
}

bool SubstitutionTree::UnificationsIterator::associate(TermList query, TermList node)
{
  return subst.unify(query,NORM_QUERY_BANK,node,NORM_RESULT_BANK);
}

SubstitutionTree::NodeIterator
  SubstitutionTree::UnificationsIterator::getNodeIterator(IntermediateNode* n)
{
  CALL("SubstitutionTree::UnificationsIterator::getNodeIterator");

  unsigned specVar=svQueue.top();
  TermList qt=subst.getSpecialVarTop(specVar);
  if(qt.isVar()) {
	return n->allChildren();
  } else {
	Node** match=n->childByTop(&qt, false);
	if(match) {
	  return NodeIterator(
		  new CatIterator<Node**>(
			  NodeIterator(new SingletonIterator<Node**>(match)),
			  n->variableChildren()
			  ));
	} else {
	  return n->variableChildren();
	}
  }
}

void SubstitutionTree::UnificationsIterator::extractSpecialVariables(
	TermList t, BacktrackData& bd)
{
  TermList* ts=&t;
  static Stack<TermList*> stack(4);
  for(;;) {
    if(ts->tag()==REF && !ts->term()->shared()) {
      stack.push(ts->term()->args());
    }
    if(ts->isSpecialVar()) {
      svQueue.backtrackableInsert(ts->var(), bd);
    }
    if(stack.isEmpty()) {
      break;
    }
    ts=stack.pop();
    if(!ts->next()->isEmpty()) {
      stack.push(ts->next());
    }
  }
}

bool SubstitutionTree::GeneralizationsIterator::associate(TermList query, TermList node)
{
  return subst.match(node, NORM_RESULT_BANK, query, NORM_QUERY_BANK);
}

SubstitutionTree::NodeIterator
  SubstitutionTree::GeneralizationsIterator::getNodeIterator(IntermediateNode* n)
{
  CALL("SubstitutionTree::GeneralizationsIterator::getNodeIterator");

  unsigned specVar=svQueue.top();
  TermList qt=subst.getSpecialVarTop(specVar);
  if(qt.isVar()) {
	return n->variableChildren();
  } else {
	Node** match=n->childByTop(&qt, false);
	if(match) {
	  return NodeIterator(
		  new CatIterator<Node**>(
			  NodeIterator(new SingletonIterator<Node**>(match)),
			  n->variableChildren()
			  ));
	} else {
	  return n->variableChildren();
	}
  }
}
