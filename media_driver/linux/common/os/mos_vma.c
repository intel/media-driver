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
//! \file     mos_vma.c
//! \brief    interface for virtual memory address allocation
//!

#include "mos_vma.h"

void
mos_vma_heap_init(mos_vma_heap *heap, uint64_t start, uint64_t size)
{
    assert(heap);
    list_inithead(&heap->holes);
    mos_vma_heap_free(heap, start, size);

    /* Default to using high addresses */
    heap->alloc_high = true;
}

void
mos_vma_heap_finish(mos_vma_heap *heap)
{
    assert(heap);
    list_for_each_entry_safe(mos_vma_hole, hole, &heap->holes, link)
    {
        free(hole);
    }
}

#ifdef _DEBUG
static void
mos_vma_heap_validate(mos_vma_heap *heap)
{
    assert(heap);
    uint64_t prev_offset = 0;

    list_for_each_entry(mos_vma_hole, hole, &heap->holes, link)
    {
        assert(hole->offset > 0);
        assert(hole->size > 0);

        if (&hole->link == heap->holes.next)
        {
            /* This must be the top-most hole.  Assert that, if it overflows, it
            * overflows to 0, i.e. 2^64.
            */
            assert(hole->size + hole->offset == 0 ||
                    hole->size + hole->offset > hole->offset);
        }
        else
        {
            /* This is not the top-most hole so it must not overflow and, in
            * fact, must be strictly lower than the top-most hole.  If
            * hole->size + hole->offset == prev_offset, then we failed to join
            * holes during a mos_vma_heap_free.
            */
            assert(hole->size + hole->offset > hole->offset &&
                    hole->size + hole->offset < prev_offset);
        }
        prev_offset = hole->offset;
   }
}
#else
#define mos_vma_heap_validate(heap)
#endif

static void
mos_vma_hole_alloc(mos_vma_hole *hole, uint64_t offset, uint64_t size)
{
    assert(hole);
    assert(hole->offset <= offset);
    assert(hole->size >= offset - hole->offset + size);

    if (offset == hole->offset && size == hole->size) {
        /* Just get rid of the hole. */
        list_del(&hole->link);
        free(hole);
        return;
    }

    uint64_t waste = (hole->size - size) - (offset - hole->offset);
    if (waste == 0) {
        /* We allocated at the top.  Shrink the hole down. */
        hole->size -= size;
        return;
    }

    if (offset == hole->offset) {
        /* We allocated at the bottom. Shrink the hole up. */
        hole->offset += size;
        hole->size -= size;
        return;
    }

    /* We allocated in the middle.  We need to split the old hole into two
    * holes, one high and one low.
    */
    mos_vma_hole *high_hole = (mos_vma_hole*)calloc(1, sizeof(*hole));
    if(high_hole == nullptr)
    {
        assert(high_hole);

        return;
    }

    high_hole->offset = offset + size;
    high_hole->size = waste;

    /* Adjust the hole to be the amount of space left at he bottom of the
    * original hole.
    */
    hole->size = offset - hole->offset;

    /* Place the new hole before the old hole so that the list is in order
    * from high to low.
    */
    list_addtail(&high_hole->link, &hole->link);
}

uint64_t
mos_vma_heap_alloc(mos_vma_heap *heap, uint64_t size, uint64_t alignment)
{
    assert(heap);
    /* The caller is expected to reject zero-size allocations */
    assert(size > 0);
    assert(alignment > 0);

    mos_vma_heap_validate(heap);

    if (heap->alloc_high) {
        list_for_each_entry_safe(mos_vma_hole, hole, &heap->holes, link)
        {
            if (size > hole->size)
                continue;

            /* Compute the offset as the highest address where a chunk of the
            * given size can be without going over the top of the hole.
            *
            * This calculation is known to not overflow because we know that
            * hole->size + hole->offset can only overflow to 0 and size > 0.
            */
            uint64_t offset = (hole->size - size) + hole->offset;

            /* Align the offset.  We align down and not up because we are
            * allocating from the top of the hole and not the bottom.
            */
            offset = (offset / alignment) * alignment;

            if (offset < hole->offset)
            continue;

            mos_vma_hole_alloc(hole, offset, size);
            mos_vma_heap_validate(heap);
            return offset;
        }
    } else {
        list_for_each_entry_safe_rev(mos_vma_hole, hole, &heap->holes, link) {
            if (size > hole->size)
                continue;

            uint64_t offset = hole->offset;

            /* Align the offset */
            uint64_t misalign = offset % alignment;
            if (misalign) {
                uint64_t pad = alignment - misalign;
                if (pad > hole->size - size)
                    continue;

                offset += pad;
            }

            mos_vma_hole_alloc(hole, offset, size);
            mos_vma_heap_validate(heap);
            return offset;
        }
    }

    /* Failed to allocate */
    return 0;
}

bool
mos_vma_heap_alloc_addr(mos_vma_heap *heap, uint64_t offset, uint64_t size)
{
    assert(heap);
    /* An offset of 0 is reserved for allocation failure.  It is not a valid
    * address and cannot be allocated.
    */
    assert(offset > 0);

    /* Allocating something with a size of 0 is also not valid. */
    assert(size > 0);

    /* It's possible for offset + size to wrap around if we touch the top of
    * the 64-bit address space, but we cannot go any higher than 2^64.
    */
    assert(offset + size == 0 || offset + size > offset);

    /* Find the hole if one exists. */
    list_for_each_entry_safe(mos_vma_hole, hole, &heap->holes, link)
    {
        if (hole->offset > offset)
            continue;

        /* Holes are ordered high-to-low so the first hole we find with
        * hole->offset <= is our hole.  If it's not big enough to contain the
        * requested range, then the allocation fails.
        */
        assert(hole->offset <= offset);
        if (hole->size < offset - hole->offset + size)
            return false;

        mos_vma_hole_alloc(hole, offset, size);
        return true;
    }

    /* We didn't find a suitable hole */
    return false;
}

void
mos_vma_heap_free(mos_vma_heap *heap, uint64_t offset, uint64_t size)
{
    assert(heap);
    /* An offset of 0 is reserved for allocation failure.  It is not a valid
    * address and cannot be freed.
    */
    assert(offset > 0);

    /* Freeing something with a size of 0 is also not valid. */
    assert(size > 0);

    /* It's possible for offset + size to wrap around if we touch the top of
    * the 64-bit address space, but we cannot go any higher than 2^64.
    */
    assert(offset + size == 0 || offset + size > offset);

    mos_vma_heap_validate(heap);

    /* Find immediately higher and lower holes if they exist. */
    mos_vma_hole *high_hole = NULL, *low_hole = NULL;
    list_for_each_entry(mos_vma_hole, hole, &heap->holes, link)
    {
        if (hole->offset <= offset) {
            low_hole = hole;
            break;
        }
        high_hole = hole;
    }

    if (high_hole)
    {
        assert(offset + size <= high_hole->offset);
    }

    bool high_adjacent = high_hole && offset + size == high_hole->offset;

    if (low_hole) {
        assert(low_hole->offset + low_hole->size > low_hole->offset);
        assert(low_hole->offset + low_hole->size <= offset);
    }
    bool low_adjacent = low_hole && low_hole->offset + low_hole->size == offset;

    if (low_adjacent && high_adjacent) {
        /* Merge the two holes */
        low_hole->size += size + high_hole->size;
        list_del(&high_hole->link);
        free(high_hole);
    } else if (low_adjacent) {
        /* Merge into the low hole */
        low_hole->size += size;
    } else if (high_adjacent) {
        /* Merge into the high hole */
        high_hole->offset = offset;
        high_hole->size += size;
    } else {
        /* Neither hole is adjacent; make a new one */
        mos_vma_hole *hole = (mos_vma_hole*)calloc(1, sizeof(*hole));
        assert(hole);
        if(hole)
        {
            hole->offset = offset;
            hole->size = size;

            /* Add it after the high hole so we maintain high-to-low ordering */
            if (high_hole)
                list_add(&hole->link, &high_hole->link);
            else
                list_add(&hole->link, &heap->holes);
        }
    }

    mos_vma_heap_validate(heap);
}
