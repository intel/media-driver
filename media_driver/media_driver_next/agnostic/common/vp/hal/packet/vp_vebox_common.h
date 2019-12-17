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
#ifndef __VP_VEBOX_COMMON_H__
#define __VP_VEBOX_COMMON_H__

#include "mhw_vebox.h"
#include "vphal_common.h"
#include "renderhal_g12.h"

typedef class VP_VEBOX_RENDER_DATA           *PVPHAL_VEBOX_RENDER_DATA;
typedef struct VP_VEBOX_RENDER_DATA_EXT      *PVPHAL_VEBOX_RENDER_DATA_EXT;

typedef struct VPHAL_VEBOX_STATE_PARAMS      *PVPHAL_VEBOX_STATE_PARAMS;
typedef class MhwVeboxInterface              *PMHW_VEBOX_INTERFACE;

typedef class VPHAL_VEBOX_IECP_PARAMS        *PVPHAL_VEBOX_IECP_PARAMS;
typedef class VPHAL_VEBOX_IECP_PARAMS_EXT    *PVPHAL_VEBOX_IECP_PARAMS_EXT;
class VPHAL_VEBOX_IECP_PARAMS
{
public:
    PVPHAL_COLORPIPE_PARAMS         pColorPipeParams = nullptr;
    PVPHAL_PROCAMP_PARAMS           pProcAmpParams = nullptr;
    MOS_FORMAT                      dstFormat = Format_Any;
    MOS_FORMAT                      srcFormat = Format_Any;

    // CSC params
    bool                            bCSCEnable = false;       // Enable CSC transform
    float*                          pfCscCoeff = nullptr;     // [3x3] CSC Coeff matrix
    float*                          pfCscInOffset = nullptr;  // [3x1] CSC Input Offset matrix
    float*                          pfCscOutOffset = nullptr; // [3x1] CSC Output Offset matrix
    bool                            bAlphaEnable = false;     // Alpha Enable Param
    uint16_t                        wAlphaValue = 0;          // Color Pipe Alpha Value

    VPHAL_VEBOX_IECP_PARAMS()
    {
    }
    virtual ~VPHAL_VEBOX_IECP_PARAMS()
    {
    }
    virtual PVPHAL_VEBOX_IECP_PARAMS_EXT   GetExtParams() { return nullptr; }
};

typedef struct VPHAL_VEBOX_STATE_PARAMS_EXT *PVPHAL_VEBOX_STATE_PARAMS_EXT;
struct VPHAL_VEBOX_STATE_PARAMS
{
    VPHAL_VEBOX_STATE_PARAMS()
    {
    }

    virtual     ~VPHAL_VEBOX_STATE_PARAMS()
    {
        pVphalVeboxIecpParams = nullptr;
        pVphalVeboxDndiParams = nullptr;
    }

    PMHW_VEBOX_DNDI_PARAMS          pVphalVeboxDndiParams = nullptr;
    PVPHAL_VEBOX_IECP_PARAMS        pVphalVeboxIecpParams = nullptr;

    virtual PVPHAL_VEBOX_STATE_PARAMS_EXT   GetExtParams() { return nullptr; }
};

typedef struct _VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS
{
    PVPHAL_SURFACE                  pSurfInput;
    PVPHAL_SURFACE                  pSurfOutput;
    PVPHAL_SURFACE                  pSurfSTMM;
    PVPHAL_SURFACE                  pSurfDNOutput;
    PVPHAL_SURFACE                  pSurfSkinScoreOutput;
    bool                            bDIEnable;
} VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS, *PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS;

enum GFX_MEDIA_VEBOX_DI_OUTPUT_MODE
{
    MEDIA_VEBOX_DI_OUTPUT_BOTH = 0,
    MEDIA_VEBOX_DI_OUTPUT_PREVIOUS = 1,
    MEDIA_VEBOX_DI_OUTPUT_CURRENT = 2
};

class VP_VEBOX_RENDER_DATA
{
  public:
    VP_VEBOX_RENDER_DATA()
    {
    }
    VP_VEBOX_RENDER_DATA(const VP_VEBOX_RENDER_DATA&) = delete;
    VP_VEBOX_RENDER_DATA& operator=(const VP_VEBOX_RENDER_DATA&) = delete;
    virtual  ~VP_VEBOX_RENDER_DATA()
    {

        if (m_pVeboxStateParams)
        {
            MOS_Delete(m_pVeboxStateParams);
            m_pVeboxStateParams = nullptr;
        }

        if (m_pVeboxIecpParams)
        {
            MOS_Delete(m_pVeboxIecpParams);
            m_pVeboxIecpParams = nullptr;
        }
    }

    MOS_STATUS Init()
    {
        // Vebox State Parameters
        // m_pVeboxStateParams needs to be set to nullptr in constructor

        if (!m_pVeboxStateParams)
        {
            m_pVeboxStateParams = MOS_New(VPHAL_VEBOX_STATE_PARAMS);
            if (!m_pVeboxStateParams)
            {
                return MOS_STATUS_NO_SPACE;
            }
        }

        // Vebox IECP State Parameters
        // m_pVeboxIecpParams needs to be set to nullptr in constructor
        if (!m_pVeboxIecpParams)
        {
            m_pVeboxIecpParams = MOS_New(VPHAL_VEBOX_IECP_PARAMS);
            if (!m_pVeboxIecpParams)
            {
                return MOS_STATUS_NO_SPACE;
            }
        }
        return MOS_STATUS_SUCCESS;
    }
    PVPHAL_VEBOX_STATE_PARAMS           GetVeboxStateParams() { return m_pVeboxStateParams; }
    PVPHAL_VEBOX_IECP_PARAMS            GetVeboxIECPParams() { return m_pVeboxIecpParams; }
    virtual PVPHAL_VEBOX_RENDER_DATA_EXT GetExtData() { return nullptr; }

    // Flags
    bool                                bRefValid = false;
    bool                                bSameSamples = false;
    bool                                bProgressive = false;
    bool                                bDenoise = false;
#if VEBOX_AUTO_DENOISE_SUPPORTED
    bool                                bAutoDenoise = false;
#endif

    bool                                bChromaDenoise = false;
    bool                                bOutOfBound = false;
    bool                                bVDIWalker = false;

    bool                                bIECP = false;
    bool                                bColorPipe = false;
    bool                                bProcamp = false;
    bool                                bBeCsc = false;

    // DNDI/Vebox
    bool                                bDeinterlace = false;
    bool                                bSingleField = false;
    bool                                bTFF = false;
    bool                                bTopField = false;
    bool                                bVeboxBypass = false;
    bool                                b60fpsDi = false;
    bool                                bQueryVariance = false;

    // Surface Information
    int32_t                             iFrame0 = 0;
    int32_t                             iFrame1 = 0;
    int32_t                             iCurDNIn = 0;
    int32_t                             iCurDNOut = 0;
    int32_t                             iCurHistIn = 0;
    int32_t                             iCurHistOut = 0;

    // Geometry
    int32_t                             iBlocksX = 0;
    int32_t                             iBlocksY = 0;
    int32_t                             iBindingTable = 0;
    int32_t                             iMediaID0 = 0;
    int32_t                             iMediaID1 = 0;

    // Perf
    VPHAL_PERFTAG                       PerfTag = VPHAL_NONE;

    // States
    PMHW_VEBOX_HEAP_STATE               pVeboxState = nullptr;
    PVPHAL_SURFACE                      pRenderTarget = nullptr;

    MHW_SAMPLER_STATE_PARAM             SamplerStateParams = {};

    MHW_VEBOX_DNDI_PARAMS               VeboxDNDIParams = {};

    PVPHAL_ALPHA_PARAMS                 pAlphaParams = nullptr;

    // Vebox output parameters
    VPHAL_OUTPUT_PIPE_MODE              OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
    // Current component
    MOS_COMPONENT                       Component = COMPONENT_UNKNOWN;

    // Memory compression flag
    bool                                bEnableMMC = false;                             //!< Enable memory compression flag

    // Scaling ratio from source to render target
    // Scaling ratio is needed to determine if SFC or VEBOX is used
    float                               fScaleX = 0.0;                                //!< X Scaling ratio
    float                               fScaleY = 0.0;                                //!< Y Scaling ratio

protected:
    // Vebox State Parameters
    PVPHAL_VEBOX_STATE_PARAMS           m_pVeboxStateParams = nullptr;                    //!< auto allocated param instance for set/submit VEBOX cmd
    // Vebox IECP Parameters
    PVPHAL_VEBOX_IECP_PARAMS            m_pVeboxIecpParams = nullptr;                     //!< auto allocated param instance for set/submit VEBOX IECP cmd
  };

#endif // !__VP_VEBOX_COMMON_H__
