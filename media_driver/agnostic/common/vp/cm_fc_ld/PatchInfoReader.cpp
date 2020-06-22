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
// PatchInfo reader.
//

#include <cassert>
#include <cstddef>
#include <map>

#include "PatchInfo.h"
#include "PatchInfoRecord.h"
#include "PatchInfoReader.h"

namespace {

class PatchInfoReader {
  const char *Data;
  std::size_t Size;
  unsigned ShEntries;

  const cm::patch::PInfoSectionHdr *Sh;  

  // All symbol tables are merged into a single one.
  std::map<unsigned, cm::patch::Symbol *> SymbolTable;

  typedef std::map<unsigned, cm::patch::Binary *> BinarySectionMapTy;
  typedef std::map<unsigned, bool> SymbolTableSectionMapTy;
  BinarySectionMapTy BinarySectionMap;
  SymbolTableSectionMapTy SymbolTableSectionMap;

public:
  PatchInfoReader(const char *B, std::size_t S) : Data(B), Size(S), ShEntries(0){Sh = nullptr;}

  bool read(cm::patch::Collection &C);

protected:
  bool readHeader(cm::patch::Collection &C);
  bool readSections(cm::patch::Collection &C);

  bool readDummySection(cm::patch::Collection &C, unsigned n);
  bool readUnknownSection(cm::patch::Collection &C, unsigned n);
  bool readBinarySection(cm::patch::Collection &C, unsigned n);
  bool readRelocationSection(cm::patch::Collection &C, unsigned n);
  bool readSymbolTableSection(cm::patch::Collection &C, unsigned n);
  bool readStringTableSection(cm::patch::Collection &C, unsigned n);
  bool readInitRegAccessTableSection(cm::patch::Collection &C, unsigned n);
  bool readFiniRegAccessTableSection(cm::patch::Collection &C, unsigned n);
  bool readTokenTableSection(cm::patch::Collection &C, unsigned n);

  bool readRegisterAccessTableSection(cm::patch::Collection &C, unsigned n,
                                      cm::patch::PInfo_U16 ShType);

  bool isValidSection(unsigned n) {
    if (n >= ShEntries)
      return false;
    if (!Sh ||
        Sh[n].ShOffset >= Size ||
        Sh[n].ShOffset + Sh[n].ShSize > Size)
      return false;
    return true;
  }
  bool isValidSectionOfType(unsigned n, cm::patch::PInfo_U16 ShType) {
    if (!isValidSection(n))
      return false;
    return Sh[n].ShType == ShType;
  }

  const char *getString(unsigned ShIdx, unsigned Idx) {
    const char *StringTable = Data + Sh[ShIdx].ShOffset;
    return StringTable + Idx;
  }

  std::pair<BinarySectionMapTy::iterator, bool>
      getOrReadBinarySection(cm::patch::Collection &C, unsigned n);

  std::pair<SymbolTableSectionMapTy::iterator, bool>
      getOrReadSymbolTableSection(cm::patch::Collection &C, unsigned n);
};

} // End anonymous namespace

bool readPatchInfo(const char *Buf, std::size_t Sz, cm::patch::Collection &C) {
  PatchInfoReader R(Buf, Sz);
  return R.read(C);
}

bool PatchInfoReader::read(cm::patch::Collection &C) {
  return readHeader(C) || readSections(C);
}

bool PatchInfoReader::readHeader(cm::patch::Collection &C) {
  if (Size < sizeof(cm::patch::PInfoHdr))
    return true;

  const cm::patch::PInfoHdr *H =
    reinterpret_cast<const cm::patch::PInfoHdr *>(Data);
  if (!H->checkMagic())
    return true;
  if (H->Version != cm::patch::PV_0)
    return true;
  if (!H->isValidPlatform())
    return true;
  if (H->ShOffset >= Size)
    return true;
  if (H->ShOffset + H->ShNum * sizeof(cm::patch::PInfoSectionHdr) > Size)
    return true;

  if (C.getPlatform() != cm::patch::PP_NONE && C.getPlatform() != H->Platform)
    return true;

  C.setPlatform(H->Platform);

  Sh = reinterpret_cast<const cm::patch::PInfoSectionHdr *>(Data + H->ShOffset);
  ShEntries = H->ShNum;

  return false;
}

std::pair<PatchInfoReader::BinarySectionMapTy::iterator, bool>
PatchInfoReader::getOrReadBinarySection(cm::patch::Collection &C, unsigned n) {
  auto BI = BinarySectionMap.end();
  if (!readBinarySection(C, n)) {
    BI = BinarySectionMap.find(n);
    assert(BI != BinarySectionMap.end());
  }
  return std::make_pair(BI, BI == BinarySectionMap.end());
}

std::pair<PatchInfoReader::SymbolTableSectionMapTy::iterator, bool>
PatchInfoReader::getOrReadSymbolTableSection(cm::patch::Collection &C,
                                             unsigned n) {
  auto SI = SymbolTableSectionMap.end();
  if (!readSymbolTableSection(C, n)) {
    SI = SymbolTableSectionMap.find(n);
    assert(SI != SymbolTableSectionMap.end());
  }
  return std::make_pair(SI, SI == SymbolTableSectionMap.end());
}

bool PatchInfoReader::readSections(cm::patch::Collection &C) {
  if (!Sh)
    return true;

  for (unsigned n = 0; n != ShEntries; ++n) {

    switch (Sh[n].ShType) {
    case cm::patch::PSHT_NONE:
      if (readDummySection(C, n)) return true;
      break;
    case cm::patch::PSHT_BINARY:
      if (readBinarySection(C, n)) return true;
      break;
    case cm::patch::PSHT_REL:
      if (readRelocationSection(C, n)) return true;
      break;
    case cm::patch::PSHT_SYMTAB:
      if (readSymbolTableSection(C, n)) return true;
      break;
    case cm::patch::PSHT_STRTAB:
      if (readStringTableSection(C, n)) return true;
      break;
    case cm::patch::PSHT_INITREGTAB:
      if (readInitRegAccessTableSection(C, n)) return true;
      break;
    case cm::patch::PSHT_FINIREGTAB:
      if (readFiniRegAccessTableSection(C, n)) return true;
      break;
    case cm::patch::PSHT_TOKTAB:
      if (readTokenTableSection(C, n)) return true;
      break;
    default:
      if (readUnknownSection(C, n)) return true;
      break;
    }
  }

  return false;
}

bool PatchInfoReader::readDummySection(cm::patch::Collection &C, unsigned n) {
  if (!isValidSectionOfType(n, cm::patch::PSHT_NONE)) return true;
  return false;
}

bool PatchInfoReader::readBinarySection(cm::patch::Collection &C, unsigned n) {
  // Skip if this binary section is ready read.
  if (BinarySectionMap.count(n))
    return false;

  // Bail out if it's not an valid binary section.
  if (!isValidSectionOfType(n, cm::patch::PSHT_BINARY))
    return true;

  const char *Buf = nullptr;
  std::size_t Sz = Sh[n].ShSize;
  if (Sz)
    Buf = Data + Sh[n].ShOffset;
  cm::patch::Binary *Bin = C.addBinary(Buf, Sz);
  BinarySectionMap.insert(std::make_pair(n, Bin));

  return false;
}

bool PatchInfoReader::readRelocationSection(cm::patch::Collection &C,
                                            unsigned n) {
  // Skip if this relocation section is already read.
  if (!isValidSectionOfType(n, cm::patch::PSHT_REL))
    return true;

  bool Ret;

  BinarySectionMapTy::iterator BI;
  std::tie(BI, Ret) = getOrReadBinarySection(C, Sh[n].ShLink2);
  if (Ret)
    return true;
  cm::patch::Binary *Bin = BI->second;

  SymbolTableSectionMapTy::iterator SI;
  std::tie(SI, Ret) = getOrReadSymbolTableSection(C, Sh[n].ShLink);
  if (Ret)
    return true;

  // Scan through relocations.
  std::size_t Sz = Sh[n].ShSize;
  const cm::patch::PInfoRelocation *Rel =
    reinterpret_cast<const cm::patch::PInfoRelocation *>(Data + Sh[n].ShOffset);
  for (unsigned i = 0; Sz > 0; ++i, Sz -= sizeof(cm::patch::PInfoRelocation)) {
    auto I = SymbolTable.find(Rel[i].RelSym);
    if (I == SymbolTable.end())
      return true;
    cm::patch::Symbol *S = I->second;
    Bin->addReloc(Rel[i].RelAddr, S);
  }

  return false;
}

bool PatchInfoReader::readSymbolTableSection(cm::patch::Collection &C,
                                            unsigned n) {
  // Skip if this section is ready read.
  if (SymbolTableSectionMap.count(n))
    return false;

  // Bail out if it's an invalid section.
  if (!isValidSectionOfType(n, cm::patch::PSHT_SYMTAB))
    return true;

  // Read string table.
  unsigned ShIdx = Sh[n].ShLink;
  if (readStringTableSection(C, ShIdx))
    return true;

  // Scan through the symbol table.
  const cm::patch::PInfoSymbol *Sym =
    reinterpret_cast<const cm::patch::PInfoSymbol *>(Data + Sh[n].ShOffset);
  std::size_t Sz = Sh[n].ShSize;
  for (unsigned i = 0; Sz > 0; ++i, Sz -= sizeof(cm::patch::PInfoSymbol)) {
    // Skip unamed symbol.
    unsigned StrIdx = Sym[i].SymName;
    if (!StrIdx)
      continue;
    const char *Name = getString(ShIdx, StrIdx);
    cm::patch::Binary *Bin = nullptr;
    unsigned Ndx = Sym[i].SymShndx;
    if (Ndx) {
      // Only support binary section so far.
      bool Ret;
      BinarySectionMapTy::iterator BI;
      std::tie(BI, Ret) = getOrReadBinarySection(C, Ndx);
      if (Ret)
        return true;
      Bin = BI->second;
    }
    cm::patch::Symbol *S = C.getSymbol(Name);
    if (Bin)
      while (S && !S->isUnresolved()) {
        // In case a symbol has multiple definitions (due to combining a single
        // kernel multiple times), rename the conflicting one.
        Name = C.getUniqueName(Name);
        S = C.getSymbol(Name);
      }
    S = C.addSymbol(Name);
    if (Bin && S->isUnresolved()) {
      S->setBinary(Bin);
      S->setAddr(Sym[i].SymValue);
      S->setExtra(Sym[i].SymExtra);
    }
    // Assume there's just one symbol table section per patch info.
    SymbolTable.insert(std::make_pair(i, S));
  }
  SymbolTableSectionMap.insert(std::make_pair(n, true));

  return false;
}

bool PatchInfoReader::readStringTableSection(cm::patch::Collection &C,
                                             unsigned n) {
  if (!isValidSectionOfType(n, cm::patch::PSHT_STRTAB))
    return true;
  return false;
}


bool
PatchInfoReader::readRegisterAccessTableSection(cm::patch::Collection &C,
                                                unsigned n,
                                                cm::patch::PInfo_U16 ShType) {
  if (!isValidSectionOfType(n, ShType))
    return true;

  BinarySectionMapTy::iterator BI;
  bool Ret;
  std::tie(BI, Ret) = getOrReadBinarySection(C, Sh[n].ShLink2);
  if (Ret)
    return true;
  cm::patch::Binary *Bin = BI->second;

  // Scan through register accesses.
  std::size_t Sz = Sh[n].ShSize;
  const cm::patch::PInfoRegAccess *Acc =
    reinterpret_cast<const cm::patch::PInfoRegAccess *>(Data + Sh[n].ShOffset);
  switch (ShType) {
  default:
    return true;
  case cm::patch::PSHT_INITREGTAB:
    for (unsigned i = 0; Sz > 0; ++i, Sz -= sizeof(cm::patch::PInfoRegAccess))
      Bin->addInitRegAccess(Acc[i].RegAccAddr, Acc[i].RegAccRegNo,
                            Acc[i].RegAccDUT);
    break;
  case cm::patch::PSHT_FINIREGTAB:
    for (unsigned i = 0; Sz > 0; ++i, Sz -= sizeof(cm::patch::PInfoRegAccess))
      Bin->addFiniRegAccess(Acc[i].RegAccAddr, Acc[i].RegAccRegNo,
                            Acc[i].RegAccDUT);
    break;
  }

  return false;
}

bool PatchInfoReader::readInitRegAccessTableSection(cm::patch::Collection &C,
                                                    unsigned n) {

  return readRegisterAccessTableSection(C, n, cm::patch::PSHT_INITREGTAB);
}

bool PatchInfoReader::readFiniRegAccessTableSection(cm::patch::Collection &C,
                                                    unsigned n) {
  return readRegisterAccessTableSection(C, n, cm::patch::PSHT_FINIREGTAB);
}

bool PatchInfoReader::readTokenTableSection(cm::patch::Collection &C,
                                            unsigned n) {
  if (!isValidSectionOfType(n, cm::patch::PSHT_TOKTAB))
    return true;

  BinarySectionMapTy::iterator BI;
  bool Ret;
  std::tie(BI, Ret) = getOrReadBinarySection(C, Sh[n].ShLink2);
  if (Ret)
    return true;
  cm::patch::Binary *Bin = BI->second;

  // Scan through tokens.
  std::size_t Sz = Sh[n].ShSize;
  const cm::patch::PInfoToken *Tok =
    reinterpret_cast<const cm::patch::PInfoToken *>(Data + Sh[n].ShOffset);
  for (unsigned i = 0; Sz > 0; ++i, Sz -= sizeof(cm::patch::PInfoToken))
    Bin->addToken(Tok[i].TokenNo);

  return false;
}


bool PatchInfoReader::readUnknownSection(cm::patch::Collection &C, unsigned n) {
  if (!isValidSection(n))
    return true;
  return false;
}
