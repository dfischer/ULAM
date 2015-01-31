/**                                        -*- mode:C++ -*-
 * Node.h - Basic Node of Nodes for ULAM
 *
 * Copyright (C) 2014-2015 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2015 Ackleyshack LLC.
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
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODE_H
#define NODE_H

#include <sstream>
#include <stdio.h>
#include <assert.h>
#include "CastOps.h"
#include "File.h"
#include "Locator.h"
#include "Symbol.h"
#include "UlamType.h"
#include "UlamValue.h"

namespace MFM{

  enum EVALS { EVAL_RHS, EVAL_LHS, EVAL_SIDEEFFECTS};
  enum EvalStatus {ERROR, NORMAL, RETURN, BREAK, CONTINUE};

  struct CompilerState; //forward

  class Node
  {
  public:

    Node(CompilerState & state);
    Node(const Node& ref);
    virtual ~Node() {}

    virtual Node * clone() = 0;

    virtual void setYourParent(Node * parent);
    virtual void updateLineage(Node * p);

    NNO getNodeNo();
    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void print(File * fp);
    virtual void printPostfix(File * fp) = 0;

    virtual const char * getName() = 0;

    virtual const std::string prettyNodeName() = 0;

    const std::string nodeName(const std::string& prettyFunction);

    UTI getNodeType();
    void setNodeType(UTI ut);

    bool isStoreIntoAble();
    void setStoreIntoAble(bool s);

    Locator getNodeLocation();
    void setNodeLocation(Locator loc);
    void printNodeLocation(File * fp);
    std::string getNodeLocationAsString();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual void constantFold(Token tok);

    virtual UTI checkAndLabelType();

    virtual void countNavNodes(u32& cnt);

    virtual bool fitsInBits(UTI fituti);
    virtual bool isNegativeConstant();
    virtual bool isWordSizeConstant();

    virtual bool installSymbolTypedef(Token atok, s32 bitsize, s32 arraysize, UTI classInstanceIdx, Symbol *& asymptr);
    virtual bool installSymbolConstantValue(Token atok, s32 bitsize, s32 arraysize, Symbol *& asymptr);
    virtual bool installSymbolVariable(Token atok, s32 bitsize, s32 arraysize, UTI classInstanceIdx, Symbol *& asymptr);

    virtual EvalStatus eval() = 0;
    virtual EvalStatus evalToStoreInto();

    void evalNodeProlog(u32 depth);
    void evalNodeEpilog();

    u32 makeRoomForNodeType(UTI type, STORAGE where = EVALRETURN);
    u32 makeRoomForSlots(u32 slots, STORAGE where = EVALRETURN);

    void assignReturnValueToStack(UlamValue rtnUV, STORAGE where = EVALRETURN);
    void assignReturnValuePtrToStack(UlamValue rtnUVptr);

    virtual void packBitsInOrderOfDeclaration(u32& offset);

    virtual void genCode(File * fp, UlamValue& uvpass);
    virtual void genCodeToStoreInto(File * fp, UlamValue& uvpass);
    virtual void genCodeReadIntoATmpVar(File * fp, UlamValue& uvpass);
    virtual void genCodeReadArrayItemIntoATmpVar(File * fp, UlamValue& uvpass);
    void genCodeReadCustomArrayItemIntoATmpVar(File * fp, UlamValue & uvpass);

    virtual void genCodeWriteFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass);
    void genCodeWriteToSelfFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass);
    virtual void genCodeWriteArrayItemFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass);
    void genCodeWriteCustomArrayItemFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass);

    void genCodeConvertATmpVarIntoBitVector(File * fp, UlamValue & uvpass);
    void genCodeConvertABitVectorIntoATmpVar(File * fp, UlamValue & uvpass);

    /**
     * Returns converted const argument to all capital letters as a string
     */
    static std::string allCAPS(const char * s);

  protected:

    CompilerState & m_state;  //for printing error messages with path
    Node * makeCastingNode(Node * node, UTI tobeType);
    bool warnOfNarrowingCast(UTI nodeType, UTI tobeType);

    virtual void genMemberNameOfMethod(File * fp); //helper method to read/write into/from tmpvar
    virtual void genElementParameterMemberNameOfMethod(File * fp, s32 epi);
    void genElementParameterHiddenArgs(File * fp, s32 epi);
    virtual void genLocalMemberNameOfMethod(File * fp);

    const std::string tmpStorageTypeForRead(UTI nuti, UlamValue uvpass);
    const std::string tmpStorageTypeForReadArrayItem(UTI nuti, UlamValue uvpass);

    const std::string readMethodForCodeGen(UTI nuti, UlamValue uvpass);
    const std::string readArrayItemMethodForCodeGen(UTI nuti, UlamValue uvpass);

    const std::string writeMethodForCodeGen(UTI nuti, UlamValue uvpass);
    const std::string writeArrayItemMethodForCodeGen(UTI nuti, UlamValue uvpass);

    const std::string readMethodForImmediateBitValueForCodeGen(UTI nuti, UlamValue uvpass);
    const std::string writeMethodForImmediateBitValueForCodeGen(UTI nuti, UlamValue uvpass);

    bool isCurrentObjectALocalVariableOrArgument();  //i.e. an immediate (right-justified); not a data member or self;
    s32 isCurrentObjectsContainingAnElementParameter(); //index of last "static" EP object; o.w.-1

    bool isCurrentObjectAnArrayItem(UTI cosuti, UlamValue uvpass); //false means its the entire array or not an array at all
    bool isCurrentObjectACustomArrayItem(UTI cosuti, UlamValue uvpass);

    bool isHandlingImmediateType();

    u32 adjustedImmediateArrayItemPtrPos(UTI cosuti, UlamValue uvpass);

  private:
    bool m_storeIntoAble;
    UTI m_nodeUType;
    Locator m_nodeLoc;
    Node * m_parent;
    NNO m_nodeNo;
  };

}

#endif //end NODE_H
