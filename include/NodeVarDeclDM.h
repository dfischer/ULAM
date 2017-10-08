/**                                        -*- mode:C++ -*-
 * NodeVarDeclDM.h -  Data Member Variable Declarations for ULAM
 *
 * Copyright (C) 2015-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2017 Ackleyshack LLC.
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
  \file NodeVarDeclDM.h -  Data Member Variable Declarations for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2017 All rights reserved.
  \gpl
*/


#ifndef NODEVARDECLDM_H
#define NODEVARDECLDM_H

#include "NodeVarDecl.h"

namespace MFM{

  class NodeVarDeclDM : public NodeVarDecl
  {
  public:

    NodeVarDeclDM(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeVarDeclDM(const NodeVarDeclDM& ref);

    virtual ~NodeVarDeclDM();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * f);

    virtual void noteTypeAndName(s32 totalsize, u32& accumsize);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool hasASymbolDataMember();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual bool checkReferenceCompatibility(UTI uti);

    virtual UTI checkAndLabelType();

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual bool buildDefaultValue(u32 wlen, BV8K& dvref);

    virtual void genCodeDefaultValueStringRegistrationNumber(File * fp, u32 startpos);

    virtual void genCodeElementTypeIntoDataMemberDefaultValue(File * fp, u32 startpos);

    virtual void setInitExpr(Node * node); //was setConstantExpr

    virtual bool foldArrayInitExpression(); //was foldConstantExpression

    virtual void packBitsInOrderOfDeclaration(u32& offset);

    virtual void printUnresolvedVariableDataMembers();

    virtual void printUnresolvedLocalVariables();

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeConstantArrayInitialization(File * fp);

    virtual void generateBuiltinConstantArrayInitializationFunction(File * fp, bool declOnly);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

    virtual void addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers);

  protected:

    virtual UlamValue makeUlamValuePtr(); //for dm

  private:

    bool updateConstant(u64 & newconst);
    bool updateConstant32(u64 & newconst);
    bool updateConstant64(u64 & newconst);

    void foldDefaultClass();
    void genCodedBitFieldTypedef(File * fp, UVPass& uvpass);

    bool checkDataMemberSizeConstraints(); //during c&l
  };
} //MFM

#endif //end NODEVARDECLDM_H
