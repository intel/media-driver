/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     cp_lib.h
//! \brief    common defines for CPLIB
//!
#include "cplib.h"
#include "cplib_utils.h"



namespace GmmLib{ class Context; } 
class MediaLibvaCapsCpInterface;
class DdiCpInterface;
class MhwCpInterface;
class MosCpInterface;
typedef struct _MOS_INTERFACE* PMOS_INTERFACE;
typedef struct _MOS_OS_CONTEXT MOS_CONTEXT;

typedef GmmLib::Context GMM_GLOBAL_CONTEXT;

extern GMM_GLOBAL_CONTEXT *pGmmGlobalContext;

extern "C" CPLIB_EXPORT MediaLibvaCapsCpInterface* Create_MediaLibvaCapsCp();
extern "C" CPLIB_EXPORT void Delete_MediaLibvaCapsCp(MediaLibvaCapsCpInterface* pMediaLibvaCapsCpInterface);
extern "C" CPLIB_EXPORT DdiCpInterface* Create_DdiCp(MOS_CONTEXT* pMosCtx);
extern "C" CPLIB_EXPORT void Delete_DdiCp(DdiCpInterface* pDdiCpInterface);
extern "C" CPLIB_EXPORT MosCpInterface* Create_MosCp(void *osInterface);
extern "C" CPLIB_EXPORT void Delete_MosCp(MosCpInterface* pMosCpInterface);
extern "C" CPLIB_EXPORT MhwCpInterface* Create_MhwCp(PMOS_INTERFACE   osInterface);
extern "C" CPLIB_EXPORT void Delete_MhwCp(MhwCpInterface* pMhwCpInterface);

extern "C" CPLIB_EXPORT uint32_t Get_CPLib_Major_Version()
{
    return CPLIB_MAJORY_VERSION;
}

extern "C" CPLIB_EXPORT void Init_CPLib_Symbols()
{
    CPLibUtils::m_symbols.clear();

    CPLibUtils::m_symbols[CPLibUtils::FUNC_GET_CPLIB_MAJOR_VERSION] = reinterpret_cast<void*>(Get_CPLib_Major_Version);
    CPLibUtils::m_symbols[CPLibUtils::FUNC_CREATE_DDICP]            = reinterpret_cast<void*>(Create_DdiCp);
    CPLibUtils::m_symbols[CPLibUtils::FUNC_DELETE_DDICP]            = reinterpret_cast<void*>(Delete_DdiCp);
    CPLibUtils::m_symbols[CPLibUtils::FUNC_CREATE_MHWCP]            = reinterpret_cast<void*>(Create_MhwCp);
    CPLibUtils::m_symbols[CPLibUtils::FUNC_DELETE_MHWCP]            = reinterpret_cast<void*>(Delete_MhwCp);
    CPLibUtils::m_symbols[CPLibUtils::FUNC_CREATE_MOSCP]            = reinterpret_cast<void*>(Create_MosCp);
    CPLibUtils::m_symbols[CPLibUtils::FUNC_DELETE_MOSCP]            = reinterpret_cast<void*>(Delete_MosCp);
    CPLibUtils::m_symbols[CPLibUtils::FUNC_CREATE_MEDIALIBVACAPSCP] = reinterpret_cast<void*>(Create_MediaLibvaCapsCp);
    CPLibUtils::m_symbols[CPLibUtils::FUNC_DELETE_MEDIALIBVACAPSCP] = reinterpret_cast<void*>(Delete_MediaLibvaCapsCp);
}

extern "C" CPLIB_EXPORT void Init_CPLib_Gmm(GMM_GLOBAL_CONTEXT* pCtx)
{
    pGmmGlobalContext = pCtx;
}