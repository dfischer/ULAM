/**                                        -*- mode:C++ -*-
 * Node.h - Basic Node of Nodes for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2017 Ackleyshack LLC.
 *
 * This file is part of the ULAM programming language compilation system.
 *
 * The ULAM programming language compilation system is free software:
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * The ULAM programming language compilation system is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the ULAM programming language compilation system
 * software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

/**
  \file Node.h - Basic Node of Nodes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef NODE_H
#define NODE_H

#include <sstream>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include "BitVector.h"
#include "CastOps.h"
#include "File.h"
#include "Locator.h"
#include "Symbol.h"
#include "SymbolTmpVar.h"
#include "UlamType.h"
#include "UlamValue.h"
#include "UVPass.h"
#include "MapClassMemberDesc.h"

namespace MFM{

  enum EVALS { EVAL_RHS, EVAL_LHS, EVAL_SIDEEFFECTS};
  enum EvalStatus {ERROR, NOTREADY, NORMAL, RETURN, BREAK, CONTINUE, UNEVALUABLE};

  struct CompilerState; //forward
  struct TypeArgs; //forward
  class NodeFunctionCall; //forward

  class Node
  {
  public:

    Node(CompilerState & state);

    Node(const Node& ref);

    virtual ~Node() {}

    virtual Node * instantiate() = 0;

    virtual void setYourParentNo(NNO pno);

    NNO getYourParentNo() const;

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    NNO getNodeNo() const;

    virtual void resetNodeNo(NNO no); //for constant folding

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp) = 0;

    virtual void noteTypeAndName(s32 totalsize, u32& accumsize);

    virtual const char * getName() = 0;

    virtual u32 getTypeNameId();

    virtual const std::string prettyNodeName() = 0;

    const std::string nodeName(const std::string& prettyFunction);

    UTI getNodeType() const;

    void setNodeType(UTI ut);

    virtual TBOOL getStoreIntoAble() const;

    void setStoreIntoAble(TBOOL s);

    virtual TBOOL getReferenceAble() const;

    void setReferenceAble(TBOOL s);

    Locator getNodeLocation() const;

    virtual void setNodeLocation(Locator loc);

    virtual void resetNodeLocations(Locator loc);

    void printNodeLocation(File * fp);

    std::string getNodeLocationAsString() const;

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual bool getStorageSymbolPtr(Symbol *& symptrref);

    virtual bool hasASymbolDataMember();

    virtual bool hasASymbolSuper();

    virtual bool hasASymbolSelf();

    virtual bool hasASymbolReference();

    virtual bool isAConstant();

    virtual bool isReadyConstant();

    virtual bool isNegativeConstant();

    virtual bool isWordSizeConstant();

    virtual bool isFunctionCall();

    virtual bool isAConstructorFunctionCall();

    virtual bool isArrayItem();

    virtual bool isAList();

    virtual bool isExplicitReferenceCast(); //only NodeCast may return true

    virtual bool isExplicitCast(); //only NodeCast may return true

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual bool exchangeNodeWithParent(Node * newnode);

    virtual bool trimToTheElement(Node ** fromleftnode, Node *& rtnnodeptr);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual UTI constantFold();

    virtual bool buildDefaultValue(u32 wlen, BV8K& dvref);

    virtual void genCodeDefaultValueStringRegistrationNumber(File * fp, u32 startpos);

    virtual void genCodeElementTypeIntoDataMemberDefaultValue(File * fp, u32 startpos);

    virtual bool installSymbolTypedef(TypeArgs& args, Symbol *& asymptr);

    virtual bool installSymbolConstantValue(TypeArgs& args, Symbol *& asymptr);

    virtual bool installSymbolModelParameterValue(TypeArgs& args, Symbol *& asymptr);

    virtual bool installSymbolVariable(TypeArgs& args,  Symbol *& asymptr);

    virtual bool assignClassArgValueInStubCopy();

    virtual EvalStatus eval() = 0;

    virtual EvalStatus evalToStoreInto();

    virtual UlamValue makeUlamValuePtr();

    virtual void packBitsInOrderOfDeclaration(u32& offset);

    virtual void printUnresolvedVariableDataMembers();

    virtual void printUnresolvedLocalVariables(u32 fid);

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

    virtual void genCodeReadIntoATmpVar(File * fp, UVPass& uvpass);

    virtual void genCodeWriteFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    virtual void genCodeConstantArrayInitialization(File * fp);

    virtual void generateBuiltinConstantArrayInitializationFunction(File * fp, bool declOnly);

    virtual void cloneAndAppendNode(std::vector<Node *> & cloneVec);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

    virtual void addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers);

    virtual void genCodeExtern(File * fp, bool declOnly);

    /**
     * Returns converted const argument to all capital letters as a string
     */
    static std::string allCAPS(const char * s);

  protected:

    CompilerState & m_state;  //for printing error messages with path

    virtual bool checkSafeToCastTo(UTI fromType, UTI& newType);

    bool makeCastingNode(Node * node, UTI tobeType, Node*& rtnNode, bool isExplicit = false);

    bool warnOfNarrowingCast(UTI nodeType, UTI tobeType);

    void evalNodeProlog(u32 depth);

    void evalNodeEpilog();

    u32 makeRoomForNodeType(UTI type, STORAGE where = EVALRETURN);

    u32 makeRoomForSlots(u32 slots, STORAGE where = EVALRETURN);

    void assignReturnValueToStack(UlamValue rtnUV, STORAGE where = EVALRETURN);

    void assignReturnValuePtrToStack(UlamValue rtnUVptr, STORAGE where = EVALRETURN);

    UlamValue assignAnonymousClassReturnValueToStack(UlamValue rtnUV);

    bool returnValueOnStackNeededForEval(UTI rtnType);

    virtual void genMemberNameOfMethod(File * fp, UVPass& uvpass, bool endingdot = true); //helper method to read/write into/from tmpvar
    virtual void genModelParameterMemberNameOfMethod(File * fp, s32 epi);

    virtual void genLocalMemberNameOfMethod(File * fp, UVPass& uvpass);

    void genLocalMemberNameOfMethodForAtomof(File * fp, UVPass& uvpass);

    //return index of stgcos in stack of symbols, -1 if stack is empty and currentSelf.
    s32 loadStorageAndCurrentObjectSymbols(Symbol *& stgcosref, Symbol *&cosref);

    //i.e. an immediate (right-justified); not a data member or self;
    bool isCurrentObjectALocalVariableOrArgument();

    //index of last "static" MP object; o.w.-1
    s32 isCurrentObjectsContainingAModelParameter();

    //index of last named constant (array) object; o.w.-1
    s32 isCurrentObjectsContainingAConstant();

    //index of first element or ele ref object; o.w. -1
    s32 isCurrentObjectsContainingAnElement();

    std::string calcPosOfCurrentObjectClassesAsString(UVPass uvpass);

    //false means its the entire array or not an array at all
    bool isCurrentObjectAnArrayItem(UTI cosuti, UVPass uvpass);

    bool isCurrentObjectACustomArrayItem(UTI cosuti, UVPass uvpass);

    bool isCurrentObjectAnUnpackedArray(UTI cosuti, UVPass uvpass);

    bool isHandlingImmediateType();

    //true if a non-ref, scalar element
    bool needAdjustToStateBits(UTI cuti);

    void adjustUVPassForElements(UVPass & uvpass);

    SymbolTmpVar * makeTmpVarSymbolForCodeGen(UVPass& uvpass, Symbol * sym);

    std::string genUlamRefUsageAsString(UTI uti);

    void genCodeConvertATmpVarIntoBitVector(File * fp, UVPass & uvpass);

    void genCodeConvertABitVectorIntoATmpVar(File * fp, UVPass & uvpass);

    //e.g. when lhs of member select is an array item of class type
    void genCodeConvertATmpVarAutoRefIntoAutoRef(File * fp, UVPass & uvpass);

    //e.g. when a variable array item, may be of class type
    void genCodeConvertATmpVarIntoAutoRef(File * fp, UVPass & luvpass, UVPass ruvpass);

    //e.g. when constant array item, primitive
    void genCodeConvertATmpVarIntoConstantAutoRef(File * fp, UVPass & luvpass, UVPass ruvpass);
    //e.g. when lhs of member select is an array item of class type, rhs data member
    void genCodeARefFromARefStorage(File * fp, UVPass stguvpass, UVPass uvpass);

    //e.g. when a ref to a primitive is used as a lhs binary op, or if condition (t41140)
    void genCodeReadAutorefIntoATmpVar(File * fp, UVPass& uvpass);

    void genCodeReferenceInitialization(File * fp, UVPass& uvpass, Symbol * vsymptr);

    virtual void checkForSymbol();

    void genCodeReadElementTypeField(File * fp, UVPass & uvpass);

    void restoreElementTypeForAncestorCasting(File * fp, UVPass & uvpass);

    //common helpers for safe casting
    bool buildCastingFunctionCallNode(Node * node, UTI tobeType, Node*& rtnNode);
    Node * buildToIntCastingNode(Node * node);
    Node * newCastingNode(Node * node, UTI tobeType);
    bool newCastingNodeWithCheck(Node * node, UTI tobeType, Node*& rtnNode);

    //used for function calls second arg, including custom array accessors
    std::string genHiddenArg2(UVPass uvpass, u32& urtmpnumref);
    virtual u32 getLengthOfMemberClassForHiddenArg(UTI cosuti);

  private:
    UTI m_utype;
    TBOOL m_storeIntoAble;
    TBOOL m_referenceAble;
    Locator m_loc;
    NNO m_parentNo;
    NNO m_no;

    void genCodeReadSelfIntoATmpVar(File * fp, UVPass & uvpass);
    void genSelfNameOfMethod(File * fp);
    void genCodeWriteToSelfFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    void genCodeReadTransientIntoATmpVar(File * fp, UVPass & uvpass);
    void genCodeWriteToTransientFromATmpVar(File * fp, UVPass & luvpass, UVPass & ruvpass);

    void genCodeWriteToAtomofRefFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    void genCodeWriteToAutorefFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    virtual void genModelParameterHiddenArgs(File * fp, s32 epi);

    void genCustomArrayMemberNameOfMethod(File * fp);
    void genCustomArrayHiddenArgs(File * fp, u32 urtmpnum);

    void genLocalMemberNameOfMethodByUsTypedef(File * fp, UVPass& uvpass);
    void genCustomArrayLocalMemberNameOfMethod(File * fp);

    const std::string localStorageTypeAsString(UTI nuti);

    const std::string tmpStorageTypeForRead(UTI nuti, UVPass uvpass);
    const std::string tmpStorageTypeForReadArrayItem(UTI nuti, UVPass uvpass);

    const std::string readMethodForCodeGen(UTI nuti, UVPass uvpass);
    const std::string readArrayItemMethodForCodeGen(UTI nuti, UVPass uvpass);

    const std::string writeMethodForCodeGen(UTI nuti, UVPass uvpass);
    const std::string writeArrayItemMethodForCodeGen(UTI nuti, UVPass uvpass);

    void genCodeArrayRefInit(File * fp, UVPass & uvpass, Symbol * vsymptr);
    void genCodeArrayItemRefInit(File * fp, UVPass & uvpass, Symbol * vsymptr);
  };

}

#endif //end NODE_H
