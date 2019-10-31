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

#pragma once

#ifndef __CM_FC_DEPGRAPH_H__
#define __CM_FC_DEPGRAPH_H__

#include <list>
#include <map>
#include <tuple>
#include <utility>

#include "PatchInfoRecord.h"

namespace cm {
namespace patch {

class DepGraph {
  Collection &C;

  unsigned Policy;

  std::list<DepNode> Nodes;
  std::list<DepEdge> Edges;

  std::map<std::tuple<Binary *, unsigned, bool>, DepNode *> NodeMap;
  std::map<std::pair<DepNode *, DepNode *>, DepEdge *> EdgeMap;

public:
  enum {
    SWSB_POLICY_0,
    SWSB_POLICY_1,
    SWSB_POLICY_2
  };
  DepGraph(Collection &_C, unsigned P) : C(_C), Policy(P) {};
  void build();
  void resolve();

protected:
  DepNode *getDepNode(Binary *B, unsigned R, bool Barrier);
  DepEdge *getDepEdge(DepNode *From, DepNode *To, bool FromDef);
};

} // End namespace patch
} // End namespace cm

#endif // __CM_FC_DEPGRAPH_H__
