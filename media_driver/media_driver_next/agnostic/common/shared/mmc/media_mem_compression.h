/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_mem_compression.h
//! \brief    Defines the common interface for mmc
//! \details
//!

#ifndef __MEDIA_MEM_COMPRESSION_H__
#define __MEDIA_MEM_COMPRESSION_H__

#include "mhw_render.h"
#include "mos_os.h"
#include "mos_os_specific.h"

#define MAX_AUXTALBE_REGISTER_COUNT 4

class MediaMemComp
{
public:
    //!
    //! \brief    Construct
    //!
    MediaMemComp(PMOS_INTERFACE osInterface, MhwMiInterface *miInterface);

    //!
    //! \brief    Deconstructor
    //!
    virtual ~MediaMemComp()
    {
    }

    //!
    //! \brief    SendPrologCmd
    //!
    virtual MOS_STATUS SendPrologCmd(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        MOS_GPU_CONTEXT             gpuContext);

    //!
    //! \brief    SendPrologCmd, for >=VE2.0
    //!
    virtual MOS_STATUS SendPrologCmd(
        PMOS_COMMAND_BUFFER         cmdBuffer);

    //!
    //! \brief    SetSurfaceMmcState
    //!
    virtual MOS_STATUS SetSurfaceMmcState(
        PMOS_SURFACE surface);

    //!
    //! \brief    SetSurfaceMmcMode
    //!
    virtual MOS_STATUS SetSurfaceMmcMode(
        PMOS_SURFACE surface);

    //!
    //! \brief    GetSurfaceMmcState
    //!
    virtual MOS_STATUS GetSurfaceMmcState(
        PMOS_SURFACE surface,
        MOS_MEMCOMP_STATE *mmcMode);

    virtual MOS_STATUS GetSurfaceMmcFormat(
        PMOS_SURFACE surface,
        uint32_t *   mmcFormat);

    //!
    //! \brief    GetResourceMmcState
    //!
    virtual MOS_STATUS GetResourceMmcState(
        PMOS_RESOURCE resource,
        MOS_MEMCOMP_STATE &mmcMode);

    //!
    //! \brief    IsMmcEnabled
    //!
    bool IsMmcEnabled();

    //!
    //! \brief    DisableMmc
    //!
    void DisableMmc();

    //!
    //! \brief    InitMmcEnabled
    //!
    MOS_STATUS InitMmcEnabled();

protected:
    //!
    //! \brief    UpdateMmcInUseFeature
    //!
    virtual MOS_STATUS UpdateMmcInUseFeature();

private:

private:
    //!
    //! \brief    IsMmcFeatureEnabled
    //!
    bool IsMmcFeatureEnabled();

    //!
    //! \brief    AddMiLoadRegisterImmCmd
    //!
    MOS_STATUS AddMiLoadRegisterImmCmd(PMOS_COMMAND_BUFFER cmdBuffer, uint32_t auxtableRegisterIndex);

public:
    // Interface
    PMOS_INTERFACE              m_osInterface = nullptr;
    MhwMiInterface              *m_mhwMiInterface = nullptr;

protected:
    bool                        m_mmcEnabled = false;
    bool                        m_bComponentMmcEnabled = false;
    uint32_t                    m_mmcFeatureId = __MOS_USER_FEATURE_KEY_MAX_ID;
    uint32_t                    m_mmcInuseFeatureId = __MOS_USER_FEATURE_KEY_MAX_ID;

private:
    uint64_t                    m_auxTableBaseAddr;
    enum AuxtableRegisterIndex
    {
        renderAuxtableRegisterIndex = 0,
        veboxAuxtableRegisterIndex,
        vdboxAuxtableRegisterIndex,
        maxAuxtableRegisterIndex
    };

    struct AuxTableRegisterParams
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS lriParams[MAX_AUXTALBE_REGISTER_COUNT];
        uint32_t                        size;
    } ;

    AuxTableRegisterParams m_auxtableRegisterArray[maxAuxtableRegisterIndex];
};

#endif //__MEDIA_MEM_COMPRESSION_H__
