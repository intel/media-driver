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

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <map>
#include <tuple>

#include "cm_fc_ld.h"

#include "DepGraph.h"

using namespace cm::patch;

DepNode *DepGraph::getDepNode(Binary *B, unsigned Off, bool Barrier = false) {
  auto P = std::make_tuple(B, Off, Barrier);
  auto I = NodeMap.find(P);
  if (I != NodeMap.end())
    return I->second;
  Nodes.push_back(DepNode{B, Off, Barrier});
  DepNode *N = &Nodes.back();
  NodeMap[P] = N;
  return N;
}

DepEdge *DepGraph::getDepEdge(DepNode *From, DepNode *To, bool FromDef) {
  if (From == To) // No dependency on itself.
    return nullptr;
  auto P = std::make_pair(From, To);
  auto I = EdgeMap.find(P);
  if (I != EdgeMap.end())
    return I->second;
  // Add new edge.
  Edges.push_back(DepEdge{From, To, FromDef});
  DepEdge *E = &Edges.back();
  EdgeMap[P] = E;
  From->addToNode(To, FromDef);
  To->addFromNode(From);
  return E;
}

void DepGraph::build() {
  // There's no need to build dependency graph for policy 0 & 2.
  if (Policy == SWSB_POLICY_0 || Policy == SWSB_POLICY_2)
    return;

  std::map<unsigned, DepNode *> State;

  auto requireDefSync = [](std::map<unsigned, DepNode *> &State) {
    for (auto &KV : State)
      if (KV.second->isDefByToken(KV.first))
        return true;
    return false;
  };

  auto requireUseSync = [](std::map<unsigned, DepNode *> &State) {
    for (auto &KV : State)
      if (KV.second->isUseByToken(KV.first))
        return true;
    return false;
  };

  unsigned Order = 0;
  for (auto I = C.bin_begin(), E = C.bin_end(); I != E; ++I) {
    Binary &B = *I;
    if (B.getLinkType() == CM_FC_LINK_TYPE_CALLEE)
      break;
    B.setOrder(++Order);
    if (B.getLinkType() == CM_FC_LINK_TYPE_CALLER) {
      // For caller, add dependency edges to the sync point just before 'call'.
      Relocation &Rel = *B.rel_begin(); // Assume there's just one 'call'.
      Symbol *Sym = Rel.getSymbol();
      Binary &Callee = *Sym->getBinary();
      // Add a barrier node just before 'call' to resolve dependency.
      DepNode *Node = getDepNode(&B, Rel.getOffset(), true);
      for (auto RI = Callee.initreg_begin(),
                RE = Callee.initreg_end(); RI != RE; ++RI) {
        unsigned Reg = RI->getRegNo();
        if (Reg == cm::patch::REG_NONE) {
          /// Check if it's necessary to insert sync points.
          bool reqDefSync = requireDefSync(State);
          bool reqUseSync = requireUseSync(State);
          if (reqDefSync || reqUseSync) {
            Node->clearAccList();
            if (reqDefSync) Node->setRdTokenMask(unsigned(-1));
            if (reqUseSync) Node->setWrTokenMask(unsigned(-1));
            B.insertSyncPoint(Node);
            // Clear state after barrier.
            State.clear();
            break;
          }
        }
        // Only build token-based dependency.
        auto SI = State.find(Reg);
        if (SI == State.end())
          continue;
        auto From = SI->second;
        // Skip access not by token.
        if (!From->isDefByToken(Reg) && !From->isUseByToken(Reg))
          continue;
        // Skip RAR.
        if (From->isUseOnly(Reg) && RI->isUse())
          continue;
        Node->appendRegAcc(&*RI);
        auto E = getDepEdge(From, Node, From->isDef(Reg));
      }
      // Only update token-based dependency.
      Node = getDepNode(&B, Rel.getOffset());
      for (auto RI = Callee.finireg_begin(),
                RE = Callee.finireg_end(); RI != RE; ++RI) {
        // Skip use-only access without token associated.
        if (RI->isDefNotByToken())
          continue;
        unsigned Reg = RI->getRegNo();
        Node->appendRegAcc(&*RI);
        State[Reg] = Node;
      }
      continue;
    }
    // Build RAW, WAW, and WAR from the current state to the first access list.
    for (auto RI = B.initreg_begin(), RE = B.initreg_end(); RI != RE; ++RI) {
      unsigned Reg = RI->getRegNo();
      // Check for sync barrier.
      if (Reg == cm::patch::REG_NONE) {
        /// Check if it's necessary to insert sync points.
        bool reqDefSync = requireDefSync(State);
        bool reqUseSync = requireUseSync(State);
        if (reqDefSync || reqUseSync) {
          DepNode *Node = getDepNode(&B, RI->getOffset(), true);
          if (reqDefSync) Node->setRdTokenMask(unsigned(-1));
          if (reqUseSync) Node->setWrTokenMask(unsigned(-1));
          B.insertSyncPoint(Node);
          // Clean state after barrier.
          State.clear();
          break;
        }
      }
      auto SI = State.find(Reg);
      // Skip if that register has no dependency.
      if (SI == State.end())
        continue;
      auto From = SI->second;
      // Skip if the previous node is a use without token.
      if (From->isUseNotByToken(Reg))
        continue;
      // Skip RAR.
      if (From->isUseOnly(Reg) && RI->isUse())
        continue;
      auto To = getDepNode(&B, RI->getOffset());
      To->appendRegAcc(&*RI);
      auto E = getDepEdge(From, To, From->isDef(Reg));
    }
    // Update the current from the last access list.
    for (auto RI = B.finireg_begin(), RE = B.finireg_end(); RI != RE; ++RI) {
      // Skip use-only access without token associated.
      if (RI->isUseNotByToken())
        continue;
      unsigned Reg = RI->getRegNo();
      auto Node = getDepNode(&B, RI->getOffset());
      Node->appendRegAcc(&*RI);
      State[Reg] = Node;
    }
  }
}

void DepGraph::resolve() {
  // In policy 2, do nothing.
  if (Policy == SWSB_POLICY_2)
    return;

  if (Policy == SWSB_POLICY_0) {
    // In policy 0, insert sync barrier before each non-callee kernels.
    for (auto I = C.bin_begin(), E = C.bin_end(); I != E; ++I) {
      Binary &B = *I;
      if (B.getLinkType() == CM_FC_LINK_TYPE_CALLEE)
        continue;
      DepNode *Node = getDepNode(&B, 0, true);
      Node->setRdTokenMask(unsigned(-1));
      Node->setWrTokenMask(unsigned(-1));
      B.insertSyncPoint(Node);
    }
    return;
  }

  auto getEncodedDistance = [](Binary *Bin, unsigned Off) {
    uint64_t swsb_mask = 0x000000000000FF00;
    uint64_t qw = *((uint64_t *)(Bin->getData() + Off));
    unsigned swsb = unsigned((qw & swsb_mask) >> 8);
    if ((swsb & 0xf0) == 0x00)
      return swsb & 0x7;
    if ((swsb & 0x80) == 0x80)
      return (swsb & 0x70) >> 4;
    return 0U;
  };

  auto calcDistance = [](Collection &C,
                         Binary *From, unsigned FOff,
                         Binary *To, unsigned TOff) {
    // Always assuem there's no compact instruction.
    if (From == To)
      return (TOff - FOff) / 16;
    unsigned D = 0;
    bool FoundFrom = false;
    for (auto BI = C.bin_begin(), BE = C.bin_end(); BI != BE; ++BI) {
      Binary *B = &*BI;
      if (From == B) {
        FoundFrom = true;
        D = B->getSize() - FOff;
        continue;
      }
      if (!FoundFrom)
        continue;
      if (To != B) {
        D += B->getSize();
        continue;
      }
      if (To == B) {
        D += TOff;
        break;
      }
    }
    return D / 16;
  };

  auto countBits = [](unsigned V) {
    unsigned B;
    for (B = 0; V; V >>= 1)
      B += V & 1;
    return B;
  };

  // Fix the dependency. Assume edges are processed in the linking/program
  // order.
  for (auto EI = Edges.begin(), EE = Edges.end(); EI != EE; ++EI) {
    auto H = EI->getHead();
    auto T = EI->getTail();

    if (H->to_empty(EI->isHeadDef()) || T->from_empty())
      continue;

    if (!T->isBarrier())
      T->setDistance(getEncodedDistance(T->getBinary(), T->getOffset()));

    for (auto RI = T->acc_begin(), RE = T->acc_end(); RI != RE; ++RI) {
      RegAccess *To = *RI;
      unsigned Reg = To->getRegNo();
      for (auto DI = T->from_begin(), DE = T->from_end(); DI != DE; ++DI) {
        auto Def = *DI;
        for (auto AI = Def->acc_begin(), AE = Def->acc_end(); AI != AE; ++AI) {
          RegAccess *From = *AI;
          if (From->getRegNo() != Reg)
            continue;
          bool HasToken;
          unsigned Tok;
          std::tie(HasToken, Tok) = From->getToken();
          if (From->isDef()) {
            // RAW or WAW
            if (HasToken)
              T->mergeWrTokenMask(1 << Tok);
            else
              T->updateDistance(calcDistance(C,
                                             Def->getBinary(), Def->getOffset(),
                                             T->getBinary(), T->getOffset()));
          } else if (To->isDef()) {
            // WAR
            if (HasToken)
              T->mergeRdTokenMask(1 << Tok);
          }
        }
      }
    }

    // Revise src token as syncing on def token implies syncing on src token.
    if (T->getRdTokenMask() != unsigned(-1)) {
      unsigned Mask = T->getRdTokenMask();
      Mask &= ~T->getWrTokenMask();
      T->setRdTokenMask(Mask);
    }

    // Update SWSB info and check if we need additional sync barrier.
    if (T->getDistance() > 7) // Clear distance if it's greater than 7.
      T->setDistance(0);
    unsigned NumRdToks = countBits(T->getRdTokenMask());
    unsigned NumWrToks = countBits(T->getWrTokenMask());
    if ((NumRdToks + NumWrToks) > 0) {
      // Already has SBID associated. Need to insert additional sync barrier.
      DepNode *Node = T;
      if (!T->isBarrier())
        Node = getDepNode(T->getBinary(), T->getOffset(), true);
      // Rd tokens.
      if (NumRdToks == 1)
        Node->setRdTokenMask(T->getRdTokenMask());
      else if (NumRdToks != 0)
        Node->setRdTokenMask(unsigned(-1));
      // Wr tokens.
      if (NumWrToks == 1)
        Node->setWrTokenMask(T->getWrTokenMask());
      else if (NumWrToks != 0)
        Node->setWrTokenMask(unsigned(-1));
      T->getBinary()->insertSyncPoint(Node);
    }

    for (auto DI = T->from_begin(), DE = T->from_end(); DI != DE; ++DI)
      (*DI)->clearToNodes(EI->isHeadDef());
    T->clearFromNodes();
  }
}

