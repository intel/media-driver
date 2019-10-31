/*
* Copyright (c) 2019, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
// PatchInfo record.
//

#pragma once

#ifndef __CM_FC_PATCHINFO_RECORD_H__
#define __CM_FC_PATCHINFO_RECORD_H__

#include <cstddef>
#include <cstring>

#include <algorithm>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <tuple>

#include "PatchInfo.h"

namespace cm {

namespace patch {


class Binary;

/// Symbol has @p Name and its definition by @p Bin and @p Addr within that
/// binary. A symbol may not has a binary associated. In that case, that's
/// an *unresolved* symbol.
///
class Symbol {
  const char *Name; ///< Name of the symbol.
  unsigned Extra;   ///< Extra info.
  Binary *Bin;      ///< Binary where the symbol is defined.
  unsigned Addr;    ///< Symbol's address within that binary.

public:
  Symbol(const char *N, unsigned E, Binary *B, unsigned A)
    : Name(N), Extra(E), Bin(B), Addr(A) {}

  bool isUnresolved() const { return !Bin; }

  const char *getName() const { return Name; }
  void setName(const char *N) { Name = N; }

  unsigned getExtra() const { return Extra; }
  void setExtra(unsigned E) { Extra = E; }

  Binary *getBinary() const { return Bin; }
  void setBinary(Binary *B) { Bin = B; }

  unsigned getAddr() const { return Addr; }
  void setAddr(unsigned A) { Addr = A; }
};

/// Relocation defines a location referencing a symbol. That location is
/// defined by @p Bin and @p Offset within that binary. @p Sym is the symbol it
/// refrenced.
///
class Relocation {
  unsigned Offset;  ///< The offset in that binary where the relocation applies.
  Symbol *Sym;      ///< The symbol this relocation uses.

public:
  Relocation(unsigned Off, Symbol *S) : Offset(Off), Sym(S) {}

  Symbol *getSymbol() const { return Sym; }
  void setSymbol(Symbol *S) { Sym = S; }

  unsigned getOffset() const { return Offset; }
  void setOffset(unsigned Off) { Offset = Off; }
};


class RegAccess {
  unsigned Offset;
  unsigned RegNo;
  unsigned DefUseToken;

public:
  RegAccess(unsigned Off, unsigned R, unsigned DUT)
      : Offset(Off), RegNo(R), DefUseToken(DUT) {}

  unsigned getOffset() const { return Offset; }
  unsigned getRegNo() const { return RegNo; }
  unsigned getDUT() const { return DefUseToken; }

  std::pair<bool, unsigned> getToken() const {
    unsigned T = DefUseToken & cm::patch::RDUT_TOKMASK;
    bool HasToken = (T != RDUT_TOKMASK);
    return std::make_pair(HasToken, T);
  }

  bool isDef() const {
    return (DefUseToken & cm::patch::RDUT_DUMASK) == cm::patch::RDUT_FULLDEF;
  }

  bool isUse() const {
    return (DefUseToken & cm::patch::RDUT_DUMASK) == cm::patch::RDUT_FULLUSE;
  }

  bool isDefByToken() const {
    bool HasToken;
    std::tie(HasToken, std::ignore) = getToken();
    return HasToken && isDef();
  }

  bool isUseByToken() const {
    bool HasToken;
    std::tie(HasToken, std::ignore) = getToken();
    return HasToken && isUse();
  }

  bool isDefNotByToken() const {
    bool HasToken;
    std::tie(HasToken, std::ignore) = getToken();
    return !HasToken && isDef();
  }

  bool isUseNotByToken() const {
    bool HasToken;
    std::tie(HasToken, std::ignore) = getToken();
    return !HasToken && isUse();
  }

  bool operator==(RegAccess &Other) const {
    return Offset == Other.Offset &&
           RegNo == Other.RegNo &&
           DefUseToken == Other.DefUseToken;
  }
};

class Token {
  unsigned TokenNo;

public:
  Token(unsigned T) : TokenNo(T) {}

  unsigned getTokenNo() const { return TokenNo; }
};

class DepNode {
  typedef std::list<RegAccess *> RegAccRefList;
  typedef std::list<DepNode *> NodeRefList;

  Binary *Bin;
  unsigned Offset;
  bool Barrier;

  unsigned Distance;
  unsigned RdTokenMask;
  unsigned WrTokenMask;

  RegAccRefList AccList;

  // From node this node depends on.
  NodeRefList FromList;
  // To nodes which depends on this node. There are two kinds of such nodes:
  // one depends on registers used by this node and
  // the other depends on registers defined by this node.
  NodeRefList ToList[2]; // 0 - from Use of this node;
                         // 1 - from Def of this node.

public:
  DepNode(Binary *B, unsigned Off, bool IsBarrier)
    : Bin(B), Offset(Off), Barrier(IsBarrier), Distance(0),
      RdTokenMask(0), WrTokenMask(0) {}

  Binary *getBinary() const { return Bin; }
  unsigned getOffset() const { return Offset; }
  bool isBarrier() const { return Barrier; }

  void clearAccList() { AccList.clear(); }

  void appendRegAcc(RegAccess *Acc) {
    for (auto AI = AccList.begin(), AE = AccList.end(); AI != AE; ++AI) {
      if (*(*AI) == *Acc)
        return;
    }
    AccList.push_back(Acc);
  }

  void addFromNode(DepNode *N) { FromList.push_back(N); }
  void addToNode(DepNode *N, bool FromDef) { ToList[FromDef].push_back(N); }

  void clearFromNodes() { FromList.clear(); }
  void clearToNodes(bool FromDef) { ToList[FromDef].clear(); }

  bool from_empty() const { return FromList.empty(); }
  bool to_empty(bool FromDef) const { return ToList[FromDef].empty(); }

  RegAccRefList::iterator acc_begin() { return AccList.begin(); }
  RegAccRefList::iterator acc_end()   { return AccList.end(); }

  NodeRefList::iterator from_begin() { return FromList.begin(); }
  NodeRefList::iterator from_end()   { return FromList.end(); }

  NodeRefList::iterator to_begin(bool FromDef) {
    return ToList[FromDef].begin();
  }
  NodeRefList::iterator to_end(bool FromDef) {
    return ToList[FromDef].end();
  }

  unsigned getDistance() const { return Distance; }
  void setDistance(unsigned D) { Distance = D; }
  void updateDistance(unsigned D) {
    if (Distance == 0)
      Distance = D;
    else
      Distance = std::min(D, Distance);
  }

  unsigned getRdTokenMask() const { return RdTokenMask; }
  unsigned getWrTokenMask() const { return WrTokenMask; }
  void setRdTokenMask(unsigned M) { RdTokenMask = M; }
  void setWrTokenMask(unsigned M) { WrTokenMask = M; }
  void mergeRdTokenMask(unsigned M) { RdTokenMask |= M; }
  void mergeWrTokenMask(unsigned M) { WrTokenMask |= M; }

  bool isDef(unsigned Reg) const {
    for (auto AI = AccList.begin(), AE = AccList.end(); AI != AE; ++AI) {
      RegAccess *Acc = *AI;
      if (Acc->getRegNo() == Reg && Acc->isDef())
        return true;
    }
    return false;
  }

  bool isUse(unsigned Reg) const {
    for (auto AI = AccList.begin(), AE = AccList.end(); AI != AE; ++AI) {
      RegAccess *Acc = *AI;
      if (Acc->getRegNo() == Reg && Acc->isUse())
        return true;
    }
    return false;
  }

  bool isDefOnly(unsigned Reg) const {
    for (auto AI = AccList.begin(), AE = AccList.end(); AI != AE; ++AI) {
      RegAccess *Acc = *AI;
      if (Acc->getRegNo() == Reg && !Acc->isDef())
        return false;
    }
    return true;
  }

  bool isUseOnly(unsigned Reg) const {
    for (auto AI = AccList.begin(), AE = AccList.end(); AI != AE; ++AI) {
      RegAccess *Acc = *AI;
      if (Acc->getRegNo() == Reg && !Acc->isUse())
        return false;
    }
    return true;
  }

  bool isDefByToken(unsigned Reg) const {
    for (auto AI = AccList.begin(), AE = AccList.end(); AI != AE; ++AI) {
      RegAccess *Acc = *AI;
      if (Acc->getRegNo() == Reg && Acc->isDefByToken())
        return true;
    }
    return false;
  }

  bool isUseByToken(unsigned Reg) const {
    for (auto AI = AccList.begin(), AE = AccList.end(); AI != AE; ++AI) {
      RegAccess *Acc = *AI;
      if (Acc->getRegNo() == Reg && Acc->isUseByToken())
        return true;
    }
    return false;
  }

  bool isDefNotByToken(unsigned Reg) const {
    for (auto AI = AccList.begin(), AE = AccList.end(); AI != AE; ++AI) {
      RegAccess *Acc = *AI;
      if (Acc->getRegNo() == Reg && !Acc->isDefNotByToken())
        return false;
    }
    return true;
  }

  bool isUseNotByToken(unsigned Reg) const {
    for (auto AI = AccList.begin(), AE = AccList.end(); AI != AE; ++AI) {
      RegAccess *Acc = *AI;
      if (Acc->getRegNo() == Reg && !Acc->isUseNotByToken())
        return false;
    }
    return true;
  }
};

class DepEdge {
  DepNode *Head;
  DepNode *Tail;
  bool HeadDef;

public:
  DepEdge(DepNode *H, DepNode *T, bool FromDef)
      : Head(H), Tail(T), HeadDef(FromDef) {}

  DepNode *getHead() const { return Head; }
  DepNode *getTail() const { return Tail; }

  bool isHeadDef() const { return HeadDef; }
};


/// Binary is a sequence of byte containing the machine code specified by @p
/// Data and @p Size. It has 0 or more reference to symbol and 0 or more
/// relocations.
///
class Binary {
public:
  typedef std::list<Relocation> RelList;
  typedef std::list<RegAccess>  RegAccList;
  typedef std::list<Token>      TokList;

  struct DepNodeCompare {
    bool operator()(DepNode *A, DepNode *B) {
      return A->getOffset() < B->getOffset();
    }
  };
  typedef std::list<DepNode *> SyncPointList;

private:
  const char *Data;     ///< The buffer containing the binary.
  std::size_t Size;     ///< The size of that binary.
  unsigned    LinkType; ///< The type of linkage, i.e., NONE, CALLER, or CALLEE.

  RelList     Rels;
  RegAccList  InitRegAcc;
  RegAccList  FiniRegAcc;
  TokList     Toks;
  SyncPointList SyncPoints;

  unsigned Order;
  unsigned Position;

  const Symbol *Name;

public:
  Binary(const char *B, std::size_t S) :
    Data(B), Size(S), LinkType(0), Order(0), Position(unsigned(-1)), Name(nullptr) {}

  RelList::const_iterator rel_begin() const { return Rels.begin(); }
  RelList::const_iterator rel_end()   const { return Rels.end(); }

  RelList::iterator rel_begin() { return Rels.begin(); }
  RelList::iterator rel_end()   { return Rels.end(); }

  Relocation *addReloc(unsigned Off, Symbol *S) {
    Rels.push_back(Relocation(Off, S));
    return &Rels.back();
  }


  RegAccList::const_iterator initreg_begin() const {
    return InitRegAcc.begin();
  }
  RegAccList::const_iterator initreg_end() const {
    return InitRegAcc.end();
  }

  RegAccList::iterator initreg_begin() { return InitRegAcc.begin(); }
  RegAccList::iterator initreg_end()   { return InitRegAcc.end(); }

  RegAccList::const_iterator finireg_begin() const {
    return FiniRegAcc.begin();
  }
  RegAccList::const_iterator finireg_end() const {
    return FiniRegAcc.end();
  }

  RegAccList::iterator finireg_begin() { return FiniRegAcc.begin(); }
  RegAccList::iterator finireg_end()   { return FiniRegAcc.end(); }

  RegAccess *addInitRegAccess(unsigned Off, unsigned RegNo, unsigned DUT) {
    InitRegAcc.push_back(RegAccess(Off, RegNo, DUT));
    return &InitRegAcc.back();
  }

  RegAccess *addFiniRegAccess(unsigned Off, unsigned RegNo, unsigned DUT) {
    FiniRegAcc.push_back(RegAccess(Off, RegNo, DUT));
    return &FiniRegAcc.back();
  }

  TokList::const_iterator tok_begin() const { return Toks.begin(); }
  TokList::const_iterator tok_end()   const { return Toks.end(); }

  TokList::iterator tok_begin() { return Toks.begin(); }
  TokList::iterator tok_end()   { return Toks.end(); }

  Token *addToken(unsigned T) {
    Toks.push_back(Token(T));
    return &Toks.back();
  }

  void clearSyncPoints() { SyncPoints.clear(); }
  void insertSyncPoint(DepNode *N) { SyncPoints.push_back(N); }
  void sortSyncPoints() { SyncPoints.sort(DepNodeCompare()); }

  SyncPointList::const_iterator sp_begin() const { return SyncPoints.begin(); }
  SyncPointList::const_iterator sp_end()   const { return SyncPoints.end(); }
  SyncPointList::iterator sp_begin() { return SyncPoints.begin(); }
  SyncPointList::iterator sp_end()   { return SyncPoints.end(); }


  const char *getData() const { return Data; }
  void setData(const char *Buf) { Data = Buf; }

  const std::size_t getSize() const { return Size; }
  void setSize(std::size_t S) { Size = S; }

  const unsigned getLinkType() const { return LinkType; }
  void setLinkType(unsigned LT) { LinkType = LT; }

  const unsigned getOrder() const { return Order; }
  void setOrder(unsigned O) { Order = O; }

  const unsigned getPos() const { return Order; }
  void setPos(unsigned O) { Order = O; }

  const Symbol *getName() const { return Name; }
  void setName(const Symbol *S) { Name = S; }
};

/// Collection
class Collection {
public:
  typedef std::list<Binary> BinaryList;
  typedef std::list<Symbol> SymbolList;

  struct cstring_less {
    bool operator()(const char *s0, const char *s1) const {
      return std::strcmp(s0, s1) < 0;
    }
  };

private:
  BinaryList Binaries;
  SymbolList Symbols;

  unsigned Platform;
  unsigned UniqueID;

  std::list<std::string> NewNames;

  std::map<const char *, Symbol *, cstring_less> SymbolMap;

  std::string Linked;

public:
  Collection() : Platform(PP_NONE), UniqueID(0) {}

  unsigned getPlatform() const { return Platform; }
  void setPlatform(unsigned P) { Platform = P; }

  BinaryList::const_iterator bin_begin() const { return Binaries.begin(); }
  BinaryList::const_iterator bin_end()   const { return Binaries.end(); }

  BinaryList::iterator bin_begin() { return Binaries.begin(); }
  BinaryList::iterator bin_end()   { return Binaries.end(); }

  SymbolList::const_iterator sym_begin() const { return Symbols.begin(); }
  SymbolList::const_iterator sym_end()   const { return Symbols.end(); }

  SymbolList::iterator sym_begin() { return Symbols.begin(); }
  SymbolList::iterator sym_end()   { return Symbols.end(); }

  Binary *addBinary(const char *B, std::size_t S) {
    Binaries.push_back(Binary(B, S));
    return &Binaries.back();
  }

  Symbol *addSymbol(const char *Name) {
    auto I = SymbolMap.find(Name);
    if (I != SymbolMap.end())
      return I->second;
    Symbols.push_back(Symbol(Name, 0, nullptr, 0));
    Symbol *S = &Symbols.back();
    SymbolMap.insert(std::make_pair(Name, S));
    return S;
  }

  Symbol *getSymbol(const char *Name) {
    auto I = SymbolMap.find(Name);
    if (I != SymbolMap.end())
      return I->second;
    return nullptr;
  }

  const char *getUniqueName(const char *Name) {
    std::string UniqueName(Name);
    UniqueName += "!";
    UniqueName += std::to_string(UniqueID++);
    NewNames.push_back(std::move(UniqueName));
    return NewNames.back().c_str();
  }

  void setLinkedBinary(std::string &&L) { Linked = L; }
  const std::string &getLinkedBinary() const { return Linked; }
};

} // End namespace patch
} // End namespace cm

#endif // __CM_FC_PATCHINFO_RECORD_H__
