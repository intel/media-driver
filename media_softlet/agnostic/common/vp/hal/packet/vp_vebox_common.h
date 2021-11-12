/*
* Copyright (c) 2018-2021, Intel Corporation
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
#include "renderhal_g12_base.h"
#include "vp_filter.h"

typedef class VP_VEBOX_RENDER_DATA           *PVPHAL_VEBOX_RENDER_DATA;
typedef struct VP_VEBOX_RENDER_DATA_EXT      *PVPHAL_VEBOX_RENDER_DATA_EXT;

typedef struct VPHAL_VEBOX_STATE_PARAMS      *PVPHAL_VEBOX_STATE_PARAMS;
typedef class MhwVeboxInterface              *PMHW_VEBOX_INTERFACE;

typedef class VPHAL_VEBOX_IECP_PARAMS        *PVPHAL_VEBOX_IECP_PARAMS;

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
    PVP_SURFACE                     pSurfInput;
    PVP_SURFACE                     pSurfOutput;
    PVP_SURFACE                     pSurfSTMM;
    PVP_SURFACE                     pSurfDNOutput;
    PVP_SURFACE                     pSurfSkinScoreOutput;
    bool                            bDIEnable;
    bool                            b3DlutEnable;
} VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS, *PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS;

enum GFX_MEDIA_VEBOX_DI_OUTPUT_MODE
{
    MEDIA_VEBOX_DI_OUTPUT_BOTH = 0,
    MEDIA_VEBOX_DI_OUTPUT_PREVIOUS = 1,
    MEDIA_VEBOX_DI_OUTPUT_CURRENT = 2
};

enum MEDIASTATE_GCC_BASIC_MODE_SELECTION
{
    MEDIASTATE_GCC_DEFAULT = 0,
    MEDIASTATE_GCC_SCALING_FACTOR,
    MEDIASTATE_GCC_SINGLE_AXIS_GAMMA_CORRECTION,
    MEDIASTATE_GCC_SCALING_FACTOR_WITH_FIXED_LUMA
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

MEDIA_CLASS_DEFINE_END(VP_VEBOX_RENDER_DATA)
  };

class VpVeboxRenderData
{
public:
    virtual ~VpVeboxRenderData()
    {
        if (pAceCacheData)
        {
            MOS_FreeMemAndSetNull(pAceCacheData);
        }
    }

    virtual MOS_STATUS Init()
    {
        MHW_ACE_PARAMS aceParams = {};
        PerfTag                  = VPHAL_NONE;

        DN.value                 = 0;
        DI.value                 = 0;
        IECP.STE.value           = 0;
        IECP.ACE.value           = 0;
        IECP.TCC.value           = 0;
        IECP.PROCAMP.value       = 0;
        IECP.LACE.value          = 0;

        VP_PUBLIC_CHK_STATUS_RETURN(MOS_SecureMemcpy(&aceParams,
                sizeof(MHW_ACE_PARAMS),
                &m_veboxIecpParams.AceParams,
                sizeof(MHW_ACE_PARAMS)));

        MOS_ZeroMemory(&m_veboxDNDIParams, sizeof(MHW_VEBOX_DNDI_PARAMS));
        MOS_ZeroMemory(&m_veboxIecpParams, sizeof(MHW_VEBOX_IECP_PARAMS));
        MOS_ZeroMemory(&m_veboxGamutParams, sizeof(MHW_VEBOX_GAMUT_PARAMS));
        MOS_ZeroMemory(&m_HvsParams, sizeof(VPHAL_HVSDENOISE_PARAMS));

        VP_PUBLIC_CHK_STATUS_RETURN(MOS_SecureMemcpy(&m_veboxIecpParams.AceParams,
                sizeof(MHW_ACE_PARAMS),
                &aceParams,
                sizeof(MHW_ACE_PARAMS)));

        VP_PUBLIC_CHK_STATUS_RETURN(InitChromaSampling());

        return MOS_STATUS_SUCCESS;
    }

    virtual VPHAL_HVSDENOISE_PARAMS &GetHVSParams()
    {
        return m_HvsParams;
    }

    virtual MHW_VEBOX_DNDI_PARAMS &GetDNDIParams()
    {
        return m_veboxDNDIParams;
    }

    virtual MHW_VEBOX_IECP_PARAMS &GetIECPParams()
    {
        return m_veboxIecpParams;
    }

    virtual MHW_VEBOX_CHROMA_SAMPLING &GetChromaSubSamplingParams()
    {
        return m_chromaSampling;
    }

    virtual MHW_VEBOX_GAMUT_PARAMS &GetGamutParams()
    {
        return m_veboxGamutParams;
    }

    bool IsDiEnabled()
    {
        return DI.bDeinterlace || DI.bQueryVariance;
    }

    union
    {
        struct
        {
            uint32_t bDnEnabled             : 1;        // DN enabled;
            uint32_t bAutoDetect            : 1;        // Auto DN enabled
            uint32_t bChromaDnEnabled       : 1;        // Chroma DN Enabled
            uint32_t bHvsDnEnabled          : 1;        // HVS DN Enabled
        };
        uint32_t value = 0;
    } DN;

    union
    {
        struct
        {
            uint32_t bDeinterlace           : 1;    // DN enabled;
            uint32_t bQueryVariance         : 1;    // DN enabled;
            uint32_t bFmdEnabled            : 1;    // FMD enabled;
            uint32_t bTFF                   : 1;
        };
        uint32_t value = 0;
    } DI;

    union
    {
        struct
        {
            uint32_t bEnable3Dlut           : 1;    // 3Dlut HDR enabled;
            uint32_t VE3DlutLength          : 30;
        };
    } VE3DLUT;

    union
    {
        struct
        {
            bool           bHdr3DLut;             //!< Enable 3DLut to process HDR
            bool           bUseVEHdrSfc;          //!< Use SFC to perform CSC/Scaling for HDR content
            uint32_t       uiMaxDisplayLum;       //!< Maximum Display Luminance
            uint32_t       uiMaxContentLevelLum;  //!< Maximum Content Level Luminance
            VPHAL_HDR_MODE hdrMode;
        };
    } HDR3DLUT;
    struct
    {
        union
        {
            struct
            {
                uint32_t bProcampEnabled : 1;              // STE enabled;
            };
            uint32_t value = 0;
        } PROCAMP;

        union
        {
            struct
            {
                uint32_t bSteEnabled : 1;              // STE enabled;
            };
            uint32_t value = 0;
        } STE;

        union
        {
            struct
            {
                uint32_t bTccEnabled : 1;              // TCC enabled;
            };
            uint32_t value = 0;
        } TCC;

        union
        {
            struct
            {
                uint32_t bAceEnabled            : 1;    // ACE enabled;
                uint32_t bAceHistogramEnabled   : 1;    // ACE histogram enabled, should be set to 1 when bAceEnabled == 1;
            };
            uint32_t value = 0;
        } ACE;

        union
        {
            struct
            {
                uint32_t bLaceEnabled           : 1;    // DN enabled;
            };
            uint32_t value = 0;
        } LACE;

        union
        {
            struct
            {
                uint32_t bBeCSCEnabled           : 1;   // Back end CSC enabled;
            };
            uint32_t value = 0;
        }BeCSC;

        bool IsIecpEnabled()
        {
            return ACE.bAceEnabled || LACE.bLaceEnabled ||
                    BeCSC.bBeCSCEnabled || TCC.bTccEnabled ||
                    STE.bSteEnabled || PROCAMP.bProcampEnabled;
        }
    } IECP;

    // Perf
    VPHAL_PERFTAG                       PerfTag = VPHAL_NONE;

    uint32_t *pAceCacheData  = nullptr;

    virtual MOS_STATUS InitChromaSampling()
    {
        MOS_ZeroMemory(&m_chromaSampling, sizeof(MHW_VEBOX_CHROMA_SAMPLING));
        m_chromaSampling.BypassChromaUpsampling                     = 1;
        m_chromaSampling.ChromaUpsamplingCoSitedHorizontalOffset    = 0;
        m_chromaSampling.ChromaUpsamplingCoSitedVerticalOffset      = 0;
        m_chromaSampling.BypassChromaDownsampling                   = 1;
        m_chromaSampling.ChromaDownsamplingCoSitedHorizontalOffset  = 0;
        m_chromaSampling.ChromaDownsamplingCoSitedVerticalOffset    = 0;
        return MOS_STATUS_SUCCESS;
    }
protected:
    VPHAL_HVSDENOISE_PARAMS m_HvsParams = {};
    MHW_VEBOX_DNDI_PARAMS   m_veboxDNDIParams = {};
    MHW_VEBOX_IECP_PARAMS   m_veboxIecpParams = {};
    MHW_VEBOX_CHROMA_SAMPLING m_chromaSampling = {};
    MHW_VEBOX_GAMUT_PARAMS  m_veboxGamutParams = {};

MEDIA_CLASS_DEFINE_END(VpVeboxRenderData)
};

using PVpVeboxRenderData = VpVeboxRenderData*;

#endif // !__VP_VEBOX_COMMON_H__