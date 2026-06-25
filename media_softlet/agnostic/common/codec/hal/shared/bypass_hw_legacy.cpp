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
//! \file     bypass_hw_legacy.cpp
//! \brief    Implementation of BypassHwLegacy for legacy codecs (HEVC/AVC/AV1)
//!

#include "bypass_hw_legacy.h"
#include "mhw_mi.h"
#include <cstdio>
#include "mos_context_next.h"
#include "mos_utilities.h"
#include "mhw_utilities_next.h"
#include "media_user_setting_specific.h"

// Static member initialization
std::mutex                    BypassHwLegacy::s_configFileMutex;
bool                          BypassHwLegacy::s_configFileLoaded = false;
std::vector<RepeatCountEntry> BypassHwLegacy::s_configEntries;

BypassHwLegacy::BypassHwLegacy()
{
}

BypassHwLegacy::~BypassHwLegacy()
{
    // Two-phase cleanup: destructor does NOT call Destroy() — caller must call it explicitly
}

OsContextNext *BypassHwLegacy::GetOsDeviceContext()
{
    if (m_osInterface && m_osInterface->osStreamState && m_osInterface->osStreamState->osDeviceContext)
    {
        return static_cast<OsContextNext *>(m_osInterface->osStreamState->osDeviceContext);
    }
    return nullptr;
}

MOS_STATUS BypassHwLegacy::Initialize(PMOS_INTERFACE pOsInterface, std::shared_ptr<mhw::mi::Itf> pMiItf)
{
    if (pOsInterface == nullptr || pMiItf == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_osInterface = pOsInterface;
    m_miItf       = std::move(pMiItf);

#if (_DEBUG || _RELEASE_INTERNAL)
    OsContextNext *osDevCtx = GetOsDeviceContext();
    if (osDevCtx == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }
    return osDevCtx->InitDummyVdboxSlots();
#else
    return MOS_STATUS_SUCCESS;
#endif // (_DEBUG || _RELEASE_INTERNAL)
}

MOS_STATUS BypassHwLegacy::FetchDummyVdNode(
    MOS_GPU_NODE      &gpuNode,
    CODECHAL_STANDARD  codec,
    bool               isEncode,
    uint32_t           width,
    uint32_t           height,
    uint8_t            chromaFormat,
    uint8_t            bitDepth,
    uint8_t            targetUsage)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    // Re-entrance guard: reuse existing slot
    if (m_claimedSlotIndex >= 0)
    {
        OsContextNext *osDevCtx = GetOsDeviceContext();
        if (osDevCtx)
        {
            gpuNode = osDevCtx->GetDummyVdboxArray()[m_claimedSlotIndex].m_node;
        }
        return MOS_STATUS_SUCCESS;
    }

    OsContextNext *osDevCtx = GetOsDeviceContext();
    if (osDevCtx == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    m_isEncode = isEncode;

    bool isScalable = LookupScalabilityFromConfig(codec, isEncode, width, height, chromaFormat, bitDepth, targetUsage);

    MOS_STATUS eStatus = osDevCtx->SelectAndClaimDummyVdSlot(isEncode, isScalable, gpuNode, m_claimedSlotIndex);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        return eStatus;
    }

    m_isScalable = isScalable;
#else
    MOS_UNUSED(gpuNode);
    MOS_UNUSED(codec);
    MOS_UNUSED(isEncode);
    MOS_UNUSED(width);
    MOS_UNUSED(height);
    MOS_UNUSED(chromaFormat);
    MOS_UNUSED(bitDepth);
    MOS_UNUSED(targetUsage);
#endif // (_DEBUG || _RELEASE_INTERNAL)

    return MOS_STATUS_SUCCESS;
}

void BypassHwLegacy::SetPipelineCharacteristics(
    CODECHAL_STANDARD codecStandard,
    uint32_t          chromaFormat,
    uint32_t          width,
    uint32_t          height,
    uint32_t          bitDepth,
    uint32_t          tu)
{
    m_pipelineCodecStandard = codecStandard;
    m_pipelineChromaFormat  = chromaFormat;
    m_pipelineWidth         = width;
    m_pipelineHeight        = height;
    m_pipelineBitDepth      = bitDepth;
    m_pipelineTU            = tu;
}

MOS_STATUS BypassHwLegacy::StartPredicate(PMOS_COMMAND_BUFFER cmdBuffer)
{
    if (m_miItf == nullptr || cmdBuffer == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    auto &par           = m_miItf->MHW_GETPAR_F(MI_SET_PREDICATE)();
    par                 = {};
    par.PredicateEnable = MHW_MI_SET_PREDICATE_ENABLE_ALWAYS;
    return m_miItf->MHW_ADDCMD_F(MI_SET_PREDICATE)(cmdBuffer);
}

MOS_STATUS BypassHwLegacy::StopPredicate(PMOS_COMMAND_BUFFER cmdBuffer)
{
    if (m_miItf == nullptr || cmdBuffer == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    auto &par           = m_miItf->MHW_GETPAR_F(MI_SET_PREDICATE)();
    par                 = {};
    par.PredicateEnable = MHW_MI_SET_PREDICATE_DISABLE;
    return m_miItf->MHW_ADDCMD_F(MI_SET_PREDICATE)(cmdBuffer);
}

void BypassHwLegacy::StatusReport(uint32_t &status, uint32_t &streamSize)
{
    status     = 0;
    streamSize = 1024;
}

MOS_STATUS BypassHwLegacy::AddNullHwProxyCmd(PMOS_COMMAND_BUFFER cmdBuffer, bool isEncode)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_osInterface == nullptr || m_miItf == nullptr || cmdBuffer == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (!m_osInterface->bNullHwIsEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // Read repeat count once per pipeline lifetime
    if (!m_readRepeatCount)
    {
        uint32_t configValue = 0;
        {
            std::lock_guard<std::mutex> lock(s_configFileMutex);
            LoadProxyConfigFile();
            configValue = LookupRepeatCount(isEncode);
        }

        if (configValue > 0)
        {
            m_repeatCount = configValue;
        }
        else
        {
            ReadUserSetting(
                m_osInterface->pfnGetUserSettingInstance(m_osInterface),
                m_repeatCount,
                __MEDIA_USER_FEATURE_VALUE_NULLHW_PROXY_REPEAT_COUNT,
                MediaUserSetting::Group::Device);
        }

        m_readRepeatCount = true;
    }

    // Allocate 2nd level batch buffer if needed
    if (!m_secondLevelBBAllocated)
    {
        MOS_ZeroMemory(&m_secondLevelBB, sizeof(MHW_BATCH_BUFFER));
        m_secondLevelBB.bSecondLevel = true;

        // Size: m_repeatCount MI_NOOPs (4 bytes each) + MI_BATCH_BUFFER_END (4 bytes) + padding
        uint32_t batchBufferSize = MOS_ALIGN_CEIL((m_repeatCount + 2) * sizeof(uint32_t), 64);
        eStatus = Mhw_AllocateBb(m_osInterface, &m_secondLevelBB, nullptr, batchBufferSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            return eStatus;
        }

        eStatus = Mhw_LockBb(m_osInterface, &m_secondLevelBB);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            return eStatus;
        }

        m_secondLevelBBAllocated = true;
    }

    // Populate 2nd level batch buffer with MI_NOOP commands (only once)
    if (!m_secondLevelBBInitialized)
    {
        MOS_COMMAND_BUFFER secondLevelCmdBuffer = {};
        MOS_ZeroMemory(&secondLevelCmdBuffer, sizeof(secondLevelCmdBuffer));
        secondLevelCmdBuffer.pCmdBase    = (uint32_t *)m_secondLevelBB.pData;
        secondLevelCmdBuffer.pCmdPtr     = secondLevelCmdBuffer.pCmdBase;
        secondLevelCmdBuffer.iRemaining  = m_secondLevelBB.iSize;
        secondLevelCmdBuffer.OsResource  = m_secondLevelBB.OsResource;
        secondLevelCmdBuffer.cmdBuf1stLvl = cmdBuffer;

        // Write m_repeatCount MI_NOOP commands into the 2nd level batch buffer.
        // MI_NOOP encodes to a single DWORD of 0x00000000 (see MI_NOOP_CMD), so this
        // is equivalent to a bulk zero-fill of m_repeatCount DWORDs. Emitting them
        // one-by-one through AddHwCmd is far slower: each call takes a mutex-locked
        // command-pool lookup plus virtual dispatch only to memcpy 4 bytes, so for
        // large repeat counts the SW latency is dominated by this loop. Fill the
        // region in one shot and advance the command buffer bookkeeping exactly as
        // AddHwCmd / MosInterface::AddCommand would have.
        const uint32_t noopBytes = m_repeatCount * (uint32_t)sizeof(uint32_t);
        MOS_ZeroMemory(secondLevelCmdBuffer.pCmdPtr, noopBytes);
        secondLevelCmdBuffer.pCmdPtr += m_repeatCount;
        secondLevelCmdBuffer.iOffset += (int32_t)noopBytes;
        secondLevelCmdBuffer.iRemaining -= (int32_t)noopBytes;

        // MI_BATCH_BUFFER_END
        auto &bbEndParams = m_miItf->MHW_GETPAR_F(MI_BATCH_BUFFER_END)();
        bbEndParams       = {};
        eStatus = m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_END)(&secondLevelCmdBuffer, nullptr);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            return eStatus;
        }

        m_secondLevelBBInitialized = true;

        eStatus = Mhw_UnlockBb(m_osInterface, &m_secondLevelBB, true);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            return eStatus;
        }
    }

    // Start 2nd level batch buffer from primary command buffer
    auto &primaryBBStartParams        = m_miItf->MHW_GETPAR_F(MI_BATCH_BUFFER_START)();
    primaryBBStartParams              = {};
    primaryBBStartParams.secondLevelBatchBuffer = true;
    eStatus = m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(cmdBuffer, &m_secondLevelBB);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        return eStatus;
    }
#endif // (_DEBUG || _RELEASE_INTERNAL)

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS BypassHwLegacy::Destroy()
{
    if (m_destroyed)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_osInterface == nullptr || !m_osInterface->bNullHwIsEnabled)
    {
        m_destroyed = true;
        return MOS_STATUS_SUCCESS;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // Release slot ref count
    OsContextNext *osDevCtx = GetOsDeviceContext();
    if (osDevCtx && m_claimedSlotIndex >= 0)
    {
        osDevCtx->ReleaseDummyVdSlot(m_claimedSlotIndex, m_isScalable);
        m_claimedSlotIndex = -1;
    }

    // Free 2nd level batch buffer
    if (m_secondLevelBBAllocated)
    {
        // Buffer is unlocked after init; only unlock here if init never completed
        if (!m_secondLevelBBInitialized)
        {
            Mhw_UnlockBb(m_osInterface, &m_secondLevelBB, true);
        }
        Mhw_FreeBb(m_osInterface, &m_secondLevelBB, nullptr);
        m_secondLevelBBAllocated    = false;
        m_secondLevelBBInitialized  = false;
    }
#endif // (_DEBUG || _RELEASE_INTERNAL)

    m_destroyed = true;
    return MOS_STATUS_SUCCESS;
}

std::string BypassHwLegacy::MapCodecName(CODECHAL_STANDARD codec)
{
    switch (codec)
    {
    case CODECHAL_HEVC:
        return "hevc";
    case CODECHAL_AVC:
        return "avc";
    case CODECHAL_AV1:
        return "av1";
    default:
        return "";
    }
}

std::string BypassHwLegacy::MapSubsampling(uint32_t chromaFormat)
{
    switch (chromaFormat)
    {
    case 1:
    case 420:
        return "420";
    case 2:
    case 422:
        return "422";
    case 3:
    case 444:
        return "444";
    default:
        return "";
    }
}

const RepeatCountEntry *BypassHwLegacy::FindEntry(
    const std::string &codec, const std::string &direction,
    const std::string &subsampling,
    uint32_t width, uint32_t height, uint32_t bitDepth, uint32_t tu)
{
    for (const auto &entry : s_configEntries)
    {
        if (entry.codec       == codec       &&
            entry.direction   == direction   &&
            entry.subsampling == subsampling &&
            entry.width       == width       &&
            entry.height      == height      &&
            entry.bitDepth    == bitDepth    &&
            entry.tu          == tu)
        {
            return &entry;
        }
    }
    return nullptr;
}


void BypassHwLegacy::LoadProxyConfigFile()
{
    if (s_configFileLoaded || !m_osInterface)
        return;

    MediaUserSetting::Value configFilePath;
    ReadUserSettingForDebug(
        m_osInterface->pfnGetUserSettingInstance(m_osInterface),
        configFilePath,
        __MEDIA_USER_FEATURE_VALUE_NULLHW_PROXY_REPEAT_COUNT_FILE,
        MediaUserSetting::Group::Device);

    std::string filePath = configFilePath.Get<std::string>();
    if (!filePath.empty())
    {
        LoadConfigFile(filePath);
    }
    s_configFileLoaded = true;  // mark attempted regardless of outcome; prevents repeated user feature key reads
}

bool BypassHwLegacy::LoadConfigFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    s_configEntries.clear();
    std::string line;
    while (std::getline(file, line))
    {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::istringstream ss(line);
        RepeatCountEntry entry = {};
        std::string token;

        // Parse: codec, direction, subsampling, width, height, bitdepth, tu, repeat_count[, scalability]
        if (!std::getline(ss, entry.codec, ','))       continue;
        if (!std::getline(ss, entry.direction, ','))   continue;
        if (!std::getline(ss, entry.subsampling, ',')) continue;

        try
        {
            if (!std::getline(ss, token, ',')) continue;
            entry.width = static_cast<uint32_t>(std::stoul(token));

            if (!std::getline(ss, token, ',')) continue;
            entry.height = static_cast<uint32_t>(std::stoul(token));

            if (!std::getline(ss, token, ',')) continue;
            entry.bitDepth = static_cast<uint32_t>(std::stoul(token));

            if (!std::getline(ss, token, ',')) continue;
            entry.tu = static_cast<uint32_t>(std::stoul(token));

            if (!std::getline(ss, token, ',')) continue;
            entry.repeatCount = static_cast<uint32_t>(std::stoul(token));
        }
        catch (const std::exception &)
        {
            continue;
        }

        // Optional scalability column
        entry.scalability = 0;
        if (std::getline(ss, token, ','))
        {
            size_t start = token.find_first_not_of(" \t\r\n");
            if (start != std::string::npos)
            {
                try
                {
                    entry.scalability = static_cast<uint32_t>(std::stoul(token.substr(start)));
                }
                catch (const std::exception &)
                {
                    entry.scalability = 0;
                }
            }
        }

        // Trim whitespace from string fields
        auto trim = [](std::string &s) {
            size_t start = s.find_first_not_of(" \t\r\n");
            size_t end   = s.find_last_not_of(" \t\r\n");
            if (start == std::string::npos) { s.clear(); return; }
            s = s.substr(start, end - start + 1);
        };
        trim(entry.codec);
        trim(entry.direction);
        trim(entry.subsampling);

        s_configEntries.push_back(std::move(entry));
    }

    return !s_configEntries.empty();
}

uint32_t BypassHwLegacy::LookupRepeatCount(bool isEncode)
{
    std::string codecName   = MapCodecName(m_pipelineCodecStandard);
    std::string direction   = isEncode ? "encode" : "decode";
    std::string subsampling = MapSubsampling(m_pipelineChromaFormat);

    if (codecName.empty() || subsampling.empty())
        return 0;

    const RepeatCountEntry *entry = FindEntry(
        codecName, direction, subsampling,
        m_pipelineWidth, m_pipelineHeight, m_pipelineBitDepth, m_pipelineTU);
    return entry ? entry->repeatCount : 0;
}

bool BypassHwLegacy::LookupScalabilityFromConfig(
    CODECHAL_STANDARD codec,
    bool              isEncode,
    uint32_t          width,
    uint32_t          height,
    uint32_t          chromaFormat,
    uint32_t          bitDepth,
    uint32_t          targetUsage)
{
    std::lock_guard<std::mutex> lock(s_configFileMutex);
    LoadProxyConfigFile();

    std::string codecName   = MapCodecName(codec);
    std::string direction   = isEncode ? "encode" : "decode";
    std::string subsampling = MapSubsampling(chromaFormat);

    if (codecName.empty() || subsampling.empty())
        return false;

    const RepeatCountEntry *entry = FindEntry(
        codecName, direction, subsampling,
        width, height, bitDepth, targetUsage);
    return entry ? entry->scalability == 1 : false;
}

