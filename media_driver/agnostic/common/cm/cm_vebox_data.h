/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      cm_vebox_data.h 
//! \brief     Contains Class CmVeboxData definitions 
//!
#pragma once
#include "cm_def.h"
#include "cm_array.h"

namespace CMRT_UMD
{
class CmVeboxData : public CmDynamicArray
{
public:
    static int32_t Create( uint8_t *pStateData, uint8_t *pSurfaceData, CmVeboxData*& pVeboxData );
    static int32_t Destroy( CmVeboxData* &pVeboxData );

    int32_t GetData( uint8_t*& pStateData, uint8_t*& pSurfaceData );
    int32_t SetVeboxDataSize(uint32_t uiStateDataSize, uint32_t uiSurfaceDataSize);
    int32_t GetVeboxDataSize(uint32_t& uiStateDataSize, uint32_t& uiSurfaceDataSize);

    int32_t Acquire(void);
    int32_t SafeRelease(void);

protected:

    CmVeboxData( uint8_t *pStateData, uint8_t *pSurfaceData );
    ~CmVeboxData ( void );

    int32_t Initialize( void );

    uint32_t m_StateDataSize;
    uint8_t *m_pStateData;

    uint32_t m_SurfaceDataSize;
    uint8_t *m_pSurfaceData;

    int32_t m_RefCount;

private:
    CmVeboxData& operator= (const CmVeboxData& other);
};
}; //namespace
