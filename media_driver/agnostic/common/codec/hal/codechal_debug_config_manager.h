/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_debug_config_manager.h
//! \brief    Defines the dump configuration manager.
//! \details  The debug interface dumps configuration manager file which parse attributes.
//!
#ifndef __CODECHAL_DEBUG_CONFIG_MANAGER_H__
#define __CODECHAL_DEBUG_CONFIG_MANAGER_H__
#if USE_CODECHAL_DEBUG_TOOL

#include <string>
#include <map>
#include <vector>
#include "mos_defs.h"
#include "codechal_hw.h"

namespace CodechalDbgAttr
{
static const char *attrPicParams              = "PicParams";
static const char *attrSubsetsParams          = "SubsetsParams";
static const char *attrFeiPicParams           = "FeiPicParams";
static const char *attrMvcExtPicParams        = "MvcExtPicParams";
static const char *attrSegmentParams          = "SegmentParams";
static const char *attrSlcParams              = "SlcParams";
static const char *attrMbParams               = "MbParams";
static const char *attrIqParams               = "IqParams";
static const char *attrSeqParams              = "SeqParams";
static const char *attrVuiParams              = "VuiParams";
static const char *attrBitstream              = "Bitstream";
static const char *attrStreamOut              = "StreamOut";
static const char *attrStreamIn               = "StreamIn";
static const char *attrResidualDifference     = "ResidualDifference";
static const char *attrDeblocking             = "Deblocking";
static const char *attrMvData                 = "MvData";
static const char *attrVc1Bitplane            = "Vc1Bitplane";
static const char *attrCoefProb               = "CoefProb";
static const char *attrSegId                  = "SegId";
static const char *attrHucRegions             = "HucRegions";
static const char *attrHuCDmem                = "HucDmem";
static const char *attrCmdBufferMfx           = "CmdBufferMfx";
static const char *attr2ndLvlBatchMfx         = "2ndLvlBatchMfx";
static const char *attrDecodeOutputSurface    = "DecodeOutputSurface";
static const char *attrDecodeAuxSurface       = "DumpAuxsurface";
static const char *attrDecodeBltOutput        = "DumpBltOutput";
static const char *attrSfcOutputSurface       = "SfcOutputSurface";
static const char *attrReferenceSurfaces      = "ReferenceSurfaces";
static const char *attrEncodeRawInputSurface  = "EncodeRawInputSurface";
static const char *attrReconstructedSurface   = "ReconstructedSurface";
static const char *attrPakInput               = "PakInput";
static const char *attrPakOutput              = "PakOutput";
static const char *attrUpsamlingInput         = "UpsamplingInput";
static const char *attrResidualSurface        = "ResidualSurface";
static const char *attrStCoeff                = "StCoeff";
static const char *attrCoeffPredCs            = "CoeffPredCs";
static const char *attrMbRecord               = "MbRecord";
static const char *attrOverwriteCommands      = "OverwriteCommands";
static const char *attrHuffmanTbl             = "HuffmanTbl";
static const char *attrScanParams             = "ScanParams";
static const char *attrDriverUltDump          = "DriverUltDump";
static const char *attrDumpBufferInBinary     = "DumpBufferInBinary";
static const char *attrDumpToThreadFolder     = "DumpToThreadFolder";
static const char *attrDumpCmdBufInBinary     = "DumpCmdBufInBinary";
static const char *attrDumpEncodePar          = "DumpEncodePar";
static const char *attrForceCmdDumpLvl        = "ForceCmdDumpLvl";
static const char *attrForceCurbeDumpLvl      = "ForceCurbeDumpLvl";
static const char *attrForceYUVDumpWithMemcpy = "ForceYUVDumpWithMemcpy";
static const char *attrVdencOutput            = "VdencOutput";
static const char *attrDecodeProcParams       = "DecodeProcParams";
static const char *attrFrameState             = "FrameState";
static const char *attrCUStreamout            = "CUStreamout";
static const char *attrBrcPakStats            = "BrcPakStats";
static const char *attrImageState             = "ImageState";
static const char *attrSliceSizeStreamout     = "SliceSizeStreamout";
static const char *attrCoeffProb              = "PakHwCoeffProbs";
static const char *attrStatusReport           = "StatusReport";
static const char *attrPakObjStreamout        = "PakObjStreamOut";
static const char *attrTileBasedStats         = "TileBasedStats";
static const char *attrROISurface             = "ROIInput";
static const char *attrHuCStitchDataBuf       = "HuCStitchDataBuffer";

// MD5 attributes
static const char *attrMD5HashEnable    = "MD5HasEnable";
static const char *attrMD5FlushInterval = "MD5FlushInterval";
static const char *attrMD5PicWidth      = "MD5PicWidth";
static const char *attrMD5PicHeight     = "MD5PicHeight";
// kernel attributes
static const char *attrDsh         = "DSH";
static const char *attrIsh         = "ISH";
static const char *attrSsh         = "SSH";
static const char *attrCurbe       = "Curbe";
static const char *attrCmdBuffer   = "CmdBuffer";
static const char *attr2ndLvlBatch = "2ndLvlBath";
static const char *attrInput       = "Input";
static const char *attrOutput      = "Output";
};

namespace CodechalDbgKernel
{
class KernelStateMap
{
public:
    using kernelMapType = std::map<CODECHAL_MEDIA_STATE_TYPE, std::string>;

    static bool RegisterKernelStr(CODECHAL_MEDIA_STATE_TYPE mediaState, std::string kernelName)
    {
        kernelMapType &kernelMap = GetKernelStateMap();
        auto           it        = kernelMap.find(mediaState);
        if (it == kernelMap.end())
        {
            kernelMap.insert(std::make_pair(mediaState, kernelName));
        }
        return true;
    }

    static kernelMapType &GetKernelStateMap()
    {
        static kernelMapType m_kernelStateMap;
        return m_kernelStateMap;
    }
};

static const char *kernelOlp                          = "Vc1Olp";
static const char *kernelEncNormal                    = "MbEncNorm";
static const char *kernelEncPerf                      = "MbEncPerf";
static const char *kernelEncQuality                   = "MbEncQlty";
static const char *kernelEncIFrameDist                = "IFrameDist";
static const char *kernel32xScaling                   = "32xScaling";
static const char *kernel16xScaling                   = "16xScaling";
static const char *kernel4xScaling                    = "4xScaling";
static const char *kernel32xMe                        = "32xMe";
static const char *kernel16xMe                        = "16xMe";
static const char *kernel4xMe                         = "4xMe";
static const char *kernelBrcInitReset                 = "BrcInitReset";
static const char *kernelBrcUpdate                    = "BrcUpdate";
static const char *kernel2xScaling                    = "2xScaling";
static const char *kernelHevc32x32PuModeDecision      = "HEVC_32x32_PU_MD";
static const char *kernelHevc16x16PuSad               = "HEVC_16x16_PU_SAD";
static const char *kernelHevc16x16PuModeDecision      = "HEVC_16x16_PU_MD";
static const char *kernelHevc8x8Pu                    = "HEVC_8x8_PU_MD";
static const char *kernelHevc8x8PuFMode               = "HEVC_8x8_PU_FMODE";
static const char *kernelHevc32x32BIntraCheck         = "HEVC_32x32_B_INTRA_CHECK";
static const char *kernelHevcBMbenc                   = "HEVC_B_MBENC";
static const char *kernelHevcBPak                     = "HEVC_B_PAK";
static const char *kernelHevcBrcLcuUpdate             = "HEVC_LCU_BRCUpdate";
static const char *kernelMeVdencStreamIn              = "MeVDEncStreamIn";
static const char *kernelVP9EncI32x32                 = "VP9_I_32x32_MBENC";
static const char *kernelVP9EncI16x16                 = "VP9_I_16x16_MBENC";
static const char *kernelVP9EncP                      = "VP9_P_MBENC";
static const char *kernelVP9EncTx                     = "VP9_TX_MBENC";
static const char *kernelVP9Dys                       = "VP9_DYS";
static const char *kernelVP9PakLumaRecon              = "VP9_I_MBPAK_LumaRecon";
static const char *kernelVP9PakChromaRecon            = "VP9_I_MBPAK_ChromaRecon";
static const char *kernelVP9PakDeblockMask            = "VP9_I_MBPAK_DeblockMask";
static const char *kernelVP9PakLumaDeblock            = "VP9_I_MBPAK_LumaDeblock";
static const char *kernelVP9PakChromaDeblock          = "VP9_I_MBPAK_ChromaDeblock";
static const char *kernelVP9PakMcPred                 = "VP9_P_MbPak_McPred";
static const char *kernelVP9PakPFrameLumaRecon        = "VP9_P_MbPak_LumaRecon";
static const char *kernelVP9PakPFrameLumaRecon32x32   = "VP9_P_MbPak_LumaRecon";
static const char *kernelVP9PakPFrameChromaRecon      = "VP9_P_MbPak_ChromaRecon";
static const char *kernelVP9PakPFrameIntraLumaRecon   = "VP9_Intra_P_MbPak_LumaRecon";
static const char *kernelVP9PakPFrameIntraChromaRecon = "VP9_Intra_P_MbPak_ChromaRecon";
static const char *kernelPreProc                      = "PreProc";
static const char *kernelEncWP                        = "WP";
static const char *kernelHevcIMbenc                   = "HEVC_I_MBENC";
static const char *kernelCscDsCopy                    = "CSCDsCopy";
static const char *kernel2x4xScaling                  = "2x_4xScaling";
static const char *kernelHevcLcu64BMbenc              = "HEVC_LCU64_B_MBENC";
static const char *kernelMbBrcUpdate                  = "MbBrcUpdate";
static const char *kernelStaticFrameDetection         = "StaticFrameDetection";
static const char *kernelHevcRoi                      = "HEVC_ROI";

static bool regOlp                          = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_OLP, kernelOlp);
static bool regEncNormal                    = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_ENC_NORMAL, kernelEncNormal);
static bool regEncQuality                   = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_ENC_PERFORMANCE, kernelEncPerf);
static bool regEncPerf                      = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_ENC_QUALITY, kernelEncQuality);
static bool regEncIFrameDist                = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST, kernelEncIFrameDist);
static bool reg32xScaling                   = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_32X_SCALING, kernel32xScaling);
static bool reg16xScaling                   = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_16X_SCALING, kernel16xScaling);
static bool reg4xScaling                    = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_4X_SCALING, kernel4xScaling);
static bool reg32xMe                        = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_32X_ME, kernel32xMe);
static bool reg16xMe                        = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_16X_ME, kernel16xMe);
static bool reg4xMe                         = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_4X_ME, kernel4xMe);
static bool regBrcInit                      = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_BRC_INIT_RESET, kernelBrcInitReset);
static bool regBrcUpdate                    = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_BRC_UPDATE, kernelBrcUpdate);
static bool reg2xScale                      = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_2X_SCALING, kernel2xScaling);
static bool regHevc32x32PuMode              = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_32x32_PU_MODE_DECISION, kernelHevc32x32PuModeDecision);
static bool regHevc16x16PuSad               = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_16x16_PU_SAD, kernelHevc16x16PuSad);
static bool regHevc16x16PuMode              = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_16x16_PU_MODE_DECISION, kernelHevc16x16PuModeDecision);
static bool regHevc8x8Pu                    = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_8x8_PU, kernelHevc8x8Pu);
static bool regHevc8x8PuFMode               = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_8x8_PU_FMODE, kernelHevc8x8PuFMode);
static bool regHevc32x32BIntraCheck         = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_32x32_B_INTRA_CHECK, kernelHevc32x32BIntraCheck);
static bool regHevcBMben                    = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_HEVC_B_MBENC, kernelHevcBMbenc);
static bool regHevcBPak                     = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_HEVC_B_PAK, kernelHevcBPak);
static bool regHevcBrcLcuUpdate             = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_HEVC_BRC_LCU_UPDATE, kernelHevcBrcLcuUpdate);
static bool regMeVdencStreamIn              = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN, kernelMeVdencStreamIn);
static bool regVp9EncI32x32                 = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_ENC_I_32x32, kernelVP9EncI32x32);
static bool regVp9EncI16x16                 = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_ENC_I_16x16, kernelVP9EncI16x16);
static bool regVp9EncP                      = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_ENC_P, kernelVP9EncP);
static bool regVp9EncTx                     = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_ENC_TX, kernelVP9EncTx);
static bool regVp9Dys                       = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_DYS, kernelVP9Dys);
static bool regVp9PakLumaRecon              = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_PAK_LUMA_RECON, kernelVP9PakLumaRecon);
static bool regVp9PakChromaRecon            = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_PAK_CHROMA_RECON, kernelVP9PakChromaRecon);
static bool regVp9PakDeblockMask            = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_PAK_DEBLOCK_MASK, kernelVP9PakDeblockMask);
static bool regVp9PakLumaDeblock            = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_PAK_LUMA_DEBLOCK, kernelVP9PakLumaDeblock);
static bool regVp9PakChromaDeblock          = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_PAK_CHROMA_DEBLOCK, kernelVP9PakChromaDeblock);
static bool regVp9PakMcPred                 = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_PAK_MC_PRED, kernelVP9PakMcPred);
static bool regVp9PakPFrameLumaRecon        = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_LUMA_RECON, kernelVP9PakPFrameLumaRecon);
static bool regVp9PakPFrameLumaRecon32x32   = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_LUMA_RECON_32x32, kernelVP9PakPFrameLumaRecon32x32);
static bool regVp9PakPFrameChromaRecon      = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_CHROMA_RECON, kernelVP9PakPFrameChromaRecon);
static bool regVp9PakPFrameIntraLumaRecon   = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_INTRA_LUMA_RECON, kernelVP9PakPFrameIntraLumaRecon);
static bool regVp9PakPFrameIntraChromaRecon = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_INTRA_CHROMA_RECON, kernelVP9PakPFrameIntraChromaRecon);
static bool regPreProc                      = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_PREPROC, kernelPreProc);
static bool regEncWP                        = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_ENC_WP, kernelEncWP);
static bool regHevcIMbenc                   = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_HEVC_I_MBENC, kernelHevcIMbenc);
static bool regCscDsCopy                    = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_CSC_DS_COPY, kernelCscDsCopy);
static bool reg2x4xScaling                  = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_2X_4X_SCALING, kernel2x4xScaling);
static bool regHevcLcu64BMbenc              = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_HEVC_LCU64_B_MBENC, kernelHevcLcu64BMbenc);
static bool regMbBrcUpdate                  = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_MB_BRC_UPDATE, kernelMbBrcUpdate);
static bool regStaticFrameDetection         = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_STATIC_FRAME_DETECTION, kernelStaticFrameDetection);
static bool regHevcRoi                      = KernelStateMap::RegisterKernelStr(CODECHAL_MEDIA_STATE_HEVC_ROI, kernelHevcRoi);
};

struct KernelDumpConfig
{
    bool dumpDsh         = false;
    bool dumpSsh         = false;
    bool dumpIsh         = false;
    bool dumpCurbe       = false;
    bool dumpCmdBuffer   = false;
    bool dump2ndLvlBatch = false;
    bool dumpInput       = false;
    bool dumpOutput      = false;
};
struct CodechalDbgCfg
{
    int32_t frameIndex;
    std::map<std::string, int32_t>          cmdAttribs;
    std::map<std::string, KernelDumpConfig> kernelAttribs;
    std::vector<std::string> forceCmds;
};
class CodechalDebugInterface;
class CodechalDebugConfigMgr
{
public:
    CodechalDebugConfigMgr(
        CodechalDebugInterface *debugInterface,
        CODECHAL_FUNCTION       CodecFunction,
        std::string             outputFolderPath);
    ~CodechalDebugConfigMgr();
    MOS_STATUS ParseConfig();
    MOS_STATUS DeleteCfgNode(uint32_t frameIdx);

    std::string GetMediaStateStr(CODECHAL_MEDIA_STATE_TYPE mediaState);

    bool AttrIsEnabled(std::string attrib);
    bool AttrIsEnabled(CODECHAL_MEDIA_STATE_TYPE mediaState, std::string attrib);

protected:
    void GenerateDefaultConfig();
    uint32_t GetFrameConfig(uint32_t frameIdx);
    void StoreDebugAttribs(std::string line, CodechalDbgCfg *dbgCfg);
    void ParseKernelAttribs(std::string line, CodechalDbgCfg *dbgCfg);
    bool KernelAttrEnabled(KernelDumpConfig kernelConfig, std::string attrName);

protected:
    CodechalDebugInterface *    m_debugInterface = nullptr;
    CODECHAL_FUNCTION           m_codecFunction;
    std::string                 m_outputFolderPath;
    std::vector<CodechalDbgCfg> m_debugFrameConfigs;
    CodechalDbgCfg *            m_debugAllConfigs = nullptr;
};

#endif  //USE_CODECHAL_DEBUG_TOOL
#endif  /* __CODECHAL_DEBUG_CONFIG_MANAGER_H__ */
