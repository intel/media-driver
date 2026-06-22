/*===================== begin_copyright_notice ==================================
Copyright (c) 2026 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/

//!
//! \file     bypass_hw_legacy.h
//! \brief    Hardware bypass utility class for legacy codecs (HEVC/AVC/AV1)
//! \details  Mirrors BypassHW operations using MhwMiInterface (media_common) instead of
//!           mhw::HwItfMgr (code-gen path). Shared slot state stored in OsContextNext.
//!

#ifndef __BYPASS_HW_LEGACY_H__
#define __BYPASS_HW_LEGACY_H__

#include "mos_bypass_hw_defs.h"
#include "mos_os.h"
#include "mhw_mi_itf.h"
#include "media_class_trace.h"
#include "codechal_setting.h"
#include <mutex>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

class OsContextNext;

//!
//! \class    BypassHwLegacy
//! \brief    Hardware bypass for legacy codecs (HEVC/AVC/AV1 decode+encode)
//! \details  Uses MhwMiInterface for MI commands. Per-pipeline slot index held locally;
//!           all shared slot state lives in OsContextNext. Two-phase cleanup:
//!           Destroy() must be called explicitly before MOS_Delete().
//!
class BypassHwLegacy
{
public:
    BypassHwLegacy();
    ~BypassHwLegacy();

    BypassHwLegacy(const BypassHwLegacy&) = delete;
    BypassHwLegacy& operator=(const BypassHwLegacy&) = delete;

    //!
    //! \brief  Initialize with OS and MI interfaces; populates OsContextNext slot state on first call
    //! \param  [in] pOsInterface   Pointer to OS interface
    //! \param  [in] pMiInterface   Pointer to legacy MI interface (from media_common)
    //! \return MOS_STATUS
    //!
    MOS_STATUS Initialize(PMOS_INTERFACE pOsInterface, std::shared_ptr<mhw::mi::Itf> pMiItf);

    //!
    //! \brief  Claim a dummy VDBox slot via least-loaded algorithm
    //! \param  [out] gpuNode       Assigned GPU node (VIDEO or VE)
    //! \param  [in]  codec         Codec standard enum
    //! \param  [in]  isEncode      true for encode, false for decode
    //! \param  [in]  width         Frame width
    //! \param  [in]  height        Frame height
    //! \param  [in]  chromaFormat  Chroma format (420/422/444)
    //! \param  [in]  bitDepth      Bit depth (8/10/12)
    //! \param  [in]  targetUsage   Target usage (1-7 encode, 0 decode)
    //! \return MOS_STATUS
    //!
    MOS_STATUS FetchDummyVdNode(
        MOS_GPU_NODE       &gpuNode,
        CODECHAL_STANDARD   codec,
        bool                isEncode,
        uint32_t            width,
        uint32_t            height,
        uint8_t             chromaFormat,
        uint8_t             bitDepth,
        uint8_t             targetUsage);

    //!
    //! \brief  Set pipeline characteristics for config file repeat count lookup
    //! \param  [in] codecStandard  Codec standard enum
    //! \param  [in] chromaFormat   Chroma subsampling (420/422/444)
    //! \param  [in] width          Frame width
    //! \param  [in] height         Frame height
    //! \param  [in] bitDepth       Bit depth in bits (8, 10, or 12)
    //! \param  [in] tu             Target usage (1-7 for encode, 0 for decode)
    //!
    void SetPipelineCharacteristics(
        CODECHAL_STANDARD codecStandard,
        uint32_t          chromaFormat,
        uint32_t          width,
        uint32_t          height,
        uint32_t          bitDepth,
        uint32_t          tu);

    //!
    //! \brief  Add predicate command to skip following commands (NOOP mode)
    //! \param  [in] cmdBuffer  Pointer to command buffer
    //! \return MOS_STATUS
    //!
    MOS_STATUS StartPredicate(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief  Add predicate command to resume normal execution
    //! \param  [in] cmdBuffer  Pointer to command buffer
    //! \return MOS_STATUS
    //!
    MOS_STATUS StopPredicate(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief  Add NullHW proxy command loop to command buffer
    //! \param  [in] cmdBuffer  Pointer to command buffer
    //! \param  [in] isEncode   true for encode context, false for decode
    //! \return MOS_STATUS
    //!
    MOS_STATUS AddNullHwProxyCmd(PMOS_COMMAND_BUFFER cmdBuffer, bool isEncode);

    //!
    //! \brief  Overwrite status report with fixed NullHW values
    //! \param  [out] status      Status value (set to 0)
    //! \param  [out] streamSize  Stream size (set to 1024)
    //!
    void StatusReport(uint32_t &status, uint32_t &streamSize);

    //!
    //! \brief  Release GPU allocations. Must be called explicitly before MOS_Delete().
    //!         Idempotent — safe to call more than once.
    //! \return MOS_STATUS
    //!
    MOS_STATUS Destroy();

#if (_DEBUG || _RELEASE_INTERNAL)
    //! \brief  Get repeat count (debug accessor for ULT)
    uint32_t GetRepeatCount() const { return m_repeatCount; }

    //! \brief  Get scalability flag (debug accessor for ULT)
    bool GetIsScalable() const { return m_isScalable; }

    //! \brief  Get claimed slot index (debug accessor for ULT)
    int32_t GetClaimedSlotIndex() const { return m_claimedSlotIndex; }
#endif

    // Static config file cache (shared across all BypassHwLegacy instances)
    static std::mutex                    s_configFileMutex;
    static bool                          s_configFileLoaded;
    static std::vector<RepeatCountEntry> s_configEntries;

private:
    //! \brief  Get OsContextNext from m_osInterface
    OsContextNext *GetOsDeviceContext();

    //! \brief  Map CODECHAL_STANDARD to config file codec name
    static std::string MapCodecName(CODECHAL_STANDARD codec);

    //! \brief  Map chroma format to subsampling string
    static std::string MapSubsampling(uint32_t chromaFormat);

    //! \brief  Load and parse config file into static cache
    static bool LoadConfigFile(const std::string &filePath);

    //! \brief  Find a matching config entry; must be called under s_configFileMutex
    static const RepeatCountEntry *FindEntry(
        const std::string &codec, const std::string &direction,
        const std::string &subsampling,
        uint32_t width, uint32_t height, uint32_t bitDepth, uint32_t tu);

    //! \brief  Read proxy config file path from user feature key and load into static cache; no-op if already loaded.
    //!         Must be called under s_configFileMutex.
    void LoadProxyConfigFile();

    //! \brief  Look up repeat count from loaded config entries
    uint32_t LookupRepeatCount(bool isEncode);

    //! \brief  Look up scalability flag from config entries
    bool LookupScalabilityFromConfig(
        CODECHAL_STANDARD codec,
        bool              isEncode,
        uint32_t          width,
        uint32_t          height,
        uint32_t          chromaFormat,
        uint32_t          bitDepth,
        uint32_t          targetUsage);

    // Instance members
    int32_t             m_claimedSlotIndex = -1;
    bool                m_isEncode         = false;
    bool                m_isScalable       = false;
    bool                m_readRepeatCount  = false;
    uint32_t            m_repeatCount      = 0;
    bool                m_destroyed        = false;

    PMOS_INTERFACE      m_osInterface  = nullptr;
    std::shared_ptr<mhw::mi::Itf> m_miItf = nullptr;

    MHW_BATCH_BUFFER    m_secondLevelBB      = {};
    bool                m_secondLevelBBAllocated    = false;
    bool                m_secondLevelBBInitialized  = false;

    // Pipeline characteristics for config file lookup
    CODECHAL_STANDARD m_pipelineCodecStandard = CODECHAL_UNDEFINED;
    uint32_t          m_pipelineChromaFormat  = 0;
    uint32_t          m_pipelineWidth         = 0;
    uint32_t          m_pipelineHeight        = 0;
    uint32_t          m_pipelineBitDepth      = 0;
    uint32_t          m_pipelineTU            = 0;

MEDIA_CLASS_DEFINE_END(BypassHwLegacy)
};

#endif // __BYPASS_HW_LEGACY_H__
