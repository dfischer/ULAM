#include <stdio.h>
#include "NodeBinaryOpShift.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpShift::NodeBinaryOpShift(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left, right, state) {}

  NodeBinaryOpShift::NodeBinaryOpShift(const NodeBinaryOpShift& ref) : NodeBinaryOp(ref) {}

  NodeBinaryOpShift::~NodeBinaryOpShift() {}

  UTI NodeBinaryOpShift::checkAndLabelType()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI leftType = m_nodeLeft->checkAndLabelType();
    UTI rightType = m_nodeRight->checkAndLabelType();
    UTI newType = calcNodeType(leftType, rightType); //Bits, or Nav error

    setNodeType(newType);
    setStoreIntoAble(false);

    if(newType != Nav && m_state.isComplete(newType))
      {
	if(leftType != newType)
	  {
	    if(!makeCastingNode(m_nodeLeft, newType, m_nodeLeft))
	      {
		newType = Nav;
		setNodeType(Nav);
	      }
	  }
	//shift by unsigned type; cast if need be. safety checked by calcNodeType.
	if(m_state.getUlamTypeByIndex(rightType)->getUlamTypeEnum() != Unsigned)
	  {
	    if(!makeCastingNode(m_nodeRight, Unsigned, m_nodeRight))
	      {
		newType = Nav;
		setNodeType(Nav);
	      }
	  }
      } //complete

    if(newType != Nav && isAConstant() && m_nodeLeft->isReadyConstant() && m_nodeRight->isReadyConstant())
      return constantFold();

    return newType;
  } //checkAndLabelType

  // third arg is the slots for the rtype; slots for the left is
  // rslot-lslot; they should be equal, unless one is a packed array
  // and the other isn't; however, currently, according to
  // CompilerState determinePackable, they should always be the same
  // since their types must be identical.
  void NodeBinaryOpShift::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    UTI nuti = getNodeType();
    if(m_state.isScalar(nuti)) //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	// leverage case when both are packed, for shift operations
	if(m_state.determinePackable(nuti) == PACKEDLOADABLE)
	  {
	    doBinaryOperationImmediate(lslot, rslot, slots);
	  }
	else
	  {
	    doBinaryOperationArray(lslot, rslot, slots);
	  }
      }
  } //dobinaryoperation

  UTI NodeBinaryOpShift::calcNodeType(UTI lt, UTI rt)  //shift
  {
    if(!m_state.isComplete(lt) || !m_state.isComplete(rt))
      return Nav;

    //no atoms, elements, nor void as either operand
    if(!NodeBinaryOp::checkForPrimitiveTypes(lt, rt))
	return Nav;

    UTI newType = Nav; //init
    // change! LHS must be Bits..up to ulam programmer to cast
    // (including quarks).
    if(NodeBinaryOp::checkScalarTypesOnly(lt, rt))
      {
	bool bok = true;

	// RHS must be Unsigned, or positive constant.
	// shift ops take unsigned as second arg;
	//note: C implementations typically shift by the lower 5 bits (6 for 64-bits) only.
	UlamType * rut = m_state.getUlamTypeByIndex(rt);
	ULAMTYPE retype = rut->getUlamTypeEnum();
	bool rconst = m_nodeRight->isAConstant(); //unready const will by hazy safecast
	FORECAST scr = rconst ? m_nodeRight->safeToCastTo(Unsigned) : CAST_BAD; //bypass var
	if((rconst && scr != CAST_CLEAR) || (!rconst && retype != Unsigned))
	  {
	    //this is an unsafe cast of constant (e.g. negative number),
	    // or not an Unsigned var, so casting to Unsigned must be explicit!
	    std::ostringstream msg;
	    msg << "Unsigned is the supported type for RHS bitwise shift value, operator";
	    msg << getName() << "; Suggest casting ";
	    msg << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	    msg << " to Unsigned";
	    if(scr == CAST_HAZY)
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    bok = false; //Nav;
	  }

	newType = lt;
	s32 lbs = m_state.getBitSize(lt);

	//ERROR if LHS is not Bits, except for constants
	bool lconst = m_nodeLeft->isAConstant();
	if(!lconst)
	  {
	    ULAMTYPE etyp = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	    if(etyp != Bits)
	      {
		std::ostringstream msg;
		msg << "Bits is the supported type for bitwise shift operator";
		msg << getName() << "; Suggest casting ";
		msg << m_state.getUlamTypeNameBriefByIndex(lt).c_str();
		msg << " to Bits";
		if(lbs > 0)
		  msg << "(" << lbs << ")";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		bok = false; //newType = Nav;
	      }
	  }
	else
	  {
	    //auto for constants, downhill cast. use lt bitsize.
	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Bits"), lbs);
	    newType = m_state.makeUlamType(newkey, Bits);
	  }

	if(!bok)
	  newType = Nav;
      } //both scalars
    return newType;
  } //calcNodeType

  const std::string NodeBinaryOpShift::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    //methodname << "_ShiftLeft";  determined by each op
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());

    // common part of name
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
      case Unsigned:
      case Bool:
      case Unary:
      case Bits:
	methodname << UlamType::getUlamTypeEnumAsString(etyp);
	break;
      default:
	assert(0);
	methodname << "NAV";
	break;
      };
    methodname << nut->getTotalWordSize();
    return methodname.str();
  } // methodNameForCodeGen

} //end MFM
