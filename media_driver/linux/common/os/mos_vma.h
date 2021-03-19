/*
* Copyright (c) 2021, Intel Corporation
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
//!
//! \file     mos_vma.h
//! \brief    interface for virtual memory address allocation
//!

#ifndef __MOS_VMA_H__
#define __MOS_VMA_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mos_vma_heap {
   struct list_head holes;

   /** If true, util_vma_heap_alloc will prefer high addresses
    *
    * Default is true.
    */
   bool alloc_high;
} mos_vma_heap;

typedef struct _mos_vma_hole {
   struct list_head link;
   uint64_t offset;
   uint64_t size;
} mos_vma_hole;

void mos_vma_heap_init(mos_vma_heap *heap, uint64_t start, uint64_t size);
void mos_vma_heap_finish(mos_vma_heap *heap);
uint64_t mos_vma_heap_alloc(mos_vma_heap *heap, uint64_t size, uint64_t alignment);
bool mos_vma_heap_alloc_addr(mos_vma_heap *heap, uint64_t addr, uint64_t size);
void mos_vma_heap_free(mos_vma_heap *heap, uint64_t offset, uint64_t size);

#ifdef __cplusplus
} /* extern C */
#endif

#endif
