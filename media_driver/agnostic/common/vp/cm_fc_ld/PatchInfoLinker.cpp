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

// PatchInfo linker.
//

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "cm_fc_ld.h"

#include "DepGraph.h"
#ifdef __WITH_IGA__
#include "IGAAutoDep.h"
#endif
#include "PatchInfoLinker.h"
#include "PatchInfoReader.h"

#ifndef __WITH_IGA__
namespace cm {
namespace toolchain {

// Dummy hook for policy 2.
std::pair<int, std::string>
resolvDep(unsigned P, const char *Bin, std::size_t Sz) {
  std::string Buf(Bin, Sz);
  return std::make_pair(0, Buf);
}

} // End namespace toolchain
} // End namespace cm
#endif

namespace {

class PatchInfoLinker {
  std::size_t NumKernels;
  cm_fc_kernel_t *Kernels;

  const char *Options;

  std::string Linked;

  unsigned Policy;
  unsigned r127Token;
  bool hasR127Token;

  unsigned Platform;

public:
  PatchInfoLinker(std::size_t NK, cm_fc_kernel_t *K, const char *O = nullptr)
    : NumKernels(NK), Kernels(K), Options(O),
      Policy(cm::patch::DepGraph::SWSB_POLICY_1),
      r127Token(-1),
      hasR127Token(false),
      Platform(0) {
    parseOptions();
  }

  bool link(cm::patch::Collection &C);

protected:
  unsigned align(unsigned);
  unsigned writeNOP(unsigned);
  unsigned writeEOT();

  void parseOptions() {
    std::string Opt;
    if (Options)
      Opt = Options;

    if (Opt.empty())
      return;

    std::string::size_type pos = 0;
    do {
      pos = Opt.find_first_not_of(':', pos);
      if (pos == std::string::npos)
        break;
      switch (Opt[pos]) {
      default:
        break;
      case 'p':
        ++pos;
        if (pos >= Opt.size())
          break;
        switch (Opt[pos]) {
        default:
          break;
        case '0':
          Policy = cm::patch::DepGraph::SWSB_POLICY_0;
          ++pos;
          break;
        case '1':
          Policy = cm::patch::DepGraph::SWSB_POLICY_1;
          ++pos;
          break;
        case '2':
          Policy = cm::patch::DepGraph::SWSB_POLICY_2;
          ++pos;
          break;
        }
      }
      if (pos >= Opt.size())
        break;
      pos = Opt.find_first_of(':', pos);
      if (pos == std::string::npos)
        break;
      ++pos;
    } while (pos < Opt.size());
  }

  unsigned writeSync(unsigned RdMask, unsigned WrMask);
};

} // End anonymous namespace

bool linkPatchInfo(cm::patch::Collection &C,
                   std::size_t NumKernels, cm_fc_kernel_t *Kernels,
                   const char *Options) {
  PatchInfoLinker LD(NumKernels, Kernels, Options);
  return LD.link(C);
}

bool PatchInfoLinker::link(cm::patch::Collection &C) {
  for (unsigned i = 0, e = unsigned(NumKernels); i != e; ++i)
    if (readPatchInfo(Kernels[i].patch_buf, Kernels[i].patch_size, C))
      return true;

  Platform = C.getPlatform();

  std::map<cm::patch::Binary *, cm::patch::Symbol *> BinMap;
  // Setup mapping from binary to symbol.
  for (auto I = C.sym_begin(), E = C.sym_end(); I != E; ++I) {
    // Bail out if there's unresolved symbol.
    if (I->isUnresolved())
      return true;
    if (I->getAddr() == 0) {
      BinMap[I->getBinary()] = &*I;
      I->getBinary()->setName(&*I);
    }
  }

  // Associate separate binaries and find the last top-level kernel.
  cm::patch::Binary *LastTopBin = nullptr;
  unsigned n = 0;
  for (auto I = C.bin_begin(), E = C.bin_end(); I != E; ++I) {
    auto &B = *I;
    if (n >= NumKernels)
      return true;
    B.setData(Kernels[n].binary_buf);
    B.setSize(Kernels[n].binary_size);
    B.clearSyncPoints();
    ++n;
    // Check link type through its symbol.
    auto M = BinMap.find(&B);
    if (M == BinMap.end()) // Bail out if there's binary without symbol name.
      return true;
    auto S = M->second;
    B.setLinkType(S->getExtra() & 0x3);
    if (B.getLinkType() != CM_FC_LINK_TYPE_CALLEE)
    {
        LastTopBin = &B;
        for (auto RI = LastTopBin->finireg_begin(),
            RE = LastTopBin->finireg_end(); RI != RE; ++RI) {
            // Skip use-only access without token associated.
            if (RI->isDefNotByToken())
                continue;
            unsigned Reg = RI->getRegNo();
            if (Reg == 127)
            {
                std::tie(hasR127Token, r127Token) = RI->getToken();
            }
        }
    }
  }
  // Bail out if there is mismatch.
  if (n != NumKernels)
    return true;

  cm::patch::DepGraph DG(C, Policy);
  DG.build();
  DG.resolve();

  // Link all kernels into 'linked' buffer.
  Linked.clear();
  for (auto I = C.bin_begin(), E = C.bin_end(); I != E; ++I) {
    auto Bin = &*I;
    align(4); // Align to 16B, i.e. 1 << 4.
    // Real binary starts from here.
    Bin->setPos(unsigned(Linked.size()));
    Bin->sortSyncPoints();
    unsigned Start = 0;
    unsigned Inserted = 0;
    for (auto SI = Bin->sp_begin(), SE = Bin->sp_end(); SI != SE; ++SI) {
      auto Node = *SI;
      unsigned Offset = Node->getOffset();
      assert(Start <= Offset && "Invalid insert point!");
      if (Start < Offset) {
        Linked.append(Bin->getData() + Start, Offset - Start);
        // Adjust relocation in this range.
        for (auto RI = Bin->rel_begin(), RE = Bin->rel_end(); RI != RE; ++RI) {
          unsigned RelOff = RI->getOffset();
          if (Start <= RelOff && RelOff < Offset)
            RI->setOffset(RelOff + Inserted);
        }
      }
      Start = Offset;
      Inserted += writeSync(Node->getRdTokenMask(), Node->getWrTokenMask());
    }
    Linked.append(Bin->getData() + Start, Bin->getSize() - Start);
    for (auto RI = Bin->rel_begin(), RE = Bin->rel_end(); RI != RE; ++RI) {
      unsigned RelOff = RI->getOffset();
      if (Start <= RelOff && RelOff < Bin->getSize())
        RI->setOffset(RelOff + Inserted);
    }
    if (Bin == LastTopBin)
      writeEOT();
  }
  writeNOP(64);

  // Fix relocations.
  for (auto I = C.bin_begin(), E = C.bin_end(); I != E; ++I) {
    auto Bin = &*I;
    unsigned Offset = Bin->getPos();
    // Bail out if this binary is not in the position map.
    if (Offset == unsigned(-1))
      return true;
    for (auto RI = Bin->rel_begin(), RE = Bin->rel_end(); RI != RE; ++RI) {
      auto S = RI->getSymbol();
      auto Target = S->getBinary();
      unsigned TargetOffset = Target->getPos();
      // Bail out if this binary is not in the position map.
      if (TargetOffset == unsigned(-1))
        return true;
      unsigned AbsIP = Offset + RI->getOffset();
      unsigned AbsJIP = TargetOffset + S->getAddr();
      int Imm = AbsJIP - AbsIP;
      uint32_t *p = reinterpret_cast<uint32_t *>(&Linked[AbsIP]);
      p[3] = Imm;
      ++n;
    }
  }

  if (Policy == cm::patch::DepGraph::SWSB_POLICY_2) {
    std::string Out;
    int Ret;
    std::tie(Ret, Out) =
      cm::toolchain::resolvDep(Platform, Linked.data(), Linked.size());
    if (Ret < 0)
      return true;
    Linked.swap(Out);
  }

  C.setLinkedBinary(std::move(Linked));

  return false;
}

unsigned PatchInfoLinker::align(unsigned Align) {
  unsigned A = (1U << Align);
  unsigned Origin = unsigned(Linked.size());
  unsigned Padding = ((Origin + A - 1) / A) * A;
  return writeNOP(Padding - Origin);
}

/// Append Linked with 'nop' taking up to N bytes.
unsigned PatchInfoLinker::writeNOP(unsigned N) {
  // Bail out if N is not aligned with 8B, i.e. 64 bits. That's the minimal
  // size of 'nop' instruction.
  if (N % 8 != 0)
    return 0;
  uint64_t regular_nop = 0;
  uint64_t compact_nop = 0;
  switch (Platform) {
  case cm::patch::PP_TGL:
  case cm::patch::PP_TGLLP:
    regular_nop = 0x00000060U;
    compact_nop = 0x20000060U;
    break;
  default:
    regular_nop = 0x0000007eU;
    compact_nop = 0x2000007eU;
    break;
  }
  unsigned B = 0;
  while (N > 8) {
    Linked.append(reinterpret_cast<char *>(&regular_nop), sizeof(regular_nop));
    Linked.append(sizeof(uint64_t), 0);
    N -= 16;
    B += 16;
  }
  while (N > 0) {
    Linked.append(reinterpret_cast<char *>(&compact_nop), sizeof(compact_nop));
    N -= 8;
    B += 8;
  }
  return B;
}

unsigned PatchInfoLinker::writeEOT() {
  uint64_t mov0 = 0;
  uint64_t mov1 = 0;
  uint64_t snd0 = 0;
  uint64_t snd1 = 0;
  uint64_t r127_sync0 = 0;
  uint64_t r127_sync1 = 0;

  switch (Platform) {
  case cm::patch::PP_TGL:
  case cm::patch::PP_TGLLP:
  {
      if (hasR127Token)
      {
          uint8_t  *sync0Ptr = (uint8_t *)&r127_sync0;
          r127_sync0 = 0x0001000000002001ULL;
          r127_sync1 = 0x0000000000000000ULL;
          sync0Ptr[1] |= (uint8_t)r127Token;
      }
      mov0 = 0x7f050aa080030161ULL;
      mov1 = 0x0000000000460005ULL;
      snd0 = 0x0000000400000131ULL;
      snd1 = 0x0000000070207f0cULL;
  }
    break;
  default:
    mov0 = 0x2fe0020c00600001ULL;
    mov1 = 0x00000000008d0000ULL;
    snd0 = 0x2000020007000031ULL;
    snd1 = 0x8200001006000fe0ULL;
    break;
  }
  unsigned B = 0;
  if (hasR127Token)
  {
      Linked.append(reinterpret_cast<char *>(&r127_sync0), sizeof(r127_sync0));
      B += sizeof(r127_sync0);
      Linked.append(reinterpret_cast<char *>(&r127_sync1), sizeof(r127_sync1));
      B += sizeof(r127_sync1);
  }
  Linked.append(reinterpret_cast<char *>(&mov0), sizeof(mov0));
  B += sizeof(mov0);
  Linked.append(reinterpret_cast<char *>(&mov1), sizeof(mov1));
  B += sizeof(mov1);
  Linked.append(reinterpret_cast<char *>(&snd0), sizeof(snd0));
  B += sizeof(snd0);
  Linked.append(reinterpret_cast<char *>(&snd1), sizeof(snd1));
  B += sizeof(snd1);

  return B;
}


unsigned PatchInfoLinker::writeSync(unsigned RdMask, unsigned WrMask) {
  uint64_t sysrd0 = 0x0001000000000101;
  uint64_t sysrd1 = 0x0000000020000000;
  uint64_t syswr0 = 0x0001000000000001;
  uint64_t syswr1 = 0x0000000030000000;

  // Force 1 distance to sync in-order instruction.
  uint64_t swsb_mask = 0x000000000000FF00;

  // Subfuncs on qword1.
  uint64_t fc_mask = 0x00000000F0000000;
  uint64_t   nop   = 0x0000000000000000;
  uint64_t allrd   = 0x0000000020000000;
  uint64_t allwr   = 0x0000000030000000;

  uint64_t dist = 1;
  unsigned B = 0;

  if (RdMask == unsigned(-1)) {
    uint64_t qw0 = (sysrd0 & ~swsb_mask) | (dist << 8);
    uint64_t qw1 = (sysrd1 & ~fc_mask) | allrd;
    Linked.append(reinterpret_cast<char *>(&qw0), sizeof(qw0));
    B += sizeof(qw0);
    Linked.append(reinterpret_cast<char *>(&qw1), sizeof(qw1));
    B += sizeof(qw1);
    // Clear distance.
    dist = 0;
  } else {
    for (unsigned Tok = 0; RdMask != 0; RdMask >>= 1, ++Tok) {
      if (RdMask & 1) {
        uint64_t swsb = 0x30 | Tok;
        uint64_t qw0 = (sysrd0 & ~swsb_mask) | (swsb << 8);
        uint64_t qw1 = (sysrd1 & ~fc_mask) | nop;
        Linked.append(reinterpret_cast<char *>(&qw0), sizeof(qw0));
        B += sizeof(qw0);
        Linked.append(reinterpret_cast<char *>(&qw1), sizeof(qw1));
        B += sizeof(qw1);
      }
    }
  }
  if (WrMask == unsigned(-1)) {
    uint64_t qw0 = (sysrd0 & ~swsb_mask) | (dist << 8);
    uint64_t qw1 = (sysrd1 & ~fc_mask) | allwr;
    Linked.append(reinterpret_cast<char *>(&qw0), sizeof(qw0));
    B += sizeof(qw0);
    Linked.append(reinterpret_cast<char *>(&qw1), sizeof(qw1));
    B += sizeof(qw1);
    // Clear distance.
    dist = 0;
  } else {
    for (unsigned Tok = 0; WrMask != 0; WrMask >>= 1, ++Tok) {
      if (WrMask & 1) {
        uint64_t swsb = 0x80 | Tok | (dist << 4);
        uint64_t qw0 = (sysrd0 & ~swsb_mask) | (swsb << 8);
        uint64_t qw1 = (sysrd1 & ~fc_mask) | nop;
        Linked.append(reinterpret_cast<char *>(&qw0), sizeof(qw0));
        B += sizeof(qw0);
        Linked.append(reinterpret_cast<char *>(&qw1), sizeof(qw1));
        B += sizeof(qw1);
        // Clear distance.
        dist = 0;
      }
    }
  }

  return B;
}
