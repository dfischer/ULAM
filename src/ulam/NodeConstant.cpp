#include <stdlib.h>
#include "NodeConstant.h"
#include "CompilerState.h"

namespace MFM {

  NodeConstant::NodeConstant(const Token& tok, SymbolWithValue * symptr, CompilerState & state) : NodeTerminal(state), m_token(tok), m_constSymbol(symptr), m_ready(false), m_constType(Nouti), m_currBlockNo(0)
  {
    assert(symptr);
    setBlockNo(symptr->getBlockNoOfST());
    m_ready = updateConstant(); //sets ready here
    m_constType = m_constSymbol->getUlamTypeIdx();
  }

  NodeConstant::NodeConstant(const NodeConstant& ref) : NodeTerminal(ref), m_token(ref.m_token), m_constSymbol(NULL), m_ready(false), m_constType(ref.m_constType), m_currBlockNo(ref.m_currBlockNo) {}

  NodeConstant::~NodeConstant(){}

  Node * NodeConstant::instantiate()
  {
    return new NodeConstant(*this);
  }

  void NodeConstant::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }

  const char * NodeConstant::getName()
  {
    if(isReadyConstant())
      return NodeTerminal::getName();
    return m_state.getTokenDataAsString(m_token).c_str();
  }

  const std::string NodeConstant::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeConstant::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return (m_constSymbol != NULL); //true;
  }

  bool NodeConstant::hasASymbolDataMember()
  {
    assert(m_constSymbol);
    return m_constSymbol->isDataMember();
  }

  bool NodeConstant::isReadyConstant()
  {
    return m_ready;
  }

  FORECAST NodeConstant::safeToCastTo(UTI newType)
  {
    if(isReadyConstant())
      return NodeTerminal::safeToCastTo(newType);
    return CAST_HAZY;
  } //safeToCastTo

  UTI NodeConstant::checkAndLabelType()
  {
    UTI it = Nav;

    bool stubcopy = m_state.isClassAStub(m_state.getCompileThisIdx());

    //instantiate, look up in class block; skip if stub copy and already ready.
    if(!stubcopy && m_constSymbol == NULL)
	checkForSymbol();
    else
      {
	stubcopy = m_state.hasClassAStub(m_state.getCompileThisIdx()); //includes ancestors
      }

    if(m_constSymbol)
      {
	it = m_constSymbol->getUlamTypeIdx();
      }
    else if(isReadyConstant() && stubcopy)
      {
	assert(m_state.okUTItoContinue(m_constType));
	setNodeType(m_constType); //t3565, t3640, t3641, t3642, t3652
	//stub copy case: still wants uti mapping
	it = NodeTerminal::checkAndLabelType();
      }
    else if(stubcopy)
      {
	// still need its symbol for a value
	// use the member class (unlike checkForSymbol)
      }

    // map incomplete UTI
    if(!m_state.isComplete(it)) //reloads to recheck
      {
	UTI mappedUTI = it;
	if(m_state.findaUTIAlias(it, mappedUTI))
	  {
	    std::ostringstream msg;
	    msg << "REPLACE " << prettyNodeName().c_str() << " for type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ", used with alias type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    it = mappedUTI;
	  }

	if(!m_state.isComplete(it)) //reloads to recheck
	  {
	    std::ostringstream msg;
	    msg << "Incomplete " << prettyNodeName().c_str() << " for type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ", used with constant symbol name '";
	    msg << m_state.getTokenDataAsString(m_token).c_str() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    //wait until updateConstant tried.
	  }
      }

    setNodeType(it);
    Node::setStoreIntoAble(TBOOL_FALSE);

    assert(m_state.isScalar(it));

    //copy m_constant from Symbol into NodeTerminal parent.
    if(!isReadyConstant())
      m_ready = updateConstant(); //sets ready here
    if(!isReadyConstant())
      {
	it = Hzy;
	if(!stubcopy)
	  m_constSymbol = NULL; //lookup again too! (e.g. inherited template instances)
	m_state.setGoAgain();
      }

    return it;
  } //checkAndLabelType

  void NodeConstant::checkForSymbol()
  {
    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_token.m_dataindex, asymptr, hazyKin))
      {
	if(asymptr->isConstant())
	  {
	    m_constSymbol = (SymbolConstantValue *) asymptr;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(1) <" << m_state.getTokenDataAsString(m_token).c_str();
	    msg << "> is not a constant, and cannot be used as one with class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Named Constant <" << m_state.getTokenDataAsString(m_token).c_str();
	msg << "> is not defined, or was used before declared in a function";
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
    m_state.popClassContext(); //restore

    if(m_constSymbol && !m_constSymbol->isDataMember() && !m_constSymbol->isLocalsFilescopeDef() && !m_constSymbol->isClassArgument() && (m_constSymbol->getDeclNodeNo() > getNodeNo()))
      {
	NodeBlock * currBlock = getBlock();
	currBlock = currBlock->getPreviousBlockPointer();
	std::ostringstream msg;
	msg << "Named constant '" << getName();
	msg << "' was used before declared in a function scope";
	if(currBlock)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    setBlockNo(currBlock->getNodeNo());
	    m_constSymbol = NULL; //t3323
	    return checkForSymbol();
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    m_constSymbol = NULL;
	  }
      }
  } //checkForSymbol

  void NodeConstant::setBlockNo(NNO n)
  {
    assert(n > 0);
    m_currBlockNo = n;
  }

  NNO NodeConstant::getBlockNo() const
  {
    return m_currBlockNo;
  }

  NodeBlock * NodeConstant::getBlock()
  {
    assert(m_currBlockNo);
    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo); //t3328, t3329, t3330, t3332 (not using StubFirst version)
    if(!currBlock)
      {
	UTI anotherclassuti = m_state.findAClassByNodeNo(m_currBlockNo);
	if(anotherclassuti != Nav)
	  {
	    currBlock = m_state.getAClassBlock(anotherclassuti);
	    assert(currBlock);
	    if(currBlock->getNodeNo() != m_currBlockNo)
	      currBlock = NULL;
	  }
	//try local scope
	if(!currBlock)
	  currBlock = m_state.findALocalsScopeByNodeNo(m_currBlockNo);
      }
    assert(currBlock);
    return currBlock;
  }

  //class context set prior to calling us; purpose is to get
  // the value of this constant from the context before
  // constant folding happens.
  bool NodeConstant::assignClassArgValueInStubCopy()
  {
    // insure current block NNOs match
    if(m_currBlockNo != m_state.getCurrentBlockNo())
      {
	std::ostringstream msg;
	msg << "Block NNO " << m_currBlockNo << " for <";
	msg << m_state.getTokenDataAsString(m_token).c_str();
	msg << "> does not match the current block no ";
	msg << m_state.getCurrentBlockNo();
	msg << "; its value cannot be used in stub copy, with class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	return false;
      }

    if(m_ready)
      return true; //nothing to do

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_token.m_dataindex, asymptr, hazyKin))
      {
	assert(hazyKin); //always hazy, right?
	if(asymptr->isConstant() && ((SymbolConstantValue *) asymptr)->isReady()) //???
	  {
	    u64 val = 0;
	    ((SymbolConstantValue *) asymptr)->getValue(val);
	    m_constant.uval = val;
	    m_ready = true;
	    //note: m_constSymbol may be NULL; ok in this circumstance (i.e. stub copy).
	  }
      }
    return m_ready;
  } //assignClassArgValueInStubCopy

  EvalStatus NodeConstant::eval()
  {
    if(!isReadyConstant())
      m_ready = updateConstant();
    if(!isReadyConstant())
      return ERROR;
    if(!m_state.isComplete(getNodeType()))
      return ERROR;
    return NodeTerminal::eval();
  } //eval

  EvalStatus NodeConstant::evalToStoreInto()
  {
    //possible constant array item (t3881)
    m_state.abortShouldntGetHere();
    return UNEVALUABLE;
  }

  void NodeConstant::genCode(File * fp, UVPass& uvpass)
  {
    if(!isReadyConstant())
      m_ready = updateConstant(); //sets ready here
    assert(isReadyConstant()); //must be
    NodeTerminal::genCode(fp, uvpass);
  } //genCode

  bool NodeConstant::updateConstant()
  {
    u64 val;
    if(!m_constSymbol)
      return false;

    if(m_constSymbol->getValue(val))
	m_constant.uval = val; //value fits type per its constantdef
      //else don't want default value here

    return m_constSymbol->isReady();
  } //updateConstant

} //end MFM
