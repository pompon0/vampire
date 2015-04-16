/**
 * @file Parse/TPTP.hpp
 * Defines class TPTP for parsing TPTP files
 *
 * @since 08/04/2011 Manchester
 */

#ifndef __Parser_TPTP__
#define __Parser_TPTP__

#include <iostream>

#include "Lib/Array.hpp"
#include "Lib/Set.hpp"
#include "Lib/Stack.hpp"
#include "Lib/Exception.hpp"
#include "Lib/IntNameTable.hpp"

#include "Kernel/Formula.hpp"
#include "Kernel/Unit.hpp"
#include "Kernel/Theory.hpp"

#ifdef VDEBUG
#define DEBUG_SHOW_STATE
#endif

using namespace std;
using namespace Lib;
using namespace Kernel;

namespace Kernel {
  class Clause;
};

namespace Parse {

class TPTP;

/**
 * Implements a TPTP parser
 * @since 08/04/2011 Manchester
 */
class TPTP 
{
public:
  /** Token types */
  enum Tag {
    /** end of file */
    T_EOF,
    /** name, identifier beginning with a lower-case letter */
    T_NAME,
    /** variable name, identifier beginning with an upper-case letter */
    T_VAR,
    /** '(' */
    T_LPAR,
    /** ')' */
    T_RPAR,
    /** '[' */
    T_LBRA,
    /** ']' */
    T_RBRA,
    /** ',' */
    T_COMMA,
    /** ':' */
    T_COLON,
    /** '~' */
    T_NOT,
    /** '&' */
    T_AND,
    /** '=' */
    T_EQUAL,
    /** string */
    T_STRING,
    /** inequality */
    T_NEQ,
    /** universal quantifier */
    T_FORALL,
    /** existential quantifier */
    T_EXISTS,
    /** type universal quantifier */
    T_PI,
    /** type existential quantifier */
    T_SIGMA,
    /** implication */
    T_IMPLY,
    /** exclusive or */
    T_XOR,
    /** equivalence */
    T_IFF,
    /** reverse implication */
    T_REVERSE_IMP,
    /** dot */
    T_DOT,
    /** real number */
    T_REAL,
    /** rational number */
    T_RAT,
    /** integer */
    T_INT,
    /** connective or */
    T_OR,
    /** := */
    T_ASS,
    /** lambda operator */
    T_LAMBDA,
    /** (higher-order) application */
    T_APP,
    /** star (product type) */
    T_STAR,
    /** union type? */
    T_UNION,
    /** arrow type */
    T_ARROW,
    /** Subtype */
    T_SUBTYPE,
    /** Geoff's invention, one connective for the price of two */
    T_NOT_OR,
    /** Geoff's invention, one connective for the price of two */
    T_NOT_AND,
    /** sequent */
    T_SEQUENT,
    /** !> */
    T_THF_QUANT_ALL,
    /** ?* */
    T_THF_QUANT_SOME,
    /** some THF quantifier */
    T_APP_PLUS,
    /** some THF quantifier */
    T_APP_MINUS,
    /** $true */
    T_TRUE,
    /** $false */
    T_FALSE,
    /** $tType */
    T_TTYPE,
    /** $o */
    T_BOOL_TYPE,
    /** $i */
    T_DEFAULT_TYPE,
    /** $int */
    T_INTEGER_TYPE,
    /** $rat */
    T_RATIONAL_TYPE,
    /** $real */
    T_REAL_TYPE,
    /** $fot, probably useless */
    T_ARRAY_TYPE,
    /** $array type*/
    T_SELECT,
    /** array select*/
    T_STORE,
    /** array store*/
    T_FOT,
    /** $fof, probably useless */
    T_FOF,
    /** $tff, probably useless */
    T_TFF,
    /** $thf, probably useless */
    T_THF,
    /** anything that begins with $$ */
    T_DOLLARS,
    /** $ite: FOOL level-polymorphic if-then-else */
    T_ITE,
    /** $let: FOOL level-polymorphic let-in */
    T_LET,
  };

  /** parser state, numbers are just temporarily for debugging */
  enum State {
    /** list of units */
    UNIT_LIST,
    /** cnf() declaration */
    CNF,
    /** fof() declaration */
    FOF,
    /** vampire() declaration */
    VAMPIRE,
    /** read formula */
    FORMULA,
    /** process end of fof() declaration */
    END_FOF,
    /** read a simple formula */
    SIMPLE_FORMULA,
    /** build formula from a connective and one or more formulas */
    END_FORMULA,
    /** read a formula that whould be put inside a term */
    FORMULA_INSIDE_TERM,
    /** */
    TERM_INFIX,
    /** wrap built formula inside a term */
    END_FORMULA_INSIDE_TERM,
    /** make built boolean term a formula */
    END_TERM_AS_FORMULA,
    /** read a variable list (for a quantifier) */
    VAR_LIST,
    /** read a function application */
    FUN_APP,
    /** process application of = or != to an atom */
    FORMULA_INFIX,
    /** read arguments */
    ARGS,
    /** read term */
    TERM,
    /** read arguments after a term */
    END_TERM,
    /** read a single token and do nothing */
    TAG,
    /** include directive */
    INCLUDE,
    /** after the equality */
    END_EQ,
    /** tff declaration */
    TFF,
    /** read type declaration */
    TYPE,
    /** after a top-level type declaration */
    END_TFF,
    /** after a type declaration */
    END_TYPE,
    /** simple type */
    SIMPLE_TYPE,
    /** unbinding previously quantified variables */
    UNBIND_VARIABLES,
    /** end of an if-then-else expression */
    END_ITE,
    /** check the end of arguments */
    END_ARGS,
    /** middle of equality */
    MID_EQ,
    /** end of $let expression */
    END_LET,
    /** start of function or predicate binding inside $let */
    BINDING,
    /** end of function or predicate binding inside $let */
    END_BINDING,
    /** end of select array terms */
    END_SELECT,
    /** end of store array terms */
    END_STORE
  };

  /** token */
  struct Token {
    /** token type */
    Tag tag;
    /** position of the beginning of this token */
    int start;
    /** content */
    vstring content;
    vstring toString() const;
  };

  /**
   * Implements lexer and parser exceptions.
   */
  class ParseErrorException 
    : public ::Exception
  {
  public:
    ParseErrorException(vstring message) : _message(message) {}
    ParseErrorException(vstring message,Token& tok);
    ParseErrorException(vstring message,int position);
    void cry(ostream&);
    ~ParseErrorException() {}
  protected:
    vstring _message;
  }; // TPTP::ParseErrorException
  friend class Exception;

#define PARSE_ERROR(msg,tok) \
  throw ParseErrorException(msg,tok)

  TPTP(istream& in);
  ~TPTP();
  void parse();
  static UnitList* parse(istream& str);
  /** Return the list of parsed units */
  inline UnitList* units() { return _units.list(); }
  /**
   * Return true if there was a conjecture formula among the parsed units
   *
   * The purpose of this information is that when we report success in the
   * SZS ontology, we decide whether to output "Theorem" or "Unsatisfiable"
   * based on this value.
   */
  bool containsConjecture() const { return _containsConjecture; }
  void addForbiddenInclude(vstring file);
  static bool findAxiomName(const Unit* unit, vstring& result);
  //this function is used also by the API
  static void assignAxiomName(const Unit* unit, vstring& name);
private:
  /** Return the input string of characters */
  const char* input() { return _chars.content(); }

  enum TypeTag {
    TT_ATOMIC,
    TT_PRODUCT,
    TT_ARROW
  };

  /**
   * Class of types. Should be removed when the Vampire type system is
   * improved.
   * @since 14/07/2011 Manchester
   */
  class Type {
  public:
    CLASS_NAME(Type);
    USE_ALLOCATOR(Type);
    explicit Type(TypeTag tag) : _tag(tag) {}
    /** return the kind of this sort */
    TypeTag tag() const {return _tag;}
  protected:
    /** kind of this type */
    TypeTag _tag;
  };

  /** An atomic type is simply a sort */
  class AtomicType
    : public Type
  {
  public:
    CLASS_NAME(AtomicType);
    USE_ALLOCATOR(AtomicType);
    explicit AtomicType(unsigned sortNumber)
      : Type(TT_ATOMIC), _sortNumber(sortNumber)
    {}
    /** return the sort number */
    unsigned sortNumber() const {return _sortNumber;}
  private:
    /** the sort identified by its number in the signature */
    unsigned _sortNumber;
  }; // AtomicType

  /** Arrow type */
  class ArrowType
    : public Type
  {
  public:
    CLASS_NAME(ArrowType);
    USE_ALLOCATOR(ArrowType);
    ArrowType(Type* lhs,Type* rhs)
      : Type(TT_ARROW), _lhs(lhs), _rhs(rhs)
    {}
    /** the argument type */
    Type* argumentType() const {return _lhs;}
    /** the return type */
    Type* returnType() const {return _rhs;}
  private:
    /** the argument type */
    Type* _lhs;
    /** the return type */
    Type* _rhs;
  }; // ArrowType

  /** Product type. It now only uses a product of two types, which might be
   * wrong for higher-order logic, yet appropriate for parsing first-order
   * types.
   */
  class ProductType
    : public Type
  {
  public:
    CLASS_NAME(ProductType);
    USE_ALLOCATOR(ProductType);
    ProductType(Type* lhs,Type* rhs)
      : Type(TT_PRODUCT), _lhs(lhs), _rhs(rhs)
    {}
    /** the left hand side type */
    Type* lhs() const {return _lhs;}
    /** the right hand side type */
    Type* rhs() const {return _rhs;}
  private:
    /** the argument type */
    Type* _lhs;
    /** the return type */
    Type* _rhs;
  }; // ProductType

  /**
   * Class that allows to create a list initially by pushing elements
   * at the end of it.
   * @since 10/05/2007 Manchester, updated from List::FIFO
   */
  class UnitStack {
  public:
    /** constructor */
    inline explicit UnitStack()
      : _initial(0),
	_last(&_initial)
    {}

    /** add element at the end of the original list */
    inline void push(Unit* u)
    {
      UnitList* newList = new UnitList(u);
      *_last = newList;
      _last = reinterpret_cast<UnitList**>(&newList->tailReference());
    }

    /** Return the collected list */
    UnitList* list() { return _initial; }

  private:
    /** reference to the initial element */
    UnitList* _initial;
    /** last element */
    UnitList** _last;
  }; // class UnitStack

  /** true if the input contains a conjecture */
  bool _containsConjecture;
  /** Allowed names of formulas.
   * If non-null, ignore formulas not included in _allowedNames.
   * This is to support the feature formula_selection of the include
   * directive of the TPTP format.
 */
  Set<vstring>* _allowedNames;
  /** stacks of allowed names when include is used */
  Stack<Set<vstring>*> _allowedNamesStack;
  /** set of files whose inclusion should be ignored */
  Set<vstring> _forbiddenIncludes;
  /** the input stream */
  istream* _in;
  /** in the case include() is used, previous streams will be saved here */
  Stack<istream*> _inputs;
  /** the current include directory */
  vstring _includeDirectory;
  /** in the case include() is used, previous sequence of directories will be
   * saved here, this is required since TPTP requires the directory to be
   * relative to the "current directory, that is, the directory used by the last include()
   */
  Stack<vstring> _includeDirectories;
  /** input characters */
  Array<char> _chars;
  /** position in the input stream of the 0th character in _chars[] */
  int _gpos;
  /** the position beyond the last read characters */
  int _cend;
  /** tokens currently at work */
  Array<Token> _tokens;
  /** the position beyond the last processed token */
  int _tend;
  /** The stack of units read */
  UnitStack _units;
  /** stack of unprocessed states */
  Stack<State> _states;
  /** input type of the last read unit */ // it must be int since -1 can be used as a value
  int _lastInputType;
  /** true if the last read unit is a question */ 
  bool _isQuestion;
  /** true if the last read unit is fof() or cnf() due to a subtle difference
   * between fof() and tff() in treating numeric constants */ 
  bool _isFof;
  /** various strings saved during parsing */
  Stack<vstring> _strings;
  /** various connectives saved during parsing */ // they must be int, since non-existing value -1 can be used
  Stack<int> _connectives;
  /** various boolean values saved during parsing */
  Stack<bool> _bools;
  /** various integer values saved during parsing */
  Stack<int> _ints;
  /** variable lists for building formulas */
  Stack<Formula::VarList*> _varLists;
  /** variable lists for binding variables */
  Stack<Formula::VarList*> _bindLists;
  /** various tokens to consume */
  Stack<Tag> _tags;
  /** various formulas */
  Stack<Formula*> _formulas;
  /** various literals */
  Stack<Literal*> _literals;
  /** term lists */
  Stack<TermList> _termLists;
  /** name table for variable names */
  IntNameTable _vars;
  /** parsed types */
  Stack<Type*> _types;
  /** various type tags saved during parsing */
  Stack<TypeTag> _typeTags;
  typedef List<unsigned> SortList;
  /** bindings of variables to sorts */
  Map<int,SortList*> _variableSorts;
  /** overflown arithmetical constants for which uninterpreted constants are introduced */
  Set<vstring> _overflow;
  /** current color, if the input contains colors */
  Color _currentColor;

  /**
   * Get the next characters at the position pos.
   */
  inline char getChar(int pos)
  {
    CALL("TPTP::getChar");

    while (_cend <= pos) {
      int c = _in->get();
      //      if (c == -1) { cout << "<EOF>"; } else {cout << char(c);}
      _chars[_cend++] = c == -1 ? 0 : c;
    }
    return _chars[pos];
  } // getChar

  /**
   * Shift characters in the buffer by n positions left.
   */
  inline void shiftChars(int n)
  {
    CALL("TPTP::shiftChars");
    ASS(n > 0);
    ASS(n <= _cend);

    for (int i = 0;i < _cend-n;i++) {
      _chars[i] = _chars[n+i];
    }
    _cend -= n;
    _gpos += n;
  } // shiftChars

  /**
   * Reset the character buffer.
   * @since 10/04/2011 Manchester
   */
  inline void resetChars()
  {
    _gpos += _cend;
    _cend = 0;
  } // resetChars

  /**
   * Get the token at the position pos.
   */
  inline Token& getTok(int pos)
  {
    CALL("TPTP::getTok");

    while (_tend <= pos) {
      Token& tok = _tokens[_tend++];
      readToken(tok);
    }
    return _tokens[pos];
  } // getTok

  /**
   * Shift tokens in the buffer by n positions left.
   */
  inline void shiftToks(int n)
  {
    CALL("TPTP::shiftToks");

    ASS(n > 0);
    ASS(n <= _tend);

    for (int i = 0;i < _tend-n;i++) {
      _tokens[i] = _tokens[n+i];
    }
    _tend -= n;
  } // shiftToks

  /**
   * Reset the character buffer.
   * @since 10/04/2011 Manchester
   */
  inline void resetToks()
  {
    _tend = 0;
  } // resetToks

  // lexer functions
  bool readToken(Token& t);
  void skipWhiteSpacesAndComments();
  void readName(Token&);
  void readReserved(Token&);
  void readString(Token&);
  void readAtom(Token&);
  Tag readNumber(Token&);
  int decimal(int pos);
  int positiveDecimal(int pos);
  static vstring toString(Tag);

  // parser functions
  static Formula* makeJunction(Connective c,Formula* lhs,Formula* rhs);
  void unitList();
  void fof(bool fo);
  void tff();
  void vampire();
  void consumeToken(Tag);
  vstring name();
  void formula();
  void funApp();
  void simpleFormula();
  void simpleType();
  void args();
  void varList();
  void term();
  void termInfix();
  void endTerm();
  void endArgs();
  Literal* createEquality(bool polarity,TermList& lhs,TermList& rhs);
  Formula* createPredicateApplication(vstring name,unsigned arity);
  TermList createFunctionApplication(vstring name,unsigned arity);
  Formula* createFormula(TermList& term);
  void endEquality();
  void midEquality();
  void formulaInfix();
  void endFormula();
  void formulaInsideTerm();
  void endFormulaInsideTerm();
  void endTermAsFormula();
  void endType();
  void tag();
  void endFof();
  void endTff();
  void include();
  void type();
  void endIte();
  void binding();
  void endBinding();
  void endLet();
  void endSelect();
  void endStore();
  void addTagState(Tag);

  unsigned readSort();
  void bindVariable(int var,unsigned sortNumber);
  void unbindVariables();
  void skipToRPAR();
  unsigned addFunction(vstring name,int arity,bool& added,TermList& someArgument);
  int addPredicate(vstring name,int arity,bool& added,TermList& someArgument);
  unsigned addOverloadedFunction(vstring name,int arity,int symbolArity,bool& added,TermList& arg,
				 Theory::Interpretation integer,Theory::Interpretation rational,
				 Theory::Interpretation real);
  unsigned addOverloadedPredicate(vstring name,int arity,int symbolArity,bool& added,TermList& arg,
				  Theory::Interpretation integer,Theory::Interpretation rational,
				  Theory::Interpretation real);
 // unsigned addOverloadedArrayFunction(vstring name,int arity,int symbolArity,bool& added,TermList& arg,Theory::Interpretation array_select);
  unsigned addIntegerConstant(const vstring&);
  unsigned addRationalConstant(const vstring&);
  unsigned addRealConstant(const vstring&);
  unsigned addUninterpretedConstant(const vstring& name,bool& added);
  unsigned sortOf(TermList& term);
  static bool higherPrecedence(int c1,int c2);

private:
  /** This field stores names of input units if the
   * output_axiom_names option is enabled */
  static DHMap<unsigned, vstring> _axiomNames;

#if VDEBUG
  void printStates(vstring extra);
  void printInts(vstring extra);
  const char* toString(State s);
#endif
#ifdef DEBUG_SHOW_STATE
  void printStacks();
#endif
}; // class TPTP

}

#endif

