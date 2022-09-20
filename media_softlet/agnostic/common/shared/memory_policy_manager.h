/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     memory_policy_manager.h
//! \brief    Defines interfaces for media memory policy manager.


#ifndef __MEMORY_POLICY_MANAGER_H__
#define __MEMORY_POLICY_MANAGER_H__

#include "mos_os.h"

//! \param   [in] skuTable
//!          The pointer to MEDIA_FEATURE_TABLE
//! \param   [in] waTable
//!          The pointer to MEDIA_WA_TABLE
//! \param   [in, out] resInfo
//!          The pointer to GMM_RESOURCE_INFO, resource description which gets updated
//! \param   [in] resName
//!          The pointer to resource name
//! \param   [in] uiType
//!          The pointer to resource type
//! \param   [in] preferredMemType
//!          Prefer which type of memory is allocated (device memory, system memory or default setting).
struct MemoryPolicyParameter
{
    MEDIA_FEATURE_TABLE *skuTable;
    MEDIA_WA_TABLE *waTable;
    GMM_RESOURCE_INFO *resInfo;
    const char* resName;
    uint32_t uiType;
    int preferredMemType;
    bool isServer;
};

class MemoryPolicyManager
{

public:

    //! \brief   Updates resource memory policy
    //!
    //! \details Update memory policy to decide which type of memory is allocated (device memory, system memory or default setting).
    //! \param   [in] memPolicyPar
    //!          The pointer to MemoryPolicyParameter
    //!
    //! \return  new memory policy
    static int UpdateMemoryPolicy(MemoryPolicyParameter* memPolicyPar);

private:

    //! \brief   Updates resource memory policy with WA
    //!
    //! \details Update memory policy to decide which type of memory is allocated (device memory, system memory or default setting).
    //! \param   [in] memPolicyPar
    //!          The pointer to MemoryPolicyParameter
    //!
    //! \param   [in/out] mem_type
    //!          The pointer to mem_type

    //! \return  new memory policy
    static int UpdateMemoryPolicyWithWA(MemoryPolicyParameter* memPolicyPar, int& mem_type);

MEDIA_CLASS_DEFINE_END(MemoryPolicyManager)
};


#endif //__MEMORY_POLICY_MANAGER_H__
