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
// CM Fast Composite Linking library.
//

#include <cstring>

#include "cm_fc_ld.h"

#include "PatchInfoLinker.h"
#include "PatchInfoReader.h"
#include "PatchInfoRecord.h"


int cm_fc_get_callee_info(const char *buf, size_t size,
                          void *c,
                          cm_fc_link_type *out_link_type) {
  // Read patch info.
  if (readPatchInfo(buf, size, *((cm::patch::Collection *)c)))
    return CM_FC_FAILURE;

  cm::patch::Collection *C = reinterpret_cast<cm::patch::Collection *>(c);

  // Bail out if there's no symbol.
  if (C->sym_begin() == C->sym_end())
    return CM_FC_FAILURE;
  // Bail out if there's no binary.
  if (C->bin_begin() == C->bin_end())
    return CM_FC_FAILURE;

  cm::patch::Binary &Bin = *(C->bin_begin());

  cm::patch::Symbol *S = nullptr;
  for (auto I = C->sym_begin(), E = C->sym_end(); I != E; ++I)
    if (I->getBinary() == &Bin) {
      S = &*I;
    }
  if (!S)
    return CM_FC_FAILURE;

  unsigned LT = S->getExtra() & 0x3;
  if (LT >= 3)
    return CM_FC_FAILURE;
  *out_link_type = cm_fc_link_type(LT);

  if (Bin.rel_begin() == Bin.rel_end()) {
    // If there is no relocation, there's no callee info to return.
    return CM_FC_FAILURE;
  }

  return CM_FC_OK;
}

int cm_fc_combine_kernels(size_t num_kernels, cm_fc_kernel_t kernels[],
                          char *out_buf, size_t *out_size,
                          const char *options) {
  if (!out_buf || !out_size)
    return CM_FC_FAILURE;

  cm::patch::Collection C;
  if (linkPatchInfo(C, num_kernels, kernels, options))
    return CM_FC_FAILURE;

  const std::string &B = C.getLinkedBinary();
  if (B.size() > *out_size) {
    *out_size = B.size();
    return CM_FC_NOBUFS;
  }

  std::memcpy(out_buf, B.data(), B.size());
  *out_size = B.size();

  return CM_FC_OK;
}
