/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     vp_pipeline_common.h
//! \brief    Defines the common interface for vp pipeline
//!           this file is for the base interface which is shared by all features/vp Pipelines.
//!
#ifndef __VP_PIPELINE_COMMON_H__
#define __VP_PIPELINE_COMMON_H__

#include "mos_utilities.h"
#include "vphal_common.h"
#include "renderhal.h"
#include "vphal.h"

using VP_PIPELINE_PARAMS   = VPHAL_RENDER_PARAMS;
using PVP_PIPELINE_PARAMS  = VPHAL_RENDER_PARAMS*;
using PCVP_PIPELINE_PARAMS = const VPHAL_RENDER_PARAMS*;

struct VP_SURFACE
{
    MOS_SURFACE                 *osSurface;         //!< mos surface
    bool                        isInternalSurface;  //!< true if created by feature manager. false if the surface is from DDI.
    VPHAL_CSPACE                ColorSpace;         //!< Color Space
    uint32_t                    ChromaSiting;       //!< ChromaSiting

    bool                        bQueryVariance;     //!< enable variance query. Not in use for internal surface
    int32_t                     FrameID;            //!< Not in use for internal surface
    bool                        ExtendedGamut;      //!< Extended Gamut Flag. Not in use for internal surface
    VPHAL_PALETTE               Palette;            //!< Palette data. Not in use for internal surface
    VPHAL_SURFACE_TYPE          SurfType;           //!< Surface type (context). Not in use for internal surface
    uint32_t                    uFwdRefCount;       //!< Not in use for internal surface
    uint32_t                    uBwdRefCount;       //!< Not in use for internal surface
    VPHAL_SURFACE               *pFwdRef;           //!< Use VP_SURFACE instead of VPHAL_SURFACE later. Not in use for internal surface.
    VPHAL_SURFACE               *pBwdRef;           //!< Use VP_SURFACE instead of VPHAL_SURFACE later. Not in use for internal surface.
    VPHAL_SAMPLE_TYPE           SampleType;         //!<  Interlaced/Progressive sample type.
    // Use index of m_InputSurfaces for layerID. No need iLayerID here anymore.

    RECT                        rcSrc;              //!< Source rectangle
    RECT                        rcDst;              //!< Destination rectangle
    RECT                        rcMaxSrc;           //!< Max source rectangle

    PVPHAL_SURFACE              pCurrent;           //!< Pointer to related vphal surface. Only be used in VpVeboxCmdPacket::PacketInit for current
                                                    //!< stage. Should be removed after vphal surface being cleaned from VpVeboxCmdPacket.
};

struct _VP_MHWINTERFACE
{
    // Internals
    PLATFORM                    m_platform;
    MEDIA_FEATURE_TABLE         *m_skuTable;
    MEDIA_WA_TABLE              *m_waTable;

    // States
    PMOS_INTERFACE              m_osInterface;
    PRENDERHAL_INTERFACE        m_renderHal;
    PMHW_VEBOX_INTERFACE        m_veboxInterface;
    MhwCpInterface             *m_cpInterface;
    PMHW_SFC_INTERFACE          m_sfcInterface;
    VphalRenderer              *m_renderer;
    PMHW_MI_INTERFACE           m_mhwMiInterface;

    // Render GPU context/node
    MOS_GPU_NODE                m_renderGpuNode;
    MOS_GPU_CONTEXT             m_renderGpuContext;

    // vp Pipeline workload status report
    PVPHAL_STATUS_TABLE        m_statusTable;
};

// To define the features enabling on different engines
struct _VP_EXECUTE_CAPS
{
    union {
        uint32_t value;
        struct {
            uint32_t bVebox         : 1;   // Vebox needed;
            uint32_t bSFC           : 1;   // SFC needed;
            uint32_t bRender        : 1;   // Render Only needed;
            // Vebox Features
            uint32_t bDN            : 1;   // Vebox DN needed;
            uint32_t bDI            : 1;   // Vebox DNDI enabled
            uint32_t bIECP          : 1;   // Vebox IECP needed;
            uint32_t bLACE          : 1;   // Vebox LACE Needed;
            uint32_t bQueryVariance : 1;
            uint32_t bRefValid      : 1;   // Vebox Ref is Valid
            uint32_t bSTD           : 1;   // Vebox LACE STD Needed;

            // SFC features
            uint32_t bSfcCsc        : 1;   // Sfc Csc enabled
            uint32_t bSfcRotMir     : 1;   // Sfc Rotation/Mirror needed;
            uint32_t bSfcScaling    : 1;   // Sfc Scaling Needed;
            uint32_t bSfcIef        : 1;   // Sfc Details Needed;

            // Render Features
            uint32_t bComposite     : 1;
            uint32_t reserved       : 18;  // Reserved
        };
    };
};

typedef struct _VP_EngineEntry
{
    union
    {
        struct
        {
            uint32_t bEnabled : 1;
            uint32_t SfcNeeded : 2;
            uint32_t VeboxNeeded : 2;
            uint32_t RenderNeeded : 2;
            uint32_t VeboxARGBOut : 1;
            uint32_t VeboxARGB10bitOutput : 1;
            uint32_t DisableVeboxSFCMode : 1;
            uint32_t FurtherProcessNeeded : 1;
            uint32_t CompositionNeeded : 1;
            uint32_t reserve : 20;
        };
        uint32_t value;
    };
}VP_EngineEntry;

enum _VP_PACKET_ENGINE
{
    VP_PACKET_COMP = 0,
    VP_PACKET_VEBOX,
};

typedef enum _VP_COMP_BYPASS_MODE
{
    VP_COMP_BYPASS_NOT_SET  = 0xffffffff,
    VP_COMP_BYPASS_DISABLED = 0x0,
    VP_COMP_BYPASS_ENABLED  = 0x1
} VP_COMP_BYPASS_MODE, * PVP_COMP_BYPASS_MODE;

using VP_MHWINTERFACE  = _VP_MHWINTERFACE;
using PVP_MHWINTERFACE = VP_MHWINTERFACE * ;
using VP_EXECUTE_CAPS  = _VP_EXECUTE_CAPS;
using VP_PACKET_ENGINE = _VP_PACKET_ENGINE;
using PVP_SURFACE      = VP_SURFACE*;

#endif