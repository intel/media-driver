/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     mhw_vdbox_mfx_impl.h
//! \brief    MHW VDBOX MFX interface common base
//! \details
//!
#ifndef __MHW_VDBOX_MFX_IMPL_H__
#define __MHW_VDBOX_MFX_IMPL_H__

#include "mhw_vdbox_mfx_itf.h"
#include "mhw_impl.h"
#include "mhw_mi_impl.h"

#ifdef IGFX_MFX_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_mfx_hwcmd_ext.h"
#endif

#define AVC_MPR_ROWSTORE_BASEADDRESS          256
#define AVC_MPR_ROWSTORE_BASEADDRESS_MBAFF    512
#define AVC_IP_ROWSTORE_BASEADDRESS           512
#define AVC_IP_ROWSTORE_BASEADDRESS_MBAFF     1024
#define AVC_VLF_ROWSTORE_BASEADDRESS          768
#define VP8_IP_ROWSTORE_BASEADDRESS           256
#define VP8_VLF_ROWSTORE_BASEADDRESS          512

namespace mhw
{
namespace vdbox
{
namespace mfx
{

template<typename cmd_t>
class Impl : public Itf, public mhw::Impl
{
    _MFX_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);

public:
    MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) override
    {
        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_RETURN(settings);

        size_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);

        return MOS_SecureMemcpy(m_cacheabilitySettings, size, settings, size);
    }

    MOS_STATUS GetRowstoreCachingAddrs(PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(rowstoreParams);

        bool avc          = rowstoreParams->Mode == CODECHAL_DECODE_MODE_AVCVLD || rowstoreParams->Mode == CODECHAL_ENCODE_MODE_AVC;
        bool vp8          = rowstoreParams->Mode == CODECHAL_DECODE_MODE_VP8VLD || rowstoreParams->Mode == CODECHAL_ENCODE_MODE_VP8;
        bool widthLE4K    = rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_4K;
        bool mbaffOrField = rowstoreParams->bMbaff || !rowstoreParams->bIsFrame;

        //BSD row store cache
        m_bsdMpcRowstoreCache.enabled   = m_bsdMpcRowstoreCache.supported && widthLE4K && (avc || vp8);
        m_bsdMpcRowstoreCache.dwAddress = m_bsdMpcRowstoreCache.enabled ? BSDMPCROWSTORE_BASEADDRESS : 0;

        //MPR row store cache
        m_mprRowstoreCache.enabled   = m_mprRowstoreCache.supported && widthLE4K && avc;
        m_mprRowstoreCache.dwAddress = m_mprRowstoreCache.enabled ? (mbaffOrField ? AVC_MPR_ROWSTORE_BASEADDRESS_MBAFF : AVC_MPR_ROWSTORE_BASEADDRESS) : 0;

        //Intra Prediction row store cache
        m_intraRowstoreCache.enabled   = m_intraRowstoreCache.supported && widthLE4K && (avc || vp8);
        if (m_intraRowstoreCache.enabled)
        {
            m_intraRowstoreCache.dwAddress = avc ? (mbaffOrField ? AVC_IP_ROWSTORE_BASEADDRESS_MBAFF
                                                                 : AVC_IP_ROWSTORE_BASEADDRESS)
                                                                 : VP8_IP_ROWSTORE_BASEADDRESS;
        }
        else
        {
            m_intraRowstoreCache.dwAddress = 0;
        }

        //VLF row store cache
        m_deblockingFilterRowstoreCache.enabled   = m_deblockingFilterRowstoreCache.supported && widthLE4K && ((avc && !mbaffOrField) || vp8);
        m_deblockingFilterRowstoreCache.dwAddress = m_deblockingFilterRowstoreCache.enabled ? (avc ? AVC_VLF_ROWSTORE_BASEADDRESS : VP8_VLF_ROWSTORE_BASEADDRESS) : 0;

        return MOS_STATUS_SUCCESS;
    }

    virtual void SetCacheabilitySettings()
    {
        MHW_FUNCTION_ENTER;
        m_preDeblockingMemoryCtrl.Value = m_osItf->pfnCachePolicyGetMemoryObject(
                                                     MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC,
                                                     m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_postDeblockingMemoryCtrl.Value = m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_POST_DEBLOCKING_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_OriginalUncompressedPictureSourceMemoryCtrl.Value = 
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_DECODE,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_streamoutDataDestinationMemoryCtrl.Value = 
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_intraRowStoreScratchBufferMemoryCtrl.Value = 
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_INTRA_ROWSTORE_SCRATCH_BUFFER_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_deblockingFilterRowStoreScratchMemoryCtrl.Value = 
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_referncePictureMemoryObjectControlStateCtrl.Value =
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_referncePictureMemoryObjectControlStateCtrlDecode.Value =
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_HW_RESOURCE_USAGE_DECODE_INPUT_REFERENCE,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_referncePictureMemoryObjectControlStateCtrlEncode.Value =
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_HW_RESOURCE_USAGE_ENCODE_INPUT_RECON,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_macroblockIldbStreamoutBufferCtrl.Value = 
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_MACROBLOCK_ILDB_STREAM_OUT_BUFFER_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_secondMacroblockIldbStreamoutBufferCtrl.Value =
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_MACROBLOCK_ILDB_STREAM_OUT_BUFFER_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_slicesizeStreamoutDataDestinationCtrl.Value =
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_mfxIndirectBitstreamCtrl.Value =
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_mfdIndirectItCoeffCtrl.Value =
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_MFD_INDIRECT_IT_COEF_OBJECT_DECODE,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_mfxIndirectMvCtrl.Value =
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_MV_OBJECT_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_mfcIndirectPakBseCtrl.Value = 
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_bsdMpcRowStoreScratchBufferCtrl.Value =
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_BSDMPC_ROWSTORE_SCRATCH_BUFFER_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_mprRowStoreScratchBufferCtrl.Value = 
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_MPR_ROWSTORE_SCRATCH_BUFFER_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;
        m_bitplaneReadBufferIndexToMemoryCtrl.Value =
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_BITPLANE_READ_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;

        m_directMvBufferForWriteCtrl.Value =
                                           m_osItf->pfnCachePolicyGetMemoryObject(
                                                      MOS_CODEC_RESOURCE_USAGE_DIRECTMV_BUFFER_CODEC,
                                                      m_osItf->pfnGetGmmClientContext(m_osItf)).DwordValue;

    }

    bool IsRowStoreCachingSupported() override
    {
        return m_rowstoreCachingSupported;
    }

    bool IsMprRowstoreCacheEnabled() override
    {
        return m_mprRowstoreCache.enabled;
    }

    uint32_t GetViewOrder(
        MFD_AVC_DPB_STATE_PAR params,
        uint32_t              currIdx,
        uint32_t              list)
    {
        auto avcPicParams    = params.pAvcPicParams;
        auto mvcExtPicParams = params.pMvcExtPicParams;
        auto avcRefList      = params.ppAvcRefList;

        // No need to check if bottom field since only progressive is supported for mvc clips.
        int32_t  currPOC   = avcPicParams->CurrFieldOrderCnt[0];
        uint32_t numRefs   = (list == LIST_0) ? mvcExtPicParams->NumInterViewRefsL0 : mvcExtPicParams->NumInterViewRefsL1;
        uint32_t viewOrder = 0xF;
        uint32_t currRef   = params.pAvcPicIdx[currIdx].ucPicIdx;

        if (params.pAvcPicIdx[currIdx].bValid &&
            avcRefList[currRef]->bUsedAsInterViewRef &&
            (currPOC == avcRefList[currRef]->iFieldOrderCnt[0]))
        {
            for (uint32_t i = 0; i < numRefs; i++)
            {
                if (mvcExtPicParams->ViewIDList[currIdx] == mvcExtPicParams->InterViewRefList[list][i])
                {
                    viewOrder = mvcExtPicParams->ViewIDList[currIdx];
                    break;
                }
            }
        }

        return viewOrder;
    }

    bool IsDeblockingFilterRowstoreCacheEnabled() override
    {
        return m_deblockingFilterRowstoreCache.enabled;
    }

    bool IsIntraRowstoreCacheEnabled() override
    {
        return m_intraRowstoreCache.enabled;
    }

    bool IsBsdMpcRowstoreCacheEnabled() override
    {
        return m_bsdMpcRowstoreCache.enabled;
    }

    MHW_VDBOX_NODE_IND GetMaxVdboxIndex() override
    {
        return MEDIA_IS_SKU(m_osItf->pfnGetSkuTable(m_osItf), FtrVcs2) ? MHW_VDBOX_NODE_2 : MHW_VDBOX_NODE_1;
    }

    //!
    //! \brief    Get the vdbox num
    //!
    //! \return   bool
    //!           vdbox num got
    //!
    uint8_t GetNumVdbox() override
    {
        MEDIA_ENGINE_INFO mediaEngineInfo = {};
        m_osItf->pfnGetMediaEngineInfo(m_osItf, mediaEngineInfo);

        m_numVdbox = (uint8_t)(mediaEngineInfo.VDBoxInfo.NumberOfVDBoxEnabled);

        return m_numVdbox;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS CheckScalabilityOverrideValidity()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MEDIA_SYSTEM_INFO *gtSystemInfo;
        uint32_t           forceVdbox;
        bool               scalableDecMode;
        bool               useVD1, useVD2, useVD3, useVD4;

        MHW_MI_CHK_NULL(m_osItf);
        scalableDecMode = m_osItf->bHcpDecScalabilityMode ? true : false;
        forceVdbox      = m_osItf->eForceVdbox;
        gtSystemInfo    = m_osItf->pfnGetGtSystemInfo(m_osItf);
        MHW_MI_CHK_NULL(gtSystemInfo);

        if (forceVdbox != MOS_FORCE_VDBOX_NONE &&
            forceVdbox != MOS_FORCE_VDBOX_1 &&
            forceVdbox != MOS_FORCE_VDBOX_2 &&
            // 2 pipes, VDBOX1-BE1, VDBOX2-BE2
            forceVdbox != MOS_FORCE_VDBOX_1_1_2 &&
            forceVdbox != MOS_FORCE_VDBOX_2_1_2)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("user feature forceVdbox value is invalid.");
            return eStatus;
        }

        if (!scalableDecMode &&
            (forceVdbox == MOS_FORCE_VDBOX_1_1_2 ||
                forceVdbox == MOS_FORCE_VDBOX_2_1_2))
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("user feature forceVdbox valude does not consistent with regkey scalability mode.");
            return eStatus;
        }

        if (scalableDecMode && !m_scalabilitySupported)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("user feature scalability mode is not allowed on current platform!");
            return eStatus;
        }

        useVD1 = useVD2 = false;
        if (forceVdbox == 0)
        {
            useVD1 = true;
        }
        else
        {
            MHW_VDBOX_IS_VDBOX_SPECIFIED(forceVdbox, MOS_FORCE_VDBOX_1, MOS_FORCEVDBOX_VDBOXID_BITSNUM, MOS_FORCEVDBOX_MASK, useVD1);
            MHW_VDBOX_IS_VDBOX_SPECIFIED(forceVdbox, MOS_FORCE_VDBOX_2, MOS_FORCEVDBOX_VDBOXID_BITSNUM, MOS_FORCEVDBOX_MASK, useVD2);
        }

        if (!gtSystemInfo->VDBoxInfo.IsValid ||
            (useVD1 && !gtSystemInfo->VDBoxInfo.Instances.Bits.VDBox0Enabled) ||
            (useVD2 && !gtSystemInfo->VDBoxInfo.Instances.Bits.VDBox1Enabled))
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("the forced VDBOX is not enabled in current platform.");
            return eStatus;
        }

        return eStatus;
    }
#endif

    MOS_STATUS FindGpuNodeToUse(PMHW_VDBOX_GPUNODE_LIMIT gpuNodeLimit) override
    {
        bool       setVideoNode = false;
        MOS_STATUS eStatus      = MOS_STATUS_SUCCESS;

        MOS_GPU_NODE videoGpuNode = MOS_GPU_NODE_VIDEO;

        if (MOS_VE_MULTINODESCALING_SUPPORTED(m_osItf))
        {
            if (GetNumVdbox() == 1)
            {
                videoGpuNode = MOS_GPU_NODE_VIDEO;
            }
            else
            {
                MHW_MI_CHK_STATUS(m_osItf->pfnCreateVideoNodeAssociation(
                    m_osItf,
                    setVideoNode,
                    &videoGpuNode));
            }
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_osItf != nullptr && m_osItf->bEnableDbgOvrdInVE &&
            (!m_osItf->bSupportVirtualEngine || !m_scalabilitySupported))
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("not support DebugOverrid on current OS or Platform.");
            return eStatus;
        }

        if (m_osItf != nullptr && m_osItf->bEnableDbgOvrdInVE)
        {
            MHW_MI_CHK_STATUS(CheckScalabilityOverrideValidity());
        }
#endif

        gpuNodeLimit->dwGpuNodeToUse = videoGpuNode;

        return eStatus;
    }

    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_preDeblockingMemoryCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_postDeblockingMemoryCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_OriginalUncompressedPictureSourceMemoryCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_streamoutDataDestinationMemoryCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_intraRowStoreScratchBufferMemoryCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_deblockingFilterRowStoreScratchMemoryCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_referncePictureMemoryObjectControlStateCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_referncePictureMemoryObjectControlStateCtrlDecode;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_referncePictureMemoryObjectControlStateCtrlEncode;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_macroblockIldbStreamoutBufferCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_secondMacroblockIldbStreamoutBufferCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_slicesizeStreamoutDataDestinationCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_mfxIndirectBitstreamCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_mfdIndirectItCoeffCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_mfxIndirectMvCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_mfcIndirectPakBseCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_bsdMpcRowStoreScratchBufferCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_mprRowStoreScratchBufferCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_bitplaneReadBufferIndexToMemoryCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_directMvBufferForWriteCtrl;

protected:
    using base_t = Itf;
    MhwCpInterface *m_cpItf = nullptr;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC] = {};

    Impl(PMOS_INTERFACE osItf, MhwCpInterface *cpItf) : mhw::Impl(osItf)
    {
        MHW_FUNCTION_ENTER;
        m_cpItf = cpItf;

        InitRowstoreUserFeatureSettings();
        SetCacheabilitySettings();
    }

    virtual ~Impl()
    {
        MHW_FUNCTION_ENTER;

#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_intraRowstoreCache.enabled ||
            m_deblockingFilterRowstoreCache.enabled ||
            m_bsdMpcRowstoreCache.enabled ||
            m_mprRowstoreCache.enabled)
        {
            // Report rowstore cache usage status to regkey
            ReportUserSettingForDebug(
                m_userSettingPtr,
                __MEDIA_USER_FEATURE_VALUE_IS_CODEC_ROW_STORE_CACHE_ENABLED,
                1,
                MediaUserSetting::Group::Device);
        }
#endif

    }

    MOS_STATUS InitRowstoreUserFeatureSettings()
    {
        MHW_FUNCTION_ENTER;

        bool rowstoreCachingDisableDefaultValue = false;
        if (m_osItf->bSimIsActive)
        {
            // Disable RowStore Cache on simulation by default
            rowstoreCachingDisableDefaultValue = true;
        }
        else
        {
            rowstoreCachingDisableDefaultValue = false;
        }
        m_rowstoreCachingSupported = !rowstoreCachingDisableDefaultValue;
#if (_DEBUG || _RELEASE_INTERNAL)
        auto userSettingPtr = m_osItf->pfnGetUserSettingInstance(m_osItf);
        {
            MediaUserSetting::Value outValue;
            ReadUserSettingForDebug(userSettingPtr,
                outValue,
                "Disable RowStore Cache",
                MediaUserSetting::Group::Device,
                rowstoreCachingDisableDefaultValue,
                true);
            m_rowstoreCachingSupported = !(outValue.Get<bool>());
        }
#endif  // _DEBUG || _RELEASE_INTERNAL

        if (m_rowstoreCachingSupported)
        {
            m_intraRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableIntraRowStoreCache",
                    MediaUserSetting::Group::Device);
                m_intraRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif // _DEBUG || _RELEASE_INTERNAL

            m_deblockingFilterRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableDeblockingFilterRowStoreCache",
                    MediaUserSetting::Group::Device);
                m_deblockingFilterRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif // _DEBUG || _RELEASE_INTERNAL

            m_bsdMpcRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableBsdMpcRowStoreCache",
                    MediaUserSetting::Group::Device);
                m_bsdMpcRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif // _DEBUG || _RELEASE_INTERNAL

            m_mprRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableMprRowStoreCache",
                    MediaUserSetting::Group::Device);
                m_mprRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif // _DEBUG || _RELEASE_INTERNAL
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_QM_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_QM_STATE);

        for (uint8_t i = 0; i < 16; i++)
        {
            cmd.ForwardQuantizerMatrix[i] = params.quantizermatrix[i];
        }

        cmd.DW1.Obj2.Avc = params.qmType;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_FQM_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_FQM_STATE);

        for (uint8_t i = 0; i < 32; i++)
        {
            cmd.ForwardQuantizerMatrix[i] = params.quantizermatrix[i];
        }

        cmd.DW1.Obj2.Avc = params.qmType;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(MFX_PIPE_MODE_SELECT);

        MHW_MI_CHK_STATUS(m_cpItf->SetProtectionSettingsForMfxPipeModeSelect((uint32_t *)&cmd));

        #define DO_FIELDS()                                                                                                   \
        DO_FIELD(DW1, StandardSelect, params.standardSelect);                                                                 \
        DO_FIELD(DW1, CodecSelect, params.codecSelect);                                                                       \
        DO_FIELD(DW1, FrameStatisticsStreamoutEnable, params.frameStatisticsStreamoutEnable);                                 \
        DO_FIELD(DW1, ScaledSurfaceEnable, params.scaledSurfaceEnable);                                                       \
        DO_FIELD(DW1, PreDeblockingOutputEnablePredeblockoutenable, params.preDeblockingOutputEnablePredeblockoutenable);     \
        DO_FIELD(DW1, PostDeblockingOutputEnablePostdeblockoutenable, params.postDeblockingOutputEnablePostdeblockoutenable); \
        DO_FIELD(DW1, StreamOutEnable, params.streamOutEnable);                                                               \
        DO_FIELD(DW1, DeblockerStreamOutEnable, params.deblockerStreamOutEnable);                                             \
        DO_FIELD(DW1, VdencMode, params.vdencMode);                                                                           \
        DO_FIELD(DW1, DecoderModeSelect, params.decoderModeSelect);                                                           \
        DO_FIELD(DW1, DecoderShortFormatMode, params.decoderShortFormatMode);                                                 \
        DO_FIELD(DW1, ExtendedStreamOutEnable, params.extendedStreamOutEnable);                                               \
        DO_FIELD(DW2, Vlf720IOddHeightInVc1Mode, params.vlf720IOddHeightInVc1Mode);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_SURFACE_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_SURFACE_STATE);

        #define DO_FIELDS()                                         \
        DO_FIELD(DW1, SurfaceId, params.surfaceId);                 \
        DO_FIELD(DW2, Height, params.height);                       \
        DO_FIELD(DW2, Width,  params.width);                        \
        DO_FIELD(DW3, Tilemode, params.tilemode);                   \
        DO_FIELD(DW3, SurfacePitch, params.surfacePitch);           \
        DO_FIELD(DW3, CompressionFormat, params.compressionFormat); \
        DO_FIELD(DW3, InterleaveChroma, params.interleaveChroma);   \
        DO_FIELD(DW3, SurfaceFormat, params.surfaceFormat);         \
        DO_FIELD(DW4, YOffsetForUCb, params.yOffsetForUCb);         \
        DO_FIELD(DW5, YOffsetForVCr, params.yOffsetForVCr);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_IND_OBJ_BASE_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_IND_OBJ_BASE_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum = MHW_VDBOX_MFX_UPPER_BOUND_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_INDIRECT_OBJ_BASE_ADDR;

        // mode specific settings
        if (CodecHalIsDecodeModeVLD(params.Mode) || (params.Mode == CODECHAL_ENCODE_MODE_VP8))
        {
            MHW_MI_CHK_NULL(params.presDataBuffer);

            InitMocsParams(resourceParams, &cmd.MfxIndirectBitstreamObjectAttributes.DW0.Value, 1, 6);

            resourceParams.presResource                      = params.presDataBuffer;
            resourceParams.dwOffset                          = params.dwDataOffset;
            resourceParams.pdwCmd                            = cmd.MfxIndirectBitstreamObjectBaseAddress.DW0_1.Value;
            resourceParams.dwLocationInCmd                   = 1;
            resourceParams.dwSize                            = params.dwDataSize;
            resourceParams.bIsWritable                       = false;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }
        else if (CodecHalIsDecodeModeIT(params.Mode))
        {
            MHW_MI_CHK_NULL(params.presDataBuffer);
            InitMocsParams(resourceParams, &cmd.MfdIndirectItCoeffObjectAttributes.DW0.Value, 1, 6);

            resourceParams.presResource                      = params.presDataBuffer;
            resourceParams.dwOffset                          = params.dwDataOffset;
            resourceParams.pdwCmd                            = cmd.MfdIndirectItCoeffObjectBaseAddress.DW0_1.Value;
            resourceParams.dwLocationInCmd                   = 11;
            resourceParams.dwSize                            = params.dwDataSize;
            resourceParams.bIsWritable                       = false;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (params.presMvObjectBuffer)
        {
            InitMocsParams(resourceParams, &cmd.MfxIndirectMvObjectAttributes.DW0.Value, 1, 6);
            cmd.MfxIndirectMvObjectAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_mfxIndirectMvCtrl.Gen12_7.Index;

            resourceParams.presResource                      = params.presMvObjectBuffer;
            resourceParams.dwOffset                          = params.dwMvObjectOffset;
            resourceParams.pdwCmd                            = cmd.MfxIndirectMvObjectBaseAddress.DW0_1.Value;
            resourceParams.dwLocationInCmd                   = 6;
            resourceParams.dwSize                            = MOS_ALIGN_CEIL(params.dwMvObjectSize, 0x1000);
            resourceParams.bIsWritable                       = false;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (params.presPakBaseObjectBuffer)
        {
            InitMocsParams(resourceParams, &cmd.MfcIndirectPakBseObjectAttributes.DW0.Value, 1, 6);
            cmd.MfcIndirectPakBseObjectAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_mfcIndirectPakBseCtrl.Gen12_7.Index;

            resourceParams.presResource                      = params.presPakBaseObjectBuffer;
            resourceParams.dwOffset                          = 0;
            resourceParams.pdwCmd                            = cmd.MfcIndirectPakBseObjectBaseAddress.DW0_1.Value;
            resourceParams.dwLocationInCmd                   = 21;
            resourceParams.dwSize                            = MOS_ALIGN_CEIL(params.dwPakBaseObjectSize, 0x1000);
            resourceParams.bIsWritable                       = true;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_BSP_BUF_BASE_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_BSP_BUF_BASE_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum      = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_BSP_BUF_BASE_ADDR;

        if (m_bsdMpcRowstoreCache.enabled)         // mbaff and non mbaff mode for all resolutions
        {
            cmd.DW3.BsdMpcRowStoreScratchBufferCacheSelect          = BUFFER_TO_INTERNALMEDIASTORAGE;
            cmd.DW1.BsdMpcRowStoreScratchBufferBaseAddressReadWrite = m_bsdMpcRowstoreCache.dwAddress;
        }
        else if (params.presBsdMpcRowStoreScratchBuffer)
        {
            InitMocsParams(resourceParams, &cmd.DW3.Value, 1, 6);
            cmd.DW3.BsdMpcRowStoreScratchBufferIndexToMemoryObjectControlStateMocsTables = m_bsdMpcRowStoreScratchBufferCtrl.Gen12_7.Index;
            cmd.DW1.BsdMpcRowStoreScratchBufferBaseAddressReadWrite = 0;

            resourceParams.presResource    = params.presBsdMpcRowStoreScratchBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = &(cmd.DW1.Value);
            resourceParams.dwLocationInCmd = 1;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (m_mprRowstoreCache.enabled)
        {
            cmd.DW6.MprRowStoreScratchBufferCacheSelect                     = BUFFER_TO_INTERNALMEDIASTORAGE;
            cmd.DW4.MprRowStoreScratchBufferBaseAddressReadWriteDecoderOnly = m_mprRowstoreCache.dwAddress;
        }
        else if (params.presMprRowStoreScratchBuffer)
        {
            InitMocsParams(resourceParams, &cmd.DW6.Value, 1, 6);
            cmd.DW4.MprRowStoreScratchBufferBaseAddressReadWriteDecoderOnly = 0;

            resourceParams.presResource    = params.presMprRowStoreScratchBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = &(cmd.DW4.Value);
            resourceParams.dwLocationInCmd = 4;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_PAK_INSERT_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(MFX_PAK_INSERT_OBJECT);

        uint32_t dwordsUsed = cmd.dwSize;

        dwordsUsed += params.dwPadding;

        #define DO_FIELDS()                                                                                                       \
        DO_FIELD(DW0, DwordLength, OP_LENGTH(dwordsUsed));                                                                        \
        DO_FIELD(DW1, BitstreamstartresetResetbitstreamstartingpos, params.bitstreamstartresetResetbitstreamstartingpos);         \
        DO_FIELD(DW1, EndofsliceflagLastdstdatainsertcommandflag, params.endofsliceflagLastdstdatainsertcommandflag);             \
        DO_FIELD(DW1, LastheaderflagLastsrcheaderdatainsertcommandflag, params.lastheaderflagLastsrcheaderdatainsertcommandflag); \
        DO_FIELD(DW1, EmulationflagEmulationbytebitsinsertenable, params.emulationflagEmulationbytebitsinsertenable);             \
        DO_FIELD(DW1, SkipemulbytecntSkipEmulationByteCount, params.skipemulbytecntSkipEmulationByteCount);                       \
        DO_FIELD(DW1, DatabitsinlastdwSrcdataendingbitinclusion50, params.databitsinlastdwSrcdataendingbitinclusion50);           \
        DO_FIELD(DW1, SliceHeaderIndicator, params.sliceHeaderIndicator);                                                         \
        DO_FIELD(DW1, Headerlengthexcludefrmsize, params.headerlengthexcludefrmsize);                                             \
        DO_FIELD(DW1, DatabyteoffsetSrcdatastartingbyteoffset10, 0);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_AVC_IMG_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_AVC_IMG_STATE);

        #define DO_FIELDS()                                                                                               \
        DO_FIELD(DW1, FrameSize, params.frameSize);                                                                       \
        DO_FIELD(DW2, FrameWidth, params.frameWidth);                                                                     \
        DO_FIELD(DW2, FrameHeight, params.frameHeight);                                                                   \
        DO_FIELD(DW3, ImgstructImageStructureImgStructure10, params.imgstructImageStructureImgStructure10);               \
        DO_FIELD(DW3, WeightedBipredIdc, params.weightedBipredIdc);                                                       \
        DO_FIELD(DW3, WeightedPredFlag, params.weightedPredFlag);                                                         \
        DO_FIELD(DW3, RhodomainRateControlEnable, params.vdencEnabled);                                                   \
        DO_FIELD(DW3, FirstChromaQpOffset, params.firstChromaQpOffset);                                                   \
        DO_FIELD(DW3, SecondChromaQpOffset, params.secondChromaQpOffset);                                                 \
        DO_FIELD(DW4, Fieldpicflag, params.fieldpicflag);                                                                 \
        DO_FIELD(DW4, Mbaffflameflag, params.mbaffflameflag);                                                             \
        DO_FIELD(DW4, Framembonlyflag, params.framembonlyflag);                                                           \
        DO_FIELD(DW4, Transform8X8Flag, params.transform8X8Flag);                                                         \
        DO_FIELD(DW4, Direct8X8Infflag, params.direct8X8Infflag);                                                         \
        DO_FIELD(DW4, Constrainedipredflag, params.constrainedipredflag);                                                 \
        DO_FIELD(DW4, Imgdisposableflag, params.imgdisposableflag);                                                       \
        DO_FIELD(DW4, Entropycodingflag, params.entropycodingflag);                                                       \
        DO_FIELD(DW4, Mbmvformatflag, params.mbmvformatflag);                                                             \
        DO_FIELD(DW4, Chromaformatidc, params.chromaformatidc);                                                           \
        DO_FIELD(DW4, Mvunpackedflag, params.mvunpackedflag);                                                             \
        DO_FIELD(DW4, Inserttestflag, 0);                                                                                 \
        DO_FIELD(DW4, Loadslicepointerflag, 0);                                                                           \
        DO_FIELD(DW4, Mbstatenabled, params.mbstatenabled);                                                               \
        DO_FIELD(DW4, Minframewsize, 0);                                                                                  \
        DO_FIELD(DW5, IntrambmaxbitflagIntrambmaxsizereportmask, params.intrambmaxbitflagIntrambmaxsizereportmask);       \
        DO_FIELD(DW5, IntermbmaxbitflagIntermbmaxsizereportmask, params.intermbmaxbitflagIntermbmaxsizereportmask);       \
        DO_FIELD(DW5, FrameszoverflagFramebitratemaxreportmask, params.frameszoverflagFramebitratemaxreportmask);         \
        DO_FIELD(DW5, FrameszunderflagFramebitrateminreportmask, params.frameszunderflagFramebitrateminreportmask);       \
        DO_FIELD(DW5, IntraIntermbipcmflagForceipcmcontrolmask, params.intraIntermbipcmflagForceipcmcontrolmask);         \
        DO_FIELD(DW5, MbratectrlflagMbLevelRateControlEnablingFlag, params.mbratectrlflagMbLevelRateControlEnablingFlag); \
        DO_FIELD(DW5, Nonfirstpassflag, false);                                                                           \
        DO_FIELD(DW5, TrellisQuantizationChromaDisableTqchromadisable, true);                                             \
        DO_FIELD(DW5, TrellisQuantizationRoundingTqr, params.trellisQuantizationRoundingTqr);                             \
        DO_FIELD(DW5, TrellisQuantizationEnabledTqenb, params.trellisQuantizationEnabledTqenb);                           \
        DO_FIELD(DW6, Intrambmaxsz, params.intrambmaxsz);                                                                 \
        DO_FIELD(DW6, Intermbmaxsz, params.intermbmaxsz);                                                                 \
        DO_FIELD(DW8, Slicedeltaqppmax0, 0);                                                                              \
        DO_FIELD(DW8, Slicedeltaqpmax1, 0);                                                                               \
        DO_FIELD(DW8, Slicedeltaqpmax2, 0);                                                                               \
        DO_FIELD(DW8, Slicedeltaqpmax3, 0);                                                                               \
        DO_FIELD(DW9, Slicedeltaqpmin0, 0);                                                                               \
        DO_FIELD(DW9, Slicedeltaqpmin1, 0);                                                                               \
        DO_FIELD(DW9, Slicedeltaqpmin2, 0);                                                                               \
        DO_FIELD(DW9, Slicedeltaqpmin3, 0);                                                                               \
        DO_FIELD(DW10, Framebitratemin, params.framebitratemin);                                                          \
        DO_FIELD(DW10, Framebitrateminunitmode, params.framebitrateminunitmode);                                          \
        DO_FIELD(DW10, Framebitrateminunit, params.framebitrateminunit);                                                  \
        DO_FIELD(DW10, Framebitratemax, params.framebitratemax);                                                          \
        DO_FIELD(DW10, Framebitratemaxunitmode, params.framebitratemaxunitmode);                                          \
        DO_FIELD(DW10, Framebitratemaxunit, params.framebitratemaxunit);                                                  \
        DO_FIELD(DW11, Framebitratemindelta, params.framebitratemindelta);                                                \
        DO_FIELD(DW11, Framebitratemaxdelta, params.framebitratemaxdelta);                                                \
        DO_FIELD(DW11, SliceStatsStreamoutEnable, params.sliceStatsStreamoutEnable);                                      \
        DO_FIELD(DW13, InitialQpValue, params.initialQpValue);                                                            \
        DO_FIELD(DW13, NumberOfActiveReferencePicturesFromL0, params.numberOfActiveReferencePicturesFromL0);              \
        DO_FIELD(DW13, NumberOfActiveReferencePicturesFromL1, params.numberOfActiveReferencePicturesFromL1);              \
        DO_FIELD(DW13, NumberOfReferenceFrames, params.numberOfReferenceFrames);                                          \
        DO_FIELD(DW13, CurrentPictureHasPerformedMmco5, 0);                                                               \
        DO_FIELD(DW14, PicOrderPresentFlag, params.picOrderPresentFlag);                                                  \
        DO_FIELD(DW14, DeltaPicOrderAlwaysZeroFlag, params.deltaPicOrderAlwaysZeroFlag);                                  \
        DO_FIELD(DW14, PicOrderCntType, params.picOrderCntType);                                                          \
        DO_FIELD(DW14, SliceGroupMapType, params.sliceGroupMapType);                                                      \
        DO_FIELD(DW14, RedundantPicCntPresentFlag, params.redundantPicCntPresentFlag);                                    \
        DO_FIELD(DW14, NumSliceGroupsMinus1, params.numSliceGroupsMinus1);                                                \
        DO_FIELD(DW14, DeblockingFilterControlPresentFlag, params.deblockingFilterControlPresentFlag);                    \
        DO_FIELD(DW14, Log2MaxFrameNumMinus4, params.log2MaxFrameNumMinus4);                                              \
        DO_FIELD(DW14, Log2MaxPicOrderCntLsbMinus4, params.log2MaxPicOrderCntLsbMinus4);                                  \
        DO_FIELD(DW15, SliceGroupChangeRate, params.sliceGroupChangeRate);                                                \
        DO_FIELD(DW15, CurrPicFrameNum, params.currPicFrameNum);                                                          \
        DO_FIELD(DW16, CurrentFrameViewId, params.currentFrameViewId);                                                    \
        DO_FIELD(DW16, MaxViewIdxl0, params.maxViewIdxl0);                                                                \
        DO_FIELD(DW16, MaxViewIdxl1, params.maxViewIdxl1);                                                                \
        DO_FIELD(DW16, InterViewOrderDisable, 0);                                                                         \
        DO_FIELD(DW17, ExtendedRhodomainStatisticsEnable, params.extendedRhodomainStatisticsEnable);                      \
        DO_FIELD(DW19, ThresholdSizeInBytes, params.thresholdSizeInBytes);                                                \
        DO_FIELD(DW20, TargetSliceSizeInBytes, params.targetSliceSizeInBytes);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_AVC_REF_IDX_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_AVC_REF_IDX_STATE);

        cmd.DW1.RefpiclistSelect = params.uiList;

        for (uint8_t i = 0; i < 8; i++)
        {
            cmd.ReferenceListEntry[i] = params.referenceListEntry[i];
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_AVC_WEIGHTOFFSET_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_AVC_WEIGHTOFFSET_STATE);

        cmd.DW1.WeightAndOffsetSelect = params.uiList;

        for (uint8_t i = 0; i < 96; i++)
        {
            cmd.Weightoffset[i] = params.weightoffset[i];
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_AVC_SLICE_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_AVC_SLICE_STATE);

        #define DO_FIELDS()                                                                                                     \
        DO_FIELD(DW1, SliceType, params.sliceType);                                                                             \
        DO_FIELD(DW2, Log2WeightDenomLuma, params.log2WeightDenomLuma);                                                         \
        DO_FIELD(DW2, Log2WeightDenomChroma, params.log2WeightDenomChroma);                                                     \
        DO_FIELD(DW2, NumberOfReferencePicturesInInterPredictionList0, params.numberOfReferencePicturesInInterPredictionList0); \
        DO_FIELD(DW2, NumberOfReferencePicturesInInterPredictionList1, params.numberOfReferencePicturesInInterPredictionList1); \
        DO_FIELD(DW3, SliceAlphaC0OffsetDiv2, params.sliceAlphaC0OffsetDiv2);                                                   \
        DO_FIELD(DW3, SliceBetaOffsetDiv2, params.sliceBetaOffsetDiv2);                                                         \
        DO_FIELD(DW3, SliceQuantizationParameter, params.sliceQuantizationParameter);                                           \
        DO_FIELD(DW3, CabacInitIdc10, params.cabacInitIdc10);                                                                   \
        DO_FIELD(DW3, DisableDeblockingFilterIndicator, params.disableDeblockingFilterIndicator);                               \
        DO_FIELD(DW3, DirectPredictionType, params.directPredictionType);                                                       \
        DO_FIELD(DW3, WeightedPredictionIndicator, params.weightedPredictionIndicator);                                         \
        DO_FIELD(DW4, SliceStartMbNum, params.sliceStartMbNum);                                                                 \
        DO_FIELD(DW4, SliceHorizontalPosition, params.sliceHorizontalPosition);                                                 \
        DO_FIELD(DW4, SliceVerticalPosition, params.sliceVerticalPosition);                                                     \
        DO_FIELD(DW5, NextSliceHorizontalPosition, params.nextSliceHorizontalPosition);                                         \
        DO_FIELD(DW5, NextSliceVerticalPosition, params.nextSliceVerticalPosition);                                             \
        DO_FIELD(DW6, StreamId10, 0);                                                                                           \
        DO_FIELD(DW6, SliceId30, params.sliceId30);                                                                             \
        DO_FIELD(DW6, Cabaczerowordinsertionenable, params.cabaczerowordinsertionenable);                                       \
        DO_FIELD(DW6, Emulationbytesliceinsertenable, params.emulationbytesliceinsertenable);                                   \
        DO_FIELD(DW6, TailInsertionPresentInBitstream, params.tailInsertionPresentInBitstream);                                 \
        DO_FIELD(DW6, SlicedataInsertionPresentInBitstream, params.slicedataInsertionPresentInBitstream);                       \
        DO_FIELD(DW6, HeaderInsertionPresentInBitstream, params.headerInsertionPresentInBitstream);                             \
        DO_FIELD(DW6, IsLastSlice, params.isLastSlice);                                                                         \
        DO_FIELD(DW6, MbTypeSkipConversionDisable, false);                                                                      \
        DO_FIELD(DW6, MbTypeDirectConversionDisable, false);                                                                    \
        DO_FIELD(DW6, RateControlCounterEnable, false);                                                                         \
        DO_FIELD(DW9, Roundintra, params.roundintra);                                                                           \
        DO_FIELD(DW9, Roundintraenable, true);                                                                                  \
        DO_FIELD(DW9, Roundinter, params.roundinter);                                                                           \
        DO_FIELD(DW9, Roundinterenable, params.roundinterenable);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_AVC_DIRECTMODE_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_AVC_DIRECTMODE_STATE);

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum      = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_AVC_DIRECT_MODE;

        if (!params.bDisableDmvBuffers)
        {
            MHW_MI_CHK_NULL(params.presAvcDmvBuffers);
            InitMocsParams(resourceParams, &cmd.DirectMvBufferForWriteAttributes.DW0.Value, 1, 6);

            // current picture
            resourceParams.presResource    = &params.presAvcDmvBuffers[params.ucAvcDmvIdx];
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = &(cmd.DirectMvBufferForWriteBaseAddress.DW0_1.Value[0]);
            resourceParams.dwLocationInCmd = 34;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        CODEC_REF_LIST** refList;
        MHW_MI_CHK_NULL(refList = (CODEC_REF_LIST**)params.avcRefList);

        if (CodecHal_PictureIsBottomField(params.CurrPic))
        {
            cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_TOP]    = 0;
            cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_BOTTOM] = refList[params.CurrPic.FrameIdx]->iFieldOrderCnt[1];
        }
        else
        {
            cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_TOP] = cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_BOTTOM] =
                refList[params.CurrPic.FrameIdx]->iFieldOrderCnt[0];
            if (CodecHal_PictureIsFrame(params.CurrPic))
            {
                cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_BOTTOM] = refList[params.CurrPic.FrameIdx]->iFieldOrderCnt[1];
            }
        }

        if (!params.bDisableDmvBuffers)
        {
            InitMocsParams(resourceParams, &cmd.DirectMvBufferAttributes.DW0.Value, 1, 6);
        }

        bool dmvPresent[CODEC_MAX_NUM_REF_FRAME] = { false };
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
        {
            if (params.pAvcPicIdx[i].bValid)
            {
                uint8_t idx = params.pAvcPicIdx[i].ucPicIdx;
                uint8_t picID = params.bPicIdRemappingInUse ? i : refList[idx]->ucFrameId;
                uint8_t mvIdx = refList[idx]->ucDMVIdx[0];

                uint8_t validRef = ((params.uiUsedForReferenceFlags >> (i * 2)) >> 0) & 1;
                uint8_t frameID = picID << 1;
                if (frameID < CODEC_AVC_NUM_REF_DMV_BUFFERS * 2)
                {
                    if (!params.bDisableDmvBuffers)
                    {
                        dmvPresent[picID] = true;

                        resourceParams.presResource       = &params.presAvcDmvBuffers[mvIdx];
                        resourceParams.dwOffset           = 0;
                        resourceParams.pdwCmd             = &(cmd.DirectMvBufferBaseAddress[picID].DW0_1.Value[0]);
                        resourceParams.dwLocationInCmd    = picID * 2 + 1;
                        resourceParams.bIsWritable        = false;

                        resourceParams.dwSharedMocsOffset = 33 - resourceParams.dwLocationInCmd;

                        MHW_MI_CHK_STATUS(AddResourceToCmd(
                            this->m_osItf,
                            this->m_currentCmdBuf,
                            &resourceParams));
                    }

                    cmd.PocList[frameID] = refList[idx]->iFieldOrderCnt[0] * validRef;
                }
                else
                {
                    return MOS_STATUS_UNKNOWN;
                }

                validRef = ((params.uiUsedForReferenceFlags >> (i * 2)) >> 1) & 1;
                frameID = (picID << 1) + 1;
                if (frameID < CODEC_AVC_NUM_REF_DMV_BUFFERS * 2)
                {
                    cmd.PocList[frameID] = refList[idx]->iFieldOrderCnt[1] * validRef;
                }
                else
                {
                    return MOS_STATUS_UNKNOWN;
                }
            }
        }

        if (!params.bDisableDmvBuffers)
        {
            // Use a valid address for remaining DMV buffers
            for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
            {
                if (dmvPresent[i] == false)
                {
                    //Give default buffer to the MV
                    resourceParams.presResource       = &params.presAvcDmvBuffers[CODEC_AVC_NUM_REF_DMV_BUFFERS];
                    resourceParams.dwOffset           = 0;
                    resourceParams.pdwCmd             = &(cmd.DirectMvBufferBaseAddress[i].DW0_1.Value[0]);
                    resourceParams.dwLocationInCmd    = i * 2 + 1;
                    resourceParams.bIsWritable        = false;

                    resourceParams.dwSharedMocsOffset = 33 - resourceParams.dwLocationInCmd;

                    MHW_MI_CHK_STATUS(AddResourceToCmd(
                        this->m_osItf,
                        this->m_currentCmdBuf,
                        &resourceParams));
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFD_AVC_SLICEADDR)
    {
        _MHW_SETCMD_CALLBASE(MFD_AVC_SLICEADDR);

        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.presDataBuffer       = params.presDataBuffer;
        sliceInfoParam.dwSliceIndex         = params.dwSliceIndex;
        sliceInfoParam.dwTotalBytesConsumed = params.dwTotalBytesConsumed;
        sliceInfoParam.dwDataStartOffset[0] = params.IndirectBsdDataStartAddress;
        sliceInfoParam.dwDataStartOffset[1] = params.avcSliceParams->slice_data_offset;

        MHW_MI_CHK_STATUS(m_cpItf->SetMfxProtectionState(
            params.decodeInUse,
            this->m_currentCmdBuf,
            nullptr,
            &sliceInfoParam));
        #define DO_FIELDS()                                                     \
            DO_FIELD(DW1, IndirectBsdDataLength, params.IndirectBsdDataLength); \
            DO_FIELD(DW2, IndirectBsdDataStartAddress, params.IndirectBsdDataStartAddress);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFD_AVC_BSD_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(MFD_AVC_BSD_OBJECT);
        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.presDataBuffer       = params.presDataBuffer;
        sliceInfoParam.dwSliceIndex         = params.dwSliceIndex;
        sliceInfoParam.dwTotalBytesConsumed = params.dwTotalBytesConsumed;
        sliceInfoParam.dwDataStartOffset[0] = params.IndirectBsdDataStartAddress;
        sliceInfoParam.dwDataStartOffset[1] = params.pAvcSliceParams->slice_data_offset;
        sliceInfoParam.dwDataLength[1]      = params.pAvcSliceParams->slice_data_size;
        MHW_MI_CHK_STATUS(m_cpItf->SetMfxProtectionState(
            params.decodeInUse,
            this->m_currentCmdBuf,
            nullptr,
            &sliceInfoParam));
        #define DO_FIELDS()                                                                                             \
            DO_FIELD(DW4, LastsliceFlag, params.LastsliceFlag);                                                         \
            DO_FIELD(DW3, IntraPredmode4X48X8LumaErrorControlBit, 1);                                                   \
            DO_FIELD(DW5, IntraPredictionErrorControlBitAppliedToIntra16X16Intra8X8Intra4X4LumaAndChroma, 1);           \
            DO_FIELD(DW5, Intra8X84X4PredictionErrorConcealmentControlBit, 1);                                          \
            DO_FIELD(DW5, ISliceConcealmentMode, 1);                                                                    \
            DO_FIELD(DW1, IndirectBsdDataLength, params.IndirectBsdDataLength);                                         \
            DO_FIELD(DW2, IndirectBsdDataStartAddress, params.IndirectBsdDataStartAddress);                             \
            DO_FIELD(DW4, FirstMbByteOffsetOfSliceDataOrSliceHeader, params.FirstMbByteOffsetOfSliceDataOrSliceHeader); \
            DO_FIELD(DW4, FirstMacroblockMbBitOffset, params.FirstMacroblockMbBitOffset);                               \
            DO_FIELD(DW4, FixPrevMbSkipped, 1);
#include "mhw_hwcmd_process_cmdfields.h"
    }

  _MHW_SETCMD_OVERRIDE_DECL(MFD_AVC_PICID_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFD_AVC_PICID_STATE);
        MOS_SecureMemcpy(cmd.Pictureidlist1616Bits,
            sizeof(params.Pictureidlist1616Bits),
            params.Pictureidlist1616Bits,
            sizeof(params.Pictureidlist1616Bits));
        #define DO_FIELDS() \
            DO_FIELD(DW1, PictureidRemappingDisable, params.PictureidRemappingDisable);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_JPEG_PIC_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_JPEG_PIC_STATE);

        #define DO_FIELDS()                                                                        \
        if (params.decodeInUse)                                                                    \
        {                                                                                          \
            DO_FIELD(DW1, Obj1.InputFormatYuv, params.inputFormatYuv);                             \
            DO_FIELD(DW1, Obj1.Rotation, params.rotation);                                         \
            DO_FIELD(DW1, Obj1.OutputFormatYuv, params.outputFormatYuv);                           \
            DO_FIELD(DW1, Obj1.VerticalDownSamplingEnable, params.verticalDownSamplingEnable);     \
            DO_FIELD(DW1, Obj1.HorizontalDownSamplingEnable, params.horizontalDownSamplingEnable); \
            DO_FIELD(DW1, Obj1.VerticalUpSamplingEnable, params.verticalUpSamplingEnable);         \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            DO_FIELD(DW1, Obj0.OutputMcuStructure, params.outputMcuStructure);                     \
            DO_FIELD(DW1, Obj0.InputSurfaceFormatYuv, params.inputSurfaceFormatYuv);               \
            DO_FIELD(DW1, Obj0.PixelsInVerticalLastMcu, params.pixelsInVerticalLastMcu);           \
            DO_FIELD(DW1, Obj0.PixelsInHorizontalLastMcu, params.pixelsInHorizontalLastMcu);       \
        }                                                                                          \
        DO_FIELD(DW2, Obj0.FrameWidthInBlocksMinus1, params.frameWidthInBlocksMinus1);             \
        DO_FIELD(DW2, Obj0.FrameHeightInBlocksMinus1, params.frameHeightInBlocksMinus1);
#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_JPEG_HUFF_TABLE_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_JPEG_HUFF_TABLE_STATE);

        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            cmd.DcBits128BitArray,
            sizeof(cmd.DcBits128BitArray),
            params.pDCBits,
            sizeof(cmd.DcBits128BitArray)));
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            cmd.DcHuffval128BitArray,
            sizeof(cmd.DcHuffval128BitArray),
            params.pDCValues,
            sizeof(cmd.DcHuffval128BitArray)));
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            cmd.AcBits168BitArray,
            sizeof(cmd.AcBits168BitArray),
            params.pACBits,
            sizeof(cmd.AcBits168BitArray)));
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            cmd.AcHuffval1608BitArray,
            sizeof(cmd.AcHuffval1608BitArray),
            params.pACValues,
            sizeof(cmd.AcHuffval1608BitArray)));

        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            &cmd.DW52.Value,
            sizeof(uint16_t),
            (uint8_t *)params.pACValues + sizeof(cmd.AcHuffval1608BitArray),
            sizeof(uint16_t)));

#define DO_FIELDS() \
    DO_FIELD(DW1, Hufftableid1Bit, params.huffTableID);

#include "mhw_hwcmd_process_cmdfields.h"
    }

   _MHW_SETCMD_OVERRIDE_DECL(MFD_JPEG_BSD_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(MFD_JPEG_BSD_OBJECT);

#define DO_FIELDS()                                                         \
    DO_FIELD(DW1, IndirectDataLength, params.indirectDataLength);           \
    DO_FIELD(DW2, IndirectDataStartAddress, params.dataStartAddress);       \
    DO_FIELD(DW3, ScanVerticalPosition, params.scanVerticalPosition);       \
    DO_FIELD(DW3, ScanHorizontalPosition, params.scanHorizontalPosition);   \
    DO_FIELD(DW4, McuCount, params.mcuCount);                               \
    DO_FIELD(DW4, ScanComponents, params.scanComponent);                    \
    DO_FIELD(DW4, Interleaved, params.interleaved);                         \
    DO_FIELD(DW5, Restartinterval16Bit, params.restartInterval);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFC_JPEG_HUFF_TABLE_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFC_JPEG_HUFF_TABLE_STATE);

        for (uint8_t i = 0; i < 12; i++)
        {
            cmd.DcTable[i] = params.dcTable[i];
        }

        for (uint8_t i = 0; i < 162; i++)
        {
            cmd.AcTable[i] = params.acTable[i];
        }

        cmd.DW1.HuffTableId = params.huffTableId;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFC_JPEG_SCAN_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(MFC_JPEG_SCAN_OBJECT);

        #define DO_FIELDS()                                     \
        DO_FIELD(DW1, McuCount, params.mcuCount);               \
        DO_FIELD(DW2, RestartInterval, params.restartInterval); \
        DO_FIELD(DW2, IsLastScan, 1);                           \
        DO_FIELD(DW2, HeadPresentFlag, 1);                      \
        DO_FIELD(DW2, HuffmanDcTable, params.huffmanDcTable);   \
        DO_FIELD(DW2, HuffmanAcTable, params.huffmanAcTable);

#include "mhw_hwcmd_process_cmdfields.h"
    }

   _MHW_SETCMD_OVERRIDE_DECL(MFD_AVC_DPB_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFD_AVC_DPB_STATE);
        for (auto i = 0, j = 0; i < 8; i++, j++)
        {
            cmd.Ltstframenumlist1616Bits[i] = (params.refFrameOrder[j++] & 0xFFFF);
            cmd.Ltstframenumlist1616Bits[i] = cmd.Ltstframenumlist1616Bits[i] | ((params.refFrameOrder[j] & 0xFFFF) << 16);  //SecondEntry
        }
        if (params.pMvcExtPicParams)
        {
            for (auto i = 0, j = 0; j < (CODEC_MAX_NUM_REF_FRAME / 2); i++, j++)
            {
                cmd.Viewidlist1616Bits[i] = params.pMvcExtPicParams->ViewIDList[j++];
                cmd.Viewidlist1616Bits[i] = cmd.Viewidlist1616Bits[i] | (params.pMvcExtPicParams->ViewIDList[j] << 16);
            }
            for (auto i = 0, j = 0; i < (CODEC_MAX_NUM_REF_FRAME / 4); i++, j++)
            {
                cmd.Vieworderlistl0168Bits[i] = GetViewOrder(params, j++, LIST_0);                                          //FirstEntry
                cmd.Vieworderlistl0168Bits[i] = cmd.Vieworderlistl0168Bits[i] | (GetViewOrder(params, j++, LIST_0) << 8);   //SecondEntry
                cmd.Vieworderlistl0168Bits[i] = cmd.Vieworderlistl0168Bits[i] | (GetViewOrder(params, j++, LIST_0) << 16);  //ThirdEntry
                cmd.Vieworderlistl0168Bits[i] = cmd.Vieworderlistl0168Bits[i] | (GetViewOrder(params, j, LIST_0) << 24);    //FourthEntry
            }
            for (auto i = 0, j = 0; i < (CODEC_MAX_NUM_REF_FRAME / 4); i++, j++)
            {
                cmd.Vieworderlistl1168Bits[i] = GetViewOrder(params, j++, LIST_1);                                          //FirstEntry
                cmd.Vieworderlistl1168Bits[i] = cmd.Vieworderlistl1168Bits[i] | (GetViewOrder(params, j++, LIST_1) << 8);   //SecondEntry
                cmd.Vieworderlistl1168Bits[i] = cmd.Vieworderlistl1168Bits[i] | (GetViewOrder(params, j++, LIST_1) << 16);  //ThirdEntry
                cmd.Vieworderlistl1168Bits[i] = cmd.Vieworderlistl1168Bits[i] | (GetViewOrder(params, j, LIST_1) << 24);    //FourthEntry
            }
        }
        else
        {
            // non-MVC usage
            MOS_ZeroMemory(cmd.Viewidlist1616Bits, sizeof(cmd.Viewidlist1616Bits));
            MOS_FillMemory(cmd.Vieworderlistl0168Bits, sizeof(cmd.Vieworderlistl0168Bits), 0xF);
            MOS_FillMemory(cmd.Vieworderlistl1168Bits, sizeof(cmd.Vieworderlistl1168Bits), 0xF);
        }

        #define DO_FIELDS()                                                               \
            DO_FIELD(DW1, NonExistingframeFlag161Bit, params.NonExistingframeFlag161Bit); \
            DO_FIELD(DW1, LongtermframeFlag161Bit, params.LongtermframeFlag161Bit);       \
            DO_FIELD(DW2, Value, params.usedForRef);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_MPEG2_PIC_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_MPEG2_PIC_STATE);

        #define DO_FIELDS()                                                                                                                                           \
            DO_FIELD(DW1, ScanOrder, params.ScanOrder);                                                                                                               \
            DO_FIELD(DW1, IntraVlcFormat, params.IntraVlcFormat);                                                                                                     \
            DO_FIELD(DW1, QuantizerScaleType, params.QuantizerScaleType);                                                                                             \
            DO_FIELD(DW1, ConcealmentMotionVectorFlag, params.ConcealmentMotionVectorFlag);                                                                           \
            DO_FIELD(DW1, FramePredictionFrameDct, params.FramePredictionFrameDct);                                                                                   \
            DO_FIELD(DW1, TffTopFieldFirst, params.TffTopFieldFirst);                                                                                                 \
            DO_FIELD(DW1, PictureStructure, params.PictureStructure);                                                                                                 \
            DO_FIELD(DW1, IntraDcPrecision, params.IntraDcPrecision);                                                                                                 \
            DO_FIELD(DW1, FCode00, params.FCode00);                                                                                                                   \
            DO_FIELD(DW1, FCode01, params.FCode01);                                                                                                                   \
            DO_FIELD(DW1, FCode10, params.FCode10);                                                                                                                   \
            DO_FIELD(DW1, FCode11, params.FCode11);                                                                                                                   \
            DO_FIELD(DW2, PictureCodingType, params.PictureCodingType);                                                                                               \
            DO_FIELD(DW2, ISliceConcealmentMode, params.ISliceConcealmentMode);                                                                                       \
            DO_FIELD(DW2, PBSliceConcealmentMode, params.PBSliceConcealmentMode);                                                                                     \
            DO_FIELD(DW2, PBSlicePredictedBidirMotionTypeOverrideBiDirectionMvTypeOverride, params.PBSlicePredictedBidirMotionTypeOverrideBiDirectionMvTypeOverride); \
            DO_FIELD(DW2, PBSlicePredictedMotionVectorOverrideFinalMvValueOverride, params.PBSlicePredictedMotionVectorOverrideFinalMvValueOverride);                 \
            DO_FIELD(DW3, SliceConcealmentDisableBit, params.SliceConcealmentDisableBit);                                                                             \
            DO_FIELD(DW3, Framewidthinmbsminus170PictureWidthInMacroblocks, params.Framewidthinmbsminus170PictureWidthInMacroblocks);                                 \
            DO_FIELD(DW3, Frameheightinmbsminus170PictureHeightInMacroblocks, params.Frameheightinmbsminus170PictureHeightInMacroblocks);                             \
            DO_FIELD(DW4, Roundintradc, 3);                                                                                                                           \
            DO_FIELD(DW4, Roundinterdc, 1);                                                                                                                           \
            DO_FIELD(DW4, Roundintraac, 5);                                                                                                                           \
            DO_FIELD(DW4, Roundinterac, 1);                                                                                                                           \
            DO_FIELD(DW3, MFX_MPEG2_PIC_STATE_CMD_DW3_BIT24_28, params.mfxMpeg2PicStatePar0);                                                                         \

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFD_MPEG2_BSD_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(MFD_MPEG2_BSD_OBJECT);

        #define DO_FIELDS()                                                                 \
            DO_FIELD(DW1, IndirectBsdDataLength, params.IndirectBsdDataLength);             \
            DO_FIELD(DW2, IndirectDataStartAddress, params.IndirectDataStartAddress);       \
            DO_FIELD(DW3, FirstMacroblockBitOffset, params.FirstMacroblockBitOffset);       \
            DO_FIELD(DW3, IsLastMb, params.IsLastMb);                                       \
            DO_FIELD(DW3, LastPicSlice, params.LastPicSlice);                               \
            DO_FIELD(DW3, MbRowLastSlice, params.MbRowLastSlice);                           \
            DO_FIELD(DW3, MacroblockCount, params.MacroblockCount);                         \
            DO_FIELD(DW3, SliceHorizontalPosition, params.SliceHorizontalPosition);         \
            DO_FIELD(DW3, SliceVerticalPosition, params.SliceVerticalPosition);             \
            DO_FIELD(DW4, QuantizerScaleCode, params.QuantizerScaleCode);                   \
            DO_FIELD(DW4, NextSliceHorizontalPosition, params.NextSliceHorizontalPosition); \
            DO_FIELD(DW4, NextSliceVerticalPosition, params.NextSliceVerticalPosition);

        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.presDataBuffer       = params.presDataBuffer;
        sliceInfoParam.dwDataStartOffset[0] = params.dwDataStartOffset;

        MHW_MI_CHK_STATUS(m_cpItf->SetMfxProtectionState(
            params.decodeInUse,
            nullptr,
            this->m_currentBatchBuf,
            &sliceInfoParam));

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFD_IT_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(MFD_IT_OBJECT);

    #define DO_FIELDS()                                                      \
        DO_FIELD(DW0, DwordLength, params.DwordLength);                      \
        DO_FIELD(DW3, IndirectItCoeffDataLength, (params.dwDCTLength) << 2); \
        DO_FIELD(DW4, IndirectItCoeffDataStartAddressOffset, params.IndirectItCoeffDataStartAddressOffset);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFD_IT_OBJECT_MPEG2_INLINE_DATA)
    {
        _MHW_SETCMD_CALLBASE(MFD_IT_OBJECT_MPEG2_INLINE_DATA);

        auto MBType = params.pMBParams->MBType;
        
        cmd.DW0.MacroblockIntraType = 1;
        cmd.DW0.DctType             = MBType.m_fieldResidual;
        cmd.DW0.CodedBlockPattern   = params.CodedBlockPattern;
        cmd.DW0.Lastmbinrow         = params.Lastmbinrow;
        cmd.DW1.Horzorigin          = params.Horzorigin;
        cmd.DW1.Vertorigin          = params.Vertorigin;

        if (params.CodingType != I_TYPE)
        {
            cmd.DW0.MacroblockIntraType       = MBType.m_intraMb;
            cmd.DW0.MacroblockMotionForward   = MBType.m_motionFwd;
            cmd.DW0.MacroblockMotionBackward  = MBType.m_motionBwd;
            cmd.DW0.MotionType                = MBType.m_motionType;
            cmd.DW0.MotionVerticalFieldSelect = MBType.m_mvertFieldSel;

            // Next, copy in the motion vectors
            if (MBType.m_intraMb == 0)
            {
                uint32_t *point = (uint32_t *)(params.sPackedMVs0);
                cmd.DW2.Value   = *point++;
                cmd.DW3.Value   = *point++;

                point = (uint32_t *)(params.sPackedMVs1);
                cmd.DW4.Value = *point++;
                cmd.DW5.Value = *point++;
            }
        }

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_VP8_PIC_STATE)
    {
        _MHW_SETCMD_CALLBASE(MFX_VP8_PIC_STATE);

        #define DO_FIELDS()                                                                                         \
            DO_FIELD(DW1, FrameWidthMinus1                   , params.FrameWidthMinus1);                            \
            DO_FIELD(DW1, FrameHeightMinus1                  , params.FrameHeightMinus1);                           \
            DO_FIELD(DW2, McFilterSelect , params.McFilterSelect);                                                  \
            DO_FIELD(DW2, ChromaFullPixelMcFilterMode , params.ChromaFullPixelMcFilterMode);                        \
            DO_FIELD(DW2, Dblkfiltertype , params.Dblkfiltertype);                                                  \
            DO_FIELD(DW2, Skeyframeflag , params.Skeyframeflag);                                                    \
            DO_FIELD(DW2, SegmentationIdStreamoutEnable , params.SegmentationIdStreamoutEnable);                    \
            DO_FIELD(DW2, SegmentationIdStreaminEnable , params.SegmentationIdStreaminEnable);                      \
            DO_FIELD(DW2, SegmentEnableFlag , params.SegmentEnableFlag);                                            \
            DO_FIELD(DW2, UpdateMbsegmentMapFlag , params.UpdateMbsegmentMapFlag);                                  \
            DO_FIELD(DW2, MbNocoeffSkipflag , params.MbNocoeffSkipflag);                                            \
            DO_FIELD(DW2, ModeReferenceLoopFilterDeltaEnabled , params.ModeReferenceLoopFilterDeltaEnabled);        \
            DO_FIELD(DW2, GoldenRefPictureMvSignbiasFlag , params.GoldenRefPictureMvSignbiasFlag);                  \
            DO_FIELD(DW2, AlternateRefPicMvSignbiasFlag , params.AlternateRefPicMvSignbiasFlag);                    \
            DO_FIELD(DW2, DeblockSharpnessLevel , params.DeblockSharpnessLevel);                                    \
            DO_FIELD(DW3, DblkfilterlevelForSegment3 , params.DblkfilterlevelForSegment3);                          \
            DO_FIELD(DW3, DblkfilterlevelForSegment2 , params.DblkfilterlevelForSegment2);                          \
            DO_FIELD(DW3, DblkfilterlevelForSegment1 , params.DblkfilterlevelForSegment1);                          \
            DO_FIELD(DW3, DblkfilterlevelForSegment0 , params.DblkfilterlevelForSegment0);                          \
            DO_FIELD(DW8, QuantizerValue1Blocktype2Uvdc , params.QuantizerValue1Blocktype2Uvdc);                    \
            DO_FIELD(DW8, QuantizerValue1Blocktype3Uvac , params.QuantizerValue1Blocktype3Uvac);                    \
            DO_FIELD(DW9, QuantizerValue1Blocktype4Y2Dc , params.QuantizerValue1Blocktype4Y2Dc);                    \
            DO_FIELD(DW9, QuantizerValue1Blocktype5Y2Ac , params.QuantizerValue1Blocktype5Y2Ac);                    \
            DO_FIELD(DW10, QuantizerValue2Blocktype0Y1Dc , params.QuantizerValue2Blocktype0Y1Dc);                   \
            DO_FIELD(DW10, QuantizerValue2Blocktype1Y1Ac , params.QuantizerValue2Blocktype1Y1Ac);                   \
            DO_FIELD(DW11, QuantizerValue2Blocktype2Uvdc , params.QuantizerValue2Blocktype2Uvdc);                   \
            DO_FIELD(DW11, QuantizerValue2Blocktype3Uvac , params.QuantizerValue2Blocktype3Uvac);                   \
            DO_FIELD(DW12, QuantizerValue2Blocktype4Y2Dc , params.QuantizerValue2Blocktype4Y2Dc);                   \
            DO_FIELD(DW12, QuantizerValue2Blocktype5Y2Ac , params.QuantizerValue2Blocktype5Y2Ac);                   \
            DO_FIELD(DW13, QuantizerValue3Blocktype0Y1Dc , params.QuantizerValue3Blocktype0Y1Dc);                   \
            DO_FIELD(DW13, QuantizerValue3Blocktype1Y1Ac , params.QuantizerValue3Blocktype1Y1Ac);                   \
            DO_FIELD(DW14, QuantizerValue3Blocktype2Uvdc , params.QuantizerValue3Blocktype2Uvdc);                   \
            DO_FIELD(DW14, QuantizerValue3Blocktype3Uvac , params.QuantizerValue3Blocktype3Uvac);                   \
            DO_FIELD(DW15, QuantizerValue3Blocktype4Y2Dc , params.QuantizerValue3Blocktype4Y2Dc);                   \
            DO_FIELD(DW15, QuantizerValue3Blocktype5Y2Ac , params.QuantizerValue3Blocktype5Y2Ac);                   \
            DO_FIELD(DW19, Mbsegmentidtreeprobs2 , params.Mbsegmentidtreeprobs2);                                   \
            DO_FIELD(DW19, Mbsegmentidtreeprobs1 , params.Mbsegmentidtreeprobs1);                                   \
            DO_FIELD(DW19, Mbsegmentidtreeprobs0 , params.Mbsegmentidtreeprobs0);                                   \
            DO_FIELD(DW20, Mbnocoeffskipfalseprob , params.Mbnocoeffskipfalseprob);                                 \
            DO_FIELD(DW20, Intrambprob , params.Intrambprob);                                                       \
            DO_FIELD(DW20, Interpredfromlastrefprob , params.Interpredfromlastrefprob);                             \
            DO_FIELD(DW20, Interpredfromgrefrefprob , params.Interpredfromgrefrefprob);                             \
            DO_FIELD(DW21, Ymodeprob3 , params.Ymodeprob3);                                                         \
            DO_FIELD(DW21, Ymodeprob2 , params.Ymodeprob2);                                                         \
            DO_FIELD(DW21, Ymodeprob1 , params.Ymodeprob1);                                                         \
            DO_FIELD(DW21, Ymodeprob0 , params.Ymodeprob0);                                                         \
            DO_FIELD(DW22, Uvmodeprob2 , params.Uvmodeprob2);                                                       \
            DO_FIELD(DW22, Uvmodeprob1 , params.Uvmodeprob1);                                                       \
            DO_FIELD(DW22, Uvmodeprob0 , params.Uvmodeprob0);                                                       \
            DO_FIELD(DW23, Mvupdateprobs00 , params.Mvupdateprobs00);                                               \
            DO_FIELD(DW23, Mvupdateprobs01 , params.Mvupdateprobs01);                                               \
            DO_FIELD(DW23, Mvupdateprobs02 , params.Mvupdateprobs02);                                               \
            DO_FIELD(DW23, Mvupdateprobs03 , params.Mvupdateprobs03);                                               \
            DO_FIELD(DW24, Mvupdateprobs04 , params.Mvupdateprobs04);                                               \
            DO_FIELD(DW24, Mvupdateprobs05 , params.Mvupdateprobs05);                                               \
            DO_FIELD(DW24, Mvupdateprobs06 , params.Mvupdateprobs06);                                               \
            DO_FIELD(DW24, Mvupdateprobs07 , params.Mvupdateprobs07);                                               \
            DO_FIELD(DW25, Mvupdateprobs08 , params.Mvupdateprobs08);                                               \
            DO_FIELD(DW25, Mvupdateprobs09 , params.Mvupdateprobs09);                                               \
            DO_FIELD(DW25, Mvupdateprobs010 , params.Mvupdateprobs010);                                             \
            DO_FIELD(DW25, Mvupdateprobs011 , params.Mvupdateprobs011);                                             \
            DO_FIELD(DW26, Mvupdateprobs012 , params.Mvupdateprobs012);                                             \
            DO_FIELD(DW26, Mvupdateprobs013 , params.Mvupdateprobs013);                                             \
            DO_FIELD(DW26, Mvupdateprobs014 , params.Mvupdateprobs014);                                             \
            DO_FIELD(DW26, Mvupdateprobs015 , params.Mvupdateprobs015);                                             \
            DO_FIELD(DW27, Mvupdateprobs016 , params.Mvupdateprobs016);                                             \
            DO_FIELD(DW27, Mvupdateprobs017 , params.Mvupdateprobs017);                                             \
            DO_FIELD(DW27, Mvupdateprobs018 , params.Mvupdateprobs018);                                             \
            DO_FIELD(DW28, Mvupdateprobs10 , params.Mvupdateprobs10);                                               \
            DO_FIELD(DW28, Mvupdateprobs11 , params.Mvupdateprobs11);                                               \
            DO_FIELD(DW28, Mvupdateprobs12 , params.Mvupdateprobs12);                                               \
            DO_FIELD(DW28, Mvupdateprobs13 , params.Mvupdateprobs13);                                               \
            DO_FIELD(DW29, Mvupdateprobs14 , params.Mvupdateprobs14);                                               \
            DO_FIELD(DW29, Mvupdateprobs15 , params.Mvupdateprobs15);                                               \
            DO_FIELD(DW29, Mvupdateprobs16 , params.Mvupdateprobs16);                                               \
            DO_FIELD(DW29, Mvupdateprobs17 , params.Mvupdateprobs17);                                               \
            DO_FIELD(DW30, Mvupdateprobs18 , params.Mvupdateprobs18);                                               \
            DO_FIELD(DW30, Mvupdateprobs19 , params.Mvupdateprobs19);                                               \
            DO_FIELD(DW30, Mvupdateprobs110 , params.Mvupdateprobs110);                                             \
            DO_FIELD(DW30, Mvupdateprobs111 , params.Mvupdateprobs111);                                             \
            DO_FIELD(DW31, Mvupdateprobs112 , params.Mvupdateprobs112);                                             \
            DO_FIELD(DW31, Mvupdateprobs113 , params.Mvupdateprobs113);                                             \
            DO_FIELD(DW31, Mvupdateprobs114 , params.Mvupdateprobs114);                                             \
            DO_FIELD(DW31, Mvupdateprobs115 , params.Mvupdateprobs115);                                             \
            DO_FIELD(DW32, Mvupdateprobs116 , params.Mvupdateprobs116);                                             \
            DO_FIELD(DW32, Mvupdateprobs117 , params.Mvupdateprobs117);                                             \
            DO_FIELD(DW32, Mvupdateprobs118 , params.Mvupdateprobs118);                                             \
            DO_FIELD(DW33, Reflfdelta0ForIntraFrame , params.Reflfdelta0ForIntraFrame);                             \
            DO_FIELD(DW33, Reflfdelta1ForLastFrame , params.Reflfdelta1ForLastFrame);                               \
            DO_FIELD(DW33, Reflfdelta2ForGoldenFrame , params.Reflfdelta2ForGoldenFrame);                           \
            DO_FIELD(DW33, Reflfdelta3ForAltrefFrame , params.Reflfdelta3ForAltrefFrame);                           \
            DO_FIELD(DW34, Modelfdelta0ForBPredMode , params.Modelfdelta0ForBPredMode);                             \
            DO_FIELD(DW34, Modelfdelta1ForZeromvMode , params.Modelfdelta1ForZeromvMode);                           \
            DO_FIELD(DW34, Modelfdelta2ForNearestNearAndNewMode , params.Modelfdelta2ForNearestNearAndNewMode);     \
            DO_FIELD(DW34, Modelfdelta3ForSplitmvMode , params.Modelfdelta3ForSplitmvMode);


            cmd.DW4.dec.QuantizerValue0Blocktype0Y1Dc = params.QuantizerValue0Blocktype0Y1Dc;
            cmd.DW4.dec.QuantizerValue0Blocktype1Y1Ac = params.QuantizerValue0Blocktype1Y1Ac;

            cmd.DW5.dec.QuantizerValue0Blocktype2Uvdc = params.QuantizerValue0Blocktype2Uvdc;
            cmd.DW5.dec.QuantizerValue0Blocktype3Uvac = params.QuantizerValue0Blocktype3Uvac;

            cmd.DW6.dec.QuantizerValue0Blocktype4Y2Dc = params.QuantizerValue0Blocktype4Y2Dc;
            cmd.DW6.dec.QuantizerValue0Blocktype5Y2Ac = params.QuantizerValue0Blocktype5Y2Ac;

            cmd.DW7.dec.QuantizerValue1Blocktype0Y1Dc = params.QuantizerValue1Blocktype0Y1Dc;
            cmd.DW7.dec.QuantizerValue1Blocktype1Y1Ac = params.QuantizerValue1Blocktype1Y1Ac;


            MHW_RESOURCE_PARAMS resourceParams;
            MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
            resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
            resourceParams.HwCommandType = MOS_MFX_VP8_PIC;

            resourceParams.presResource = params.presCoefProbBuffer;
            resourceParams.dwOffset = params.dwCoefProbTableOffset;
            resourceParams.pdwCmd = cmd.CoeffprobabilityStreaminBaseAddress.DW0_1.Value;
            resourceParams.dwLocationInCmd = 16;
            resourceParams.bIsWritable = false;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
            
            if (params.SegmentEnableFlag)
            {
                resourceParams.presResource = params.presSegmentationIdStreamBuffer;
                resourceParams.dwOffset = 0;
                resourceParams.pdwCmd = cmd.SegmentationIdStreamBaseAddress.DW0_1.Value;
                resourceParams.dwLocationInCmd = 35;
                resourceParams.bIsWritable = true;


                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
            

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFD_VP8_BSD_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(MFD_VP8_BSD_OBJECT);

        #define DO_FIELDS()                                                                                 \
            DO_FIELD(DW1,   CodedNumOfCoeffTokenPartitions,     params.CodedNumOfCoeffTokenPartitions);     \
            DO_FIELD(DW1,   Partition0CpbacEntropyRange,        params.Partition0CpbacEntropyRange);        \
            DO_FIELD(DW1,   Partition0CpbacEntropyCount,        params.Partition0CpbacEntropyCount);        \
            DO_FIELD(DW2,   Partition0CpbacEntropyValue,        params.Partition0CpbacEntropyValue);        \
            DO_FIELD(DW3,   IndirectPartition0DataLength,       params.IndirectPartition0DataLength);       \
            DO_FIELD(DW4,   IndirectPartition0DataStartOffset,  params.IndirectPartition0DataStartOffset);  \
            DO_FIELD(DW5,   IndirectPartition1DataLength,       params.IndirectPartition1DataLength);       \
            DO_FIELD(DW6,   IndirectPartition1DataStartOffset,  params.IndirectPartition1DataStartOffset);  \
            DO_FIELD(DW7,   IndirectPartition2DataLength,       params.IndirectPartition2DataLength);       \
            DO_FIELD(DW8,   IndirectPartition2DataStartOffset,  params.IndirectPartition2DataStartOffset);  \
            DO_FIELD(DW9,   IndirectPartition3DataLength,       params.IndirectPartition3DataLength);       \
            DO_FIELD(DW10,  IndirectPartition3DataStartOffset,  params.IndirectPartition3DataStartOffset);  \
            DO_FIELD(DW11,  IndirectPartition4DataLength,       params.IndirectPartition4DataLength);       \
            DO_FIELD(DW12,  IndirectPartition4DataStartOffset,  params.IndirectPartition4DataStartOffset);  \
            DO_FIELD(DW13,  IndirectPartition5DataLength,       params.IndirectPartition5DataLength);       \
            DO_FIELD(DW14,  IndirectPartition5DataStartOffset,  params.IndirectPartition5DataStartOffset);  \
            DO_FIELD(DW15,  IndirectPartition6DataLength,       params.IndirectPartition6DataLength);       \
            DO_FIELD(DW16,  IndirectPartition6DataStartOffset,  params.IndirectPartition6DataStartOffset);  \
            DO_FIELD(DW17,  IndirectPartition7DataLength,       params.IndirectPartition7DataLength);       \
            DO_FIELD(DW18,  IndirectPartition7DataStartOffset,  params.IndirectPartition7DataStartOffset);  \
            DO_FIELD(DW19,  IndirectPartition8DataLength,       params.IndirectPartition8DataLength);       \
            DO_FIELD(DW20,  IndirectPartition8DataStartOffset,  params.IndirectPartition8DataStartOffset);  \

#include "mhw_hwcmd_process_cmdfields.h"
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__mfx__Impl)
};
}//namespace mfx
}//namespace vdbox
}//namespace mhw

#endif  // __MHW_VDBOX_MFX_IMPL_H__
