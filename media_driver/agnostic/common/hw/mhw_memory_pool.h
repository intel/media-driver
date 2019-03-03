/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file      mhw_memory_pool.h 
//! \brief         This modules implements simple memory pool infrastructure for MHW 
//!
#ifndef __MHW_MEMORY_POOL_H__
#define __MHW_MEMORY_POOL_H__

#include "mos_os.h"

typedef struct _MHW_MEMORY_POOL_ENTRY *PMHW_MEMORY_POOL_ENTRY;
typedef struct MHW_MEMORY_POOL *PMHW_MEMORY_POOL;

struct MHW_MEMORY_POOL
{

    PMHW_MEMORY_POOL_ENTRY  m_pFirst;             //!< First pool
    PMHW_MEMORY_POOL_ENTRY  m_pLast;              //!< Last pool
    uint32_t                m_dwCount;            //!< Number of pools
    uint32_t                m_dwSize;             //!< Total memory in all pool objects
    uint32_t                m_dwObjSize;          //!< Object size
    uint32_t                m_dwObjAlignment;     //!< Object alignment
    uint32_t                m_dwObjCount;         //!< Total number of objects in all pools

    //!
    //! \brief    Constructor of MHW_MEMORY_POOL
    //! \details  Constructor of MHW_BLOCK_MANAGER which initializes all members
    //! \param    [in] dwObjSize
    //!           Size of object
    //! \param    [in] dwObjAlignment
    //!           Alignment of object
    //!
    MHW_MEMORY_POOL(uint32_t dwObjSize, uint32_t dwObjAlignment);

    //!
    //! \brief    Destructor of MHW_MEMORY_POOL
    //! \details  Destructor of MHW_MEMORY_POOL which releases all the elements in the list
    //!
    ~MHW_MEMORY_POOL();

    //!
    //! \brief    Allocate objects 
    //! \details  Allocate objects and insert them into list
    //! \param    [in] dwObjCount
    //!           Count of objects need to be allocated
    //!
    void  *Allocate(uint32_t dwObjCount);

} ;

#endif // __MHW_MEMORY_POOL_H__
