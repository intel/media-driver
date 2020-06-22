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
//! \file     cplib_utils.h
//! \brief    Utils to interact with CPLib
//!
#ifndef __CPLIB_UTILS_H__
#define __CPLIB_UTILS_H__

#include "mos_util_debug.h"

#include <dlfcn.h>
#include <unordered_map>
#include <mutex>

#define REQUIRED_CPLIB_MAJOR_VERSION 1

#define CPLIB_NORMALMESSAGE(_message, ...)                                    \
MOS_NORMALMESSAGE(MOS_COMPONENT_CPLIB, MOS_CP_SUBCOMP_CPLIB, _message, ##__VA_ARGS__)

#define CPLIB_ASSERTMESSAGE(_message, ...)                                    \
MOS_ASSERTMESSAGE(MOS_COMPONENT_CPLIB, MOS_CP_SUBCOMP_CPLIB, _message, ##__VA_ARGS__)

typedef struct VADriverContext* VADriverContextP;

class CPLibUtils
{
public:
    CPLibUtils() = delete;
    ~CPLibUtils() = delete;
    CPLibUtils(const CPLibUtils& ) = delete;
    CPLibUtils & operator=(const CPLibUtils& other) = delete;
    
    //!
    //! \brief    Try load CPLIB
    //!
    static bool LoadCPLib(VADriverContextP ctx);

    //!
    //! \brief    Unload CPLIB if it is loaded
    //!
    static void UnloadCPLib(VADriverContextP ctx);

    //!
    //! \brief    Invoke CPLIB function
    //! \param    [out] _ret
    //!           To store the return of involved function
    //!
    //! \param    [in] *symbol
    //!           The symbol want to involve in CPLIB
    //!
    //! \param    [in] args
    //!           All the args needed for the function you involved in CPLIB
    //!
    template<typename _FuncType, typename _RetType, typename... Args> 
    static void InvokeCpFunc(_RetType& _ret, const char * symbol, Args... args);

    //!
    //! \brief    Invoke CPLIB function
    //!
    //! \param    [in] *symbol
    //!           The symbol want to involve in CPLIB
    //!
    //! \param    [in] args
    //!           All the args needed for the function you involved in CPLIB
    //!
    template<typename _FuncType, typename... Args> 
    static void InvokeCpFunc(const char * symbol, Args... args);

    static const char* CPLIB_PATH;
    static const char* FUNC_GET_CPLIB_MAJOR_VERSION;
    static const char* FUNC_INIT_CPLIB;
    static const char* FUNC_RELEASE_CPLIB;
    static const char* FUNC_CREATE_DDICP;
    static const char* FUNC_DELETE_DDICP;
    static const char* FUNC_CREATE_MHWCP;
    static const char* FUNC_DELETE_MHWCP;
    static const char* FUNC_CREATE_MOSCP;
    static const char* FUNC_DELETE_MOSCP;
    static const char* FUNC_CREATE_MEDIALIBVACAPSCP;
    static const char* FUNC_DELETE_MEDIALIBVACAPSCP;
    static const char* FUNC_CREATE_SECUREDECODE;
    static const char* FUNC_DELETE_SECUREDECODE;
    static const char* FUNC_CREATE_DECODECP;
    static const char* FUNC_DELETE_DECODECP;
    static const char* FUNC_CREATE_CPSTREAMOUT;
    static const char* FUNC_DELETE_CPSTREAMOUT;

    static std::unordered_map<const char*, void*> m_symbols;
    static std::mutex                             m_referenceMutex;
    static int                                    m_referenceCount;

private:
    static void* m_phandle;
};

template<typename _FuncType, typename _RetType, typename... Args> 
void CPLibUtils::InvokeCpFunc(_RetType& _ret, const char* symbol, Args... args)
{
    if (m_symbols.end() != m_symbols.find(symbol))
    {
        _FuncType func = reinterpret_cast<_FuncType>(m_symbols[symbol]);
        if (nullptr != func)
        {
            _ret = func(args...);
            return;
        }
    }

    CPLIB_NORMALMESSAGE("Symbol: %s not found in CPLIB", symbol);
}

template<typename _FuncType, typename... Args> 
void CPLibUtils::InvokeCpFunc(const char* symbol, Args... args)
{
    if (m_symbols.end() != m_symbols.find(symbol))
    {
        _FuncType func = reinterpret_cast<_FuncType>(m_symbols[symbol]);
        if (nullptr != func)
        {
            func(args...);
            return;
        }
    }

    CPLIB_NORMALMESSAGE("Symbol: %s not found in CPLIB", symbol);
}

#endif
