/**                                        -*- mode:C++ -*-
 * SymbolFunctionName.h -  Function Symbol Name handling for ULAM
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
  \file SymbolFunctionName.h -  Function Symbol Name handling for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017   All rights reserved.
  \gpl
*/


#ifndef SYMBOLFUNCTIONNAME_H
#define SYMBOLFUNCTIONNAME_H

#include <map>
#include <vector>
#include "Symbol.h"
#include "SymbolFunction.h"
#include "MapClassMemberDesc.h"

namespace MFM{

  class Node; //forward
  class NodeBlockClass; //forward
  class SymbolTable; //forward

  class SymbolFunctionName : public Symbol
  {
  public:
    SymbolFunctionName(const Token& id, UTI typetoreturn, CompilerState& state);

    SymbolFunctionName(const SymbolFunctionName& sref);

    virtual ~SymbolFunctionName();

    virtual Symbol * clone();

    virtual bool isFunction();

    virtual const std::string getMangledPrefix();

    bool isOperatorOverloadFunctionName();
    void setOperatorOverloadFunctionName();

    bool overloadFunction(SymbolFunction * fsym);

    u32 findMatchingFunctionStrictlyByTypes(std::vector<UTI> argTypes, SymbolFunction *& funcSymbol, bool& hasHazyArgs);

    u32 findMatchingFunctionStrictlyVoid(SymbolFunction *& funcSymbol);

    u32 findMatchingFunction(std::vector<Node*> argNodes, SymbolFunction *& funcSymbol, bool& hasHazyArgs);

    u32 findMatchingFunctionWithSafeCasts(std::vector<Node*> argNodes, SymbolFunction *& funcSymbol, bool& hasHazyArgs);

    u32 findMatchingFunctionWithSafeCastsInAncestors(std::vector<Node*> argNodes, SymbolFunction *& funcSymbol, bool& hasHazyArgs);

    void noteAmbiguousFunctionSignatures(std::vector<Node *> argNodes, u32 numMatchesFound);

    u32 getDepthSumOfFunctions();

    void calcMaxDepthOfFunctions(); //called after all UTI sizes are known

    void calcMaxIndexOfVirtualFunctions(s32& maxidx);

    void setupConstantSlotIndexesInFunctions(u32& cslotidx);

    void checkAbstractInstanceErrorsInFunctions();

    void checkFunctionNames(std::map<std::string, UTI>& mangledFunctionMap, u32& probcount);

    void checkFunctionNamesInAncestor(std::map<std::string, UTI>& mangledFunctionMap, u32& probcount);

    u32 checkCustomArrayGetFunctions(UTI& rtnType);

    UTI getCustomArrayReturnType();

    u32 getCustomArrayIndexTypeFor(Node * rnode, UTI& idxuti, bool& hasHazyArgs);

    void linkToParentNodesInFunctionDefs(NodeBlockClass * p);

    void updatePrevBlockPtrInFunctionDefs(NodeBlockClass * p);

    bool findNodeNoInFunctionDefs(NNO n, Node*& foundNode);

    bool labelFunctions();

    void countNavNodesInFunctionDefs(u32& ncnt, u32& hcnt, u32& nocnt);

    void printUnresolvedLocalVariablesInFunctionDefs();

    u32 countNativeFuncDecls();

    void generateCodedFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    void addFunctionDescriptionsToClassMemberMap(UTI classType, ClassMemberMap & classmembers);

  protected:

  private:
    bool m_isOperatorOverload;
    std::map<std::string, SymbolFunction *> m_mangledFunctionNames; //mangled func name -> symbol function ptr

    bool isDefined(std::string mangledFName, SymbolFunction * & foundSym);
    bool checkForDuplicateFunctionSignature(std::map<std::string, UTI>& mangledFunctionMap, u32& probcount, SymbolFunction * fsym);
  };

}

#endif //end SYMBOLFUNCTIONNAME_H
