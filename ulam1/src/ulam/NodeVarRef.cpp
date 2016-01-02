#include <stdlib.h>
#include "NodeVarRef.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"

namespace MFM {

  NodeVarRef::NodeVarRef(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeVarDecl(sym, nodetype, state) { }

  NodeVarRef::NodeVarRef(const NodeVarRef& ref) : NodeVarDecl(ref) { }

  NodeVarRef::~NodeVarRef() { }

  Node * NodeVarRef::instantiate()
  {
    return new NodeVarRef(*this);
  }

  void NodeVarRef::updateLineage(NNO pno)
  {
    NodeVarDecl::updateLineage(pno);
  } //updateLineage

  bool NodeVarRef::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeVarDecl::findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeVarRef::checkAbstractInstanceErrors()
  {
    //unlike NodeVarDecl, an abstract class can be a reference!
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    if(nut->getUlamTypeEnum() == Class)
      {
	SymbolClass * csym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
	assert(isDefined);
	if(csym->isAbstract())
	  {
	    std::ostringstream msg;
	    msg << "Instance of Abstract Class ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << " used with reference variable symbol name '";
	    msg << getName() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
	  }
      }
  } //checkAbstractInstanceErrors

  //see also SymbolVariable: printPostfixValuesOfVariableDeclarations via ST.
  void NodeVarRef::printPostfix(File * fp)
  {
    NodeVarDecl::printPostfix(fp);
  } //printPostfix

  void NodeVarRef::printTypeAndName(File * fp)
  {
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamKeyTypeSignature vkey = m_state.getUlamKeyTypeSignatureByIndex(vuti);
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    fp->write(" ");
    if(vut->getUlamTypeEnum() != Class)
      fp->write(vkey.getUlamKeyTypeSignatureNameAndBitSize(&m_state).c_str());
    else
      fp->write(vut->getUlamTypeNameBrief().c_str());

    fp->write("& "); //<--the difference!!!
    fp->write(getName());

    s32 arraysize = m_state.getArraySize(vuti);
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	fp->write("[UNKNOWN]");
      }
  } //printTypeAndName

  const char * NodeVarRef::getName()
  {
    return NodeVarDecl::getName(); //& ??
  }

  const std::string NodeVarRef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeVarRef::safeToCastTo(UTI newType)
  {
    UTI nuti = getNodeType();
    nuti = m_state.getUlamTypeAsDeref(nuti);
    //cast RHS if necessary and safe
    //insure lval is same bitsize/arraysize
    // if classes, safe to cast a subclass to any of its superclasses
    FORECAST rscr = CAST_CLEAR;
    if(UlamType::compare(nuti, newType, m_state) != UTIC_SAME)
      {
	UlamType * nut = m_state.getUlamTypeByIndex(nuti);
	UlamType * newt = m_state.getUlamTypeByIndex(newType);
	rscr = newt->safeCast(nuti);

	if((nut->getUlamTypeEnum() == Class))
	  {
	    if(rscr != CAST_CLEAR)
	      {
		std::ostringstream msg;
		msg << "Incompatible class types ";
		msg << nut->getUlamTypeNameBrief().c_str();
		msg << " and ";
		msg << newt->getUlamTypeNameBrief().c_str();
		msg << " used to initalize reference '" << getName() <<"'";
		if(rscr == CAST_HAZY)
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		else
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    //primitives must be exactly the same size;
	    // even "clear" when complete yet not the same is "bad".
	    if(rscr != CAST_CLEAR)
	      {
		std::ostringstream msg;
		msg << "Reference variable " << getName() << "'s type ";
		msg << nut->getUlamTypeNameBrief().c_str();
		msg << ", and its initial value type ";
		msg << newt->getUlamTypeNameBrief().c_str();
		msg << ", are incompatible";
		if(rscr == CAST_HAZY)
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		else
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		rscr = CAST_BAD;
	      }
	  }
      }
    return rscr;
  } //safeToCastTo

  UTI NodeVarRef::checkAndLabelType()
  {
    UTI it = NodeVarDecl::checkAndLabelType();

    assert((it == Nav) || m_state.getUlamTypeByIndex(it)->isReference());

    ////requires non-constant, non-funccall value
    //NOASSIGN REQUIRED (e.g. for function parameters) doesn't have to have this!
    if(m_nodeInitExpr)
      {
	UTI eit = m_nodeInitExpr->getNodeType();
	if(eit == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Storage expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", is invalid";
	    if(!m_nodeInitExpr->isStoreIntoAble())
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //possibly still hazy
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	//if(m_storageExpr->isStoreIntoAble())
	Symbol * storsym = NULL;
	if(m_nodeInitExpr->getSymbolPtr(storsym) && (storsym->isConstant() || storsym->isFunction()))
	  {
	    std::ostringstream msg;
	    msg << "Storage expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", must be storeintoable";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }
      }

    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  void NodeVarRef::packBitsInOrderOfDeclaration(u32& offset)
  {
    assert(0); //refs can't be data members
  } //packBitsInOrderOfDeclaration

  void NodeVarRef::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    return NodeVarDecl::calcMaxDepth(depth, maxdepth, base);
  } //calcMaxDepth

  void NodeVarRef::countNavNodes(u32& cnt)
  {
    NodeVarDecl::countNavNodes(cnt);
  } //countNavNodes

  EvalStatus NodeVarRef::eval()
  {
    assert(m_varSymbol);

    assert(m_varSymbol->getAutoLocalType() != ALT_AS); //NodeVarRefAuto

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    assert(m_varSymbol->getUlamTypeIdx() == nuti);
    assert(m_nodeInitExpr);

    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr

    EvalStatus evs = m_nodeInitExpr->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.popArg();
    ((SymbolVariableStack *) m_varSymbol)->setAutoPtrForEval(pluv); //for future ident eval uses
    ((SymbolVariableStack *) m_varSymbol)->setAutoStorageTypeForEval(m_nodeInitExpr->getNodeType()); //for future virtual function call eval uses

    m_state.m_funcCallStack.storeUlamValueInSlot(pluv, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex()); //doesn't seem to matter..

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus  NodeVarRef::evalToStoreInto()
  {
    evalNodeProlog(0); //new current node eval frame pointer

    // (from NodeIdent's makeUlamValuePtr)
    // return ptr to this data member within the m_currentObjPtr
    // 'pos' modified by this data member symbol's packed bit position
    UlamValue rtnUVPtr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId());

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  }

  void NodeVarRef::genCode(File * fp, UlamValue & uvpass)
  {
    //reference always has initial value, unless func param
    assert(m_varSymbol->isAutoLocal());
    assert(m_varSymbol->getAutoLocalType() != ALT_AS);

    if(m_nodeInitExpr)
      {
	// get the right-hand side, stgcos
	// can be same type (e.g. element, quark, or primitive),
	// or ancestor quark if a class.
	m_nodeInitExpr->genCodeToStoreInto(fp, uvpass);

	assert(m_state.m_currentObjSymbolsForCodeGen.size() == 1);
	Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen.back();
	UTI stgcosuti = stgcos->getUlamTypeIdx();
	UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

	UTI vuti = m_varSymbol->getUlamTypeIdx(); //i.e. this node
	UlamType * vut = m_state.getUlamTypeByIndex(vuti);
	ULAMCLASSTYPE vclasstype = vut->getUlamClass();

	assert(vut->getUlamTypeEnum() == stgcosut->getUlamTypeEnum());

	m_state.indent(fp);
	fp->write(vut->getUlamTypeImmediateAutoMangledName().c_str()); //for C++ local vars, ie non-data members
	if(vclasstype == UC_ELEMENT)
	  fp->write("<EC> ");
	else if(vclasstype == UC_QUARK)
	  {
	    fp->write("<EC, ");
	    fp->write_decimal_unsigned(uvpass.getPtrPos());
	    fp->write("u> ");
	  }
	else //primitive, right-just
	  {
	    fp->write("<EC, ");
	    if(stgcos->isDataMember())
	      fp->write("T::ATOM_FIRST_STATE_BIT + ");
	    //ptr pos is absolute for non-data members (r-just primitives)
	    fp->write_decimal_unsigned(uvpass.getPtrPos());
	    fp->write("u> ");
	  }

	fp->write(m_varSymbol->getMangledName().c_str());
	fp->write("("); //pass ref in constructor (ref's not assigned with =)
	if(stgcos->isDataMember()) //can't be an element
	  {
	    fp->write("Uv_4atom, ");
	    fp->write_decimal_unsigned(stgcos->getPosOffset()); //relative off
	    fp->write("u");
	  }
	else
	  {
	    fp->write(stgcos->getMangledName().c_str());
	    if(stgcos->getId() != m_state.m_pool.getIndexForDataString("atom")) //not isSelf check; was "self"
	      fp->write(".getRef()");

	    if(vclasstype == UC_NOTACLASS)
	      {
		fp->write(", ");
		fp->write_decimal_unsigned(BITSPERATOM - stgcosut->getTotalBitSize()); //right-justified
		fp->write("u");
	      }
	    else if(vclasstype == UC_QUARK)
	      fp->write(", 0u"); //left-justified
	  }
	fp->write(");\n");
      } //storage

    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of rhs ?
  } //genCode

} //end MFM