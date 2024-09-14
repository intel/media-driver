/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     media_debug_interface.cpp
//! \brief    Defines the debug interface shared by all of Media.
//! \details  The debug interface dumps output from Media based on input config file.
//!
#include "media_debug_interface.h"
#if USE_MEDIA_DEBUG_TOOL
#include "media_debug_config_manager.h"
#include "media_debug_fast_dump.h"
#include "codec_hw_next.h"
#include <fstream>
#include <sstream>
#include <iomanip>

#if !defined(LINUX) && !defined(ANDROID)
#include "UmdStateSeparation.h"
#endif

MediaDebugInterface::MediaDebugInterface()
{
    memset(m_fileName, 0, sizeof(m_fileName));
    memset(m_path, 0, sizeof(m_path));
    InitCRCTable(m_crcTable);
}

MediaDebugInterface::~MediaDebugInterface()
{
    if (nullptr != m_configMgr)
    {
        MOS_Delete(m_configMgr);
    }

    if (!Mos_ResourceIsNull(&m_temp2DSurfForCopy.OsResource))
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_temp2DSurfForCopy.OsResource);
    }
}

MOS_STATUS MediaDebugInterface::InitDumpLocation()
{
    m_crcGoldenRefFileName = m_outputFilePath + std::string("GoldenReference.txt");
    if (m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpToThreadFolder))
    {
        std::string ThreadSubFolder = "T" + std::to_string(MosUtilities::MosGetCurrentThreadId()) + MOS_DIRECTORY_DELIMITER;
        m_outputFilePath            = m_outputFilePath + ThreadSubFolder;
        MosUtilities::MosCreateDirectory(const_cast<char *>(m_outputFilePath.c_str()));
    }

    m_ddiFileName = m_outputFilePath + "ddi.par";
    std::ofstream ofs(m_ddiFileName, std::ios::out);
    ofs << "ParamFilePath"
        << " = \"" << m_fileName << "\"" << std::endl;
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::SetOutputFilePath()
{
    std::string          outputPathKey;
    std::string          dumpFilePath;

    MEDIA_DEBUG_FUNCTION_ENTER;

#ifdef LINUX
    char *customizedOutputPath = getenv("MOS_DEBUG_OUTPUT_LOCATION");
    if (customizedOutputPath != nullptr && strlen(customizedOutputPath) != 0)
    {
        m_outputFilePath = customizedOutputPath;
        m_outputFilePath.erase(m_outputFilePath.find_last_not_of(" \n\r\t") + 1);
        if (m_outputFilePath[m_outputFilePath.length() - 1] != MOS_DIRECTORY_DELIMITER)
            m_outputFilePath += MOS_DIRECTORY_DELIMITER;
    }
    else
#endif
    {
        outputPathKey = SetOutputPathKey();
        ReadUserSettingForDebug(
            m_userSettingPtr,
            m_outputFilePath,
            outputPathKey,
            MediaUserSetting::Group::Device);

        if (!m_outputFilePath.empty())
        {
            if (m_outputFilePath.back() != MOS_DIRECTORY_DELIMITER)
            {
                m_outputFilePath += MOS_DIRECTORY_DELIMITER;
            }
        }
        else
        {
#if defined(LINUX) || defined(ANDROID)
            m_outputFilePath = MOS_DEBUG_DEFAULT_OUTPUT_LOCATION;
#else
            // Use state separation APIs to obtain appropriate storage location
            if (SUCCEEDED(GetDriverPersistentStorageLocation(dumpFilePath)))
            {
                m_outputFilePath = dumpFilePath.c_str();
                outputPathKey    = InitDefaultOutput();
                ReportUserSettingForDebug(
                    m_userSettingPtr,
                    outputPathKey,
                    m_outputFilePath,
                    MediaUserSetting::Group::Device);
            }
            else
            {
                return MOS_STATUS_UNKNOWN;
            }
#endif
        }
    }
    return MOS_STATUS_SUCCESS;
}

void MediaDebugInterface::PackGoldenReferences(std::initializer_list<std::vector<uint32_t>> goldenReference)
{
    for (auto beg = goldenReference.begin(); beg != goldenReference.end(); beg++)
    {
        m_goldenReferences.push_back(*beg);
    }
}

MOS_STATUS MediaDebugInterface::CaptureGoldenReference(uint8_t *buf, uint32_t size, uint32_t hwCrcValue)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    if (m_enableHwDebugHooks &&
        !m_goldenReferenceExist)
    {
        if (m_swCRC)
        {
            if (buf == nullptr)
            {
                return MOS_STATUS_NULL_POINTER;
            }
            auto crcVal = CalculateCRC(buf, size, 0);
            m_crcGoldenReference.push_back(crcVal);
        }
        else
        {
            m_crcGoldenReference.push_back(hwCrcValue);
        }
    }
    return eStatus;
}

MOS_STATUS MediaDebugInterface::FillSemaResource(std::vector<uint32_t*> &vSemaData, std::vector<uint32_t> &data)
{
    if (vSemaData.size() != data.size())
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    for (uint32_t i = 0; i < vSemaData.size(); i++)
    {
        MEDIA_DEBUG_CHK_NULL(vSemaData[i]);
        *vSemaData[i] = data[i];
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::LockResource(uint32_t *semaData, PMOS_RESOURCE reSemaphore)
{
    CodechalResLock semaLock(m_osInterface, reSemaphore);
    semaData = (uint32_t *)semaLock.Lock(CodechalResLock::writeOnly);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::LockSemaResource(std::vector<uint32_t *> &vSemaData, std::vector<MOS_RESOURCE> &vResource)
{
    for (uint32_t i = 0; i < vResource.size(); i++)
    {
        CodechalResLock semaLock(m_osInterface, &vResource[i]);
        uint32_t *      smData = (uint32_t *)semaLock.Lock(CodechalResLock::writeOnly);
        LockResource(smData, &vResource[i]);
        vSemaData.push_back(smData);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpToFile(const GoldenReferences &goldenReferences)
{
    std::ofstream ofs(m_crcGoldenRefFileName, std::ios_base::out);
    if (goldenReferences.size() <= 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    for (uint32_t i = 0; i < goldenReferences[0].size(); i++)
    {
        for (auto golden : goldenReferences)
        {
            ofs << golden[i] << '\t';
        }
        ofs << '\n';
    }
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpGoldenReference()
{
    if (m_enableHwDebugHooks && !m_goldenReferenceExist && m_goldenReferences.size() > 0)
    {
        return DumpToFile(m_goldenReferences);
    }
    else
    {
        return MOS_STATUS_UNKNOWN;
    }
}

MOS_STATUS MediaDebugInterface::LoadGoldenReference()
{
    std::ifstream ifs(m_crcGoldenRefFileName, std::ios_base::in);
    std::vector<uint32_t> lines;
    std::string           str;
    uint32_t              num;
    if (!ifs)
    {
        return MOS_STATUS_FILE_OPEN_FAILED;
    }
    uint32_t crc;
    while (!ifs.eof())
    {
        std::getline(ifs, str);
        std::stringstream stringin(str); 
        lines.clear();
        while (stringin >> num)
        {
            lines.push_back(num);
        }
        m_goldenReferences.push_back(lines);
    }
    ifs.close();
    m_goldenReferences.pop_back();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpUncompressedYUVSurface(PMOS_SURFACE surface)
{
    if (surface == nullptr)
    {
        MEDIA_DEBUG_ASSERTMESSAGE("Surface for Dump is nullptr");
        return MOS_STATUS_NULL_POINTER;
    }

    MEDIA_DEBUG_CHK_STATUS(ReAllocateSurface(
        &m_temp2DSurfForCopy,
        surface,
        "Temp2DSurfForSurfDumper",
        MOS_GFXRES_2D,
        true));

    MEDIA_DEBUG_VERBOSEMESSAGE("Temp2DSurfaceForCopy width %d, height %d, pitch %d, TileType %d, bIsCompressed %d, CompressionMode %d",
        m_temp2DSurfForCopy.dwWidth,
        m_temp2DSurfForCopy.dwHeight,
        m_temp2DSurfForCopy.dwPitch,
        m_temp2DSurfForCopy.TileType,
        m_temp2DSurfForCopy.bIsCompressed,
        m_temp2DSurfForCopy.CompressionMode);

    // Copy Original Surface to a Linear/Uncompressed surface, later lock can use m_temp2DSurfForCopy for uncompresed surfaces
    // if the return is not supported, then the lock should only happen on origianl resource
    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnDoubleBufferCopyResource(
        m_osInterface,
        &surface->OsResource,
        &m_temp2DSurfForCopy.OsResource,
        false));

    return MOS_STATUS_SUCCESS;
}

#define FIELD_TO_OFS(name, shift) ofs << shift #name << ": " << (int64_t)ptr->name << std::endl;
#define EMPTY_TO_OFS()
#define UNION_STRUCT_START_TO_OFS()     ofs << "union"      << std::endl \
                                            << "{"          << std::endl \
                                            << "    struct" << std::endl \
                                            << "    {"      << std::endl;

#define UNION_STRUCT_FIELD_TO_OFS(name) ofs << "        "#name << ": " << ptr->name << std::endl;
#define UNION_END_TO_OFS(name)          ofs << "    }"      << std::endl \
                                            << "    "#name << ": " << ptr->name << std::endl \
                                            << "}" << std::endl;
#define OFFSET_FIELD_TO_OFS(class_name, f_name, shift) << shift "                 "#f_name": " << ptr->class_name.f_name << std::endl
#define PLANE_OFFSET_TO_OFS(name) ofs << "MOS_PLANE_OFFSET "#name << std::endl \
                                                            OFFSET_FIELD_TO_OFS(name, iSurfaceOffset,)    \
                                                            OFFSET_FIELD_TO_OFS(name, iXOffset,)          \
                                                            OFFSET_FIELD_TO_OFS(name, iYOffset,)          \
                                                            OFFSET_FIELD_TO_OFS(name, iLockSurfaceOffset,);
#define RESOURCE_OFFSET_TO_OFS(name, shift) ofs << shift "MOS_RESOURCE_OFFSETS "#name << std::endl \
                                                                                OFFSET_FIELD_TO_OFS(name, BaseOffset, shift) \
                                                                                OFFSET_FIELD_TO_OFS(name, XOffset, shift)    \
                                                                                OFFSET_FIELD_TO_OFS(name, YOffset, shift);

#define FIELD_TO_OFS_8SHIFT(name) FIELD_TO_OFS(name, "        ")
MOS_STATUS MediaDebugInterface::DumpMosSpecificResourceInfoToOfs(
    PMOS_RESOURCE pOsResource,
    std::ofstream& ofs)
{
    MEDIA_DEBUG_FUNCTION_ENTER;
    if (Mos_ResourceIsNull(pOsResource))
    {
        MEDIA_DEBUG_ASSERTMESSAGE("pOsResource is null");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    PMOS_RESOURCE ptr = pOsResource;
    ofs << "MOS_RESOURCE:" << std::endl;
#if !defined(LINUX) || defined(WDDM_LINUX)
    FIELD_TO_OFS(RunTimeHandle, );
#else
    FIELD_TO_OFS(iWidth, );
    FIELD_TO_OFS(iHeight, );
    FIELD_TO_OFS(iSize, );
    FIELD_TO_OFS(iPitch, );
    FIELD_TO_OFS(iDepth, );
    FIELD_TO_OFS(Format, );
    FIELD_TO_OFS(iCount, );
    FIELD_TO_OFS(dwGfxAddress, );
    FIELD_TO_OFS(isTiled, );
    FIELD_TO_OFS(TileType, );
    FIELD_TO_OFS(bMapped, );

    EMPTY_TO_OFS();
    FIELD_TO_OFS(bo->size, );
    FIELD_TO_OFS(bo->align, );
    FIELD_TO_OFS(bo->offset, );
    FIELD_TO_OFS(bo->handle, );
    FIELD_TO_OFS(bo->offset64, );
    FIELD_TO_OFS(bo->aux_mapped, );
#endif
    ofs << "iAllocationIndex[MOS_GPU_CONTEXT_MAX == " << (uint32_t)MOS_GPU_CONTEXT_MAX << "]: {";
    for (int i = 0; i < MOS_GPU_CONTEXT_MAX; ++i)
        ofs << ptr->iAllocationIndex[i] << ", ";
    ofs << "}" << std::endl;

    {
        PGMM_RESOURCE_INFO ptr = pOsResource->pGmmResInfo;
        EMPTY_TO_OFS();
        ofs << "GMM_RESOURCE_INFO:" << std::endl;
        FIELD_TO_OFS(GetResourceType(), );
        FIELD_TO_OFS(GetResourceFormat(), );
        FIELD_TO_OFS(GetBitsPerPixel(), );

        {
            GMM_RESOURCE_FLAG flags = pOsResource->pGmmResInfo->GetResFlags();
            GMM_RESOURCE_FLAG* ptr = &flags;

            EMPTY_TO_OFS();
            ofs << "    GMM_RESOURCE_FLAG:" << std::endl;
            FIELD_TO_OFS_8SHIFT(Gpu.CameraCapture);
            FIELD_TO_OFS_8SHIFT(Gpu.CCS);
            FIELD_TO_OFS_8SHIFT(Gpu.ColorDiscard);
            FIELD_TO_OFS_8SHIFT(Gpu.ColorSeparation);
            FIELD_TO_OFS_8SHIFT(Gpu.ColorSeparationRGBX);
            FIELD_TO_OFS_8SHIFT(Gpu.Constant);
            FIELD_TO_OFS_8SHIFT(Gpu.Depth);
            FIELD_TO_OFS_8SHIFT(Gpu.FlipChain);
            FIELD_TO_OFS_8SHIFT(Gpu.FlipChainPreferred);
            FIELD_TO_OFS_8SHIFT(Gpu.HistoryBuffer);
            FIELD_TO_OFS_8SHIFT(Gpu.HiZ);
            FIELD_TO_OFS_8SHIFT(Gpu.Index);
            FIELD_TO_OFS_8SHIFT(Gpu.IndirectClearColor);
            FIELD_TO_OFS_8SHIFT(Gpu.InstructionFlat);
            FIELD_TO_OFS_8SHIFT(Gpu.InterlacedScan);
            FIELD_TO_OFS_8SHIFT(Gpu.MCS);
            FIELD_TO_OFS_8SHIFT(Gpu.MMC);
            FIELD_TO_OFS_8SHIFT(Gpu.MotionComp);
            FIELD_TO_OFS_8SHIFT(Gpu.NoRestriction);
            FIELD_TO_OFS_8SHIFT(Gpu.Overlay);
            FIELD_TO_OFS_8SHIFT(Gpu.Presentable);
            FIELD_TO_OFS_8SHIFT(Gpu.ProceduralTexture);
            FIELD_TO_OFS_8SHIFT(Gpu.Query);
            FIELD_TO_OFS_8SHIFT(Gpu.RenderTarget);
            FIELD_TO_OFS_8SHIFT(Gpu.S3d);
            FIELD_TO_OFS_8SHIFT(Gpu.S3dDx);
            FIELD_TO_OFS_8SHIFT(Gpu.__S3dNonPacked);
            FIELD_TO_OFS_8SHIFT(Gpu.ScratchFlat);
            FIELD_TO_OFS_8SHIFT(Gpu.SeparateStencil);
            FIELD_TO_OFS_8SHIFT(Gpu.State);
            FIELD_TO_OFS_8SHIFT(Gpu.Stream);
            FIELD_TO_OFS_8SHIFT(Gpu.TextApi);
            FIELD_TO_OFS_8SHIFT(Gpu.Texture);
            FIELD_TO_OFS_8SHIFT(Gpu.TiledResource);
            FIELD_TO_OFS_8SHIFT(Gpu.TilePool);
            FIELD_TO_OFS_8SHIFT(Gpu.UnifiedAuxSurface);
            FIELD_TO_OFS_8SHIFT(Gpu.Vertex);
            FIELD_TO_OFS_8SHIFT(Gpu.Video);
            FIELD_TO_OFS_8SHIFT(Gpu.__NonMsaaTileXCcs);
            FIELD_TO_OFS_8SHIFT(Gpu.__NonMsaaTileYCcs);
            FIELD_TO_OFS_8SHIFT(Gpu.__MsaaTileMcs);
            FIELD_TO_OFS_8SHIFT(Gpu.__NonMsaaLinearCCS);
            FIELD_TO_OFS_8SHIFT(Gpu.__Remaining);

            EMPTY_TO_OFS();
            FIELD_TO_OFS_8SHIFT(Info.AllowVirtualPadding);
            FIELD_TO_OFS_8SHIFT(Info.BigPage);
            FIELD_TO_OFS_8SHIFT(Info.Cacheable);
            FIELD_TO_OFS_8SHIFT(Info.ContigPhysMemoryForiDART);
            FIELD_TO_OFS_8SHIFT(Info.CornerTexelMode);
            FIELD_TO_OFS_8SHIFT(Info.ExistingSysMem);
            FIELD_TO_OFS_8SHIFT(Info.ForceResidency);
            FIELD_TO_OFS_8SHIFT(Info.Gfdt);
            FIELD_TO_OFS_8SHIFT(Info.GttMapType);
            FIELD_TO_OFS_8SHIFT(Info.HardwareProtected);
            FIELD_TO_OFS_8SHIFT(Info.KernelModeMapped);
            FIELD_TO_OFS_8SHIFT(Info.LayoutBelow);
            FIELD_TO_OFS_8SHIFT(Info.LayoutMono);
            FIELD_TO_OFS_8SHIFT(Info.LayoutRight);
            FIELD_TO_OFS_8SHIFT(Info.LocalOnly);
            FIELD_TO_OFS_8SHIFT(Info.Linear);
            FIELD_TO_OFS_8SHIFT(Info.MediaCompressed);
            FIELD_TO_OFS_8SHIFT(Info.NoOptimizationPadding);
            FIELD_TO_OFS_8SHIFT(Info.NoPhysMemory);
            FIELD_TO_OFS_8SHIFT(Info.NotLockable);
            FIELD_TO_OFS_8SHIFT(Info.NonLocalOnly);
            FIELD_TO_OFS_8SHIFT(Info.StdSwizzle);
            FIELD_TO_OFS_8SHIFT(Info.PseudoStdSwizzle);
            FIELD_TO_OFS_8SHIFT(Info.Undefined64KBSwizzle);
            FIELD_TO_OFS_8SHIFT(Info.RedecribedPlanes);
            FIELD_TO_OFS_8SHIFT(Info.RenderCompressed);
            FIELD_TO_OFS_8SHIFT(Info.Rotated);
            FIELD_TO_OFS_8SHIFT(Info.Shared);
            FIELD_TO_OFS_8SHIFT(Info.SoftwareProtected);
            FIELD_TO_OFS_8SHIFT(Info.SVM);
#if !defined(LINUX)
            FIELD_TO_OFS_8SHIFT(Info.Tile4);
            FIELD_TO_OFS_8SHIFT(Info.Tile64);
#endif
            FIELD_TO_OFS_8SHIFT(Info.TiledW);
            FIELD_TO_OFS_8SHIFT(Info.TiledX);
            FIELD_TO_OFS_8SHIFT(Info.TiledY);
            FIELD_TO_OFS_8SHIFT(Info.TiledYf);
            FIELD_TO_OFS_8SHIFT(Info.TiledYs);
            FIELD_TO_OFS_8SHIFT(Info.XAdapter);
            FIELD_TO_OFS_8SHIFT(Info.__PreallocatedResInfo);
#if !defined(LINUX)
            FIELD_TO_OFS_8SHIFT(Info.LMemBarPreferred);
            FIELD_TO_OFS_8SHIFT(Info.LMemBarOrNonlocalOnly);
            FIELD_TO_OFS_8SHIFT(Info.LMemBarIndifferent);
            FIELD_TO_OFS_8SHIFT(Info.CpuVisibleOnDemand);
            FIELD_TO_OFS_8SHIFT(Info.DwmFbrResource);
#endif

            EMPTY_TO_OFS();
            FIELD_TO_OFS_8SHIFT(Wa.GTMfx2ndLevelBatchRingSizeAlign);
            FIELD_TO_OFS_8SHIFT(Wa.ILKNeedAvcMprRowStore32KAlign);
            FIELD_TO_OFS_8SHIFT(Wa.ILKNeedAvcDmvBuffer32KAlign);
            FIELD_TO_OFS_8SHIFT(Wa.NoBufferSamplerPadding);
            FIELD_TO_OFS_8SHIFT(Wa.NoLegacyPlanarLinearVideoRestrictions);
            FIELD_TO_OFS_8SHIFT(Wa.CHVAstcSkipVirtualMips);
            FIELD_TO_OFS_8SHIFT(Wa.DisablePackedMipTail);
            FIELD_TO_OFS_8SHIFT(Wa.__ForceOtherHVALIGN4);
            FIELD_TO_OFS_8SHIFT(Wa.DisableDisplayCcsClearColor);
            FIELD_TO_OFS_8SHIFT(Wa.DisableDisplayCcsCompression);
            FIELD_TO_OFS_8SHIFT(Wa.PreGen12FastClearOnly);
#if !defined(LINUX)
            FIELD_TO_OFS_8SHIFT(Wa.ForceStdAllocAlign);
#endif
        }

        FIELD_TO_OFS(GetBaseWidth(), );
        FIELD_TO_OFS(GetBaseHeight(), );
        FIELD_TO_OFS(GetBaseDepth(), );
        FIELD_TO_OFS(GetMaxLod(), );
        FIELD_TO_OFS(GetArraySize(), );
        FIELD_TO_OFS(GetSetCpSurfTag(0, 0), );
        FIELD_TO_OFS(GetCachePolicyUsage(), );
        FIELD_TO_OFS(GetNumSamples(), );
        FIELD_TO_OFS(GetSamplePattern(), );

        EMPTY_TO_OFS();
        FIELD_TO_OFS(IsArraySpacingSingleLod(), );
        FIELD_TO_OFS(GetBaseAlignment(), );
        FIELD_TO_OFS(GetHAlign(), );
        FIELD_TO_OFS(GetVAlign(), );
        FIELD_TO_OFS(GetMipTailStartLodSurfaceState(), );
        FIELD_TO_OFS(GetQPitch(), );

        EMPTY_TO_OFS();
        ofs << "MmcMode[GMM_MAX_MMC_INDEX == " << GMM_MAX_MMC_INDEX << "]: {";
        for (uint32_t i = 0; i < GMM_MAX_MMC_INDEX; ++i)
            ofs << (uint32_t)ptr->GetMmcMode(i) << ", ";
        ofs << "}" << std::endl;

        ofs << "MmcHint[GMM_MAX_MMC_INDEX == " << GMM_MAX_MMC_INDEX << "]: {";
        for (uint32_t i = 0; i < GMM_MAX_MMC_INDEX; ++i)
            ofs << (uint32_t)ptr->GetMmcHint(i) << ", ";
        ofs << "}" << std::endl;

        FIELD_TO_OFS(GetRenderPitch(), );
        FIELD_TO_OFS(GetSizeMainSurface(), );
        FIELD_TO_OFS(GmmGetTileMode(), );

#if !defined(LINUX)
        EMPTY_TO_OFS();
        FIELD_TO_OFS(GetMultiTileArch().Enable, );
#endif
    }

    return MOS_STATUS_SUCCESS;
}
#undef FIELD_TO_OFS_8SHIFT
#undef RESOURCE_OFFSET_TO_OFS
#undef PLANE_OFFSET_TO_OFS
#undef OFFSET_FIELD_TO_OFS
#undef UNION_END_TO_OFS
#undef UNION_STRUCT_FIELD_TO_OFS
#undef UNION_STRUCT_START_TO_OFS
#undef EMPTY_TO_OFS
#undef FIELD_TO_OFS

MOS_STATUS MediaDebugInterface::DumpBltOutput(
    PMOS_SURFACE surface,
    const char * attrName)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DeleteCfgLinkNode(uint32_t frameIdx)
{
    return m_configMgr->DeleteCfgNode(frameIdx);
}

MOS_STATUS MediaDebugInterface::ReAllocateSurface(
    PMOS_SURFACE    pSurface,
    PMOS_SURFACE    pSrcSurf,
    PCCHAR          pSurfaceName,
    MOS_GFXRES_TYPE defaultResType,
    bool            useLinearResource)
{
    MOS_ALLOC_GFXRES_PARAMS allocParams;

    MEDIA_DEBUG_ASSERT(m_osInterface);
    MEDIA_DEBUG_ASSERT(&pSurface->OsResource);

    // bCompressible should be compared with bCompressible since it is inited by bCompressible in previous call
    // TileType of surface should be compared since we need to reallocate surface if TileType changes
    if (!Mos_ResourceIsNull(&pSurface->OsResource) &&
        (pSurface->dwWidth == pSrcSurf->dwWidth) &&
        (pSurface->dwHeight == pSrcSurf->dwHeight) &&
        (pSurface->Format == pSrcSurf->Format) &&
        ((pSurface->bCompressible == pSrcSurf->bCompressible) || useLinearResource) && // omit this check as linear surface is uncompressible
        (pSurface->CompressionMode == pSrcSurf->CompressionMode) &&
        ((pSurface->TileType == pSrcSurf->TileType) ||
         (pSurface->TileType == MOS_TILE_LINEAR && useLinearResource))) // when useLinearResource no reallocation needed
    {
        MEDIA_DEBUG_VERBOSEMESSAGE("Skip to reallocate temp surface.");
        return MOS_STATUS_SUCCESS;
    }
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

#if !EMUL
    //  Need to reallocate surface according to expected tiletype instead of tiletype of the surface what we have
    if ((pSurface->OsResource.pGmmResInfo != nullptr) &&
        (pSurface->TileType == pSrcSurf->TileType))
    {
        // Reallocate but use same tile type and resource type as current
        allocParams.TileType = pSurface->OsResource.TileType;
        allocParams.Type     = defaultResType;
    }
    else
#endif
    {
        // First time allocation. Caller must specify default params
        allocParams.TileType = pSrcSurf->TileType;
        allocParams.Type     = defaultResType;
    }

    // Force to use tile linear for reallocated resource
    if (useLinearResource)
    {
        allocParams.TileType = MOS_TILE_LINEAR;
        allocParams.Type = MOS_GFXRES_2D;
    }

    allocParams.dwWidth         = pSrcSurf->dwWidth;
    allocParams.dwHeight        = pSrcSurf->dwHeight;
    allocParams.Format          = pSrcSurf->Format;
    allocParams.bIsCompressible = pSrcSurf->bCompressible;
    allocParams.CompressionMode = pSrcSurf->CompressionMode;
    allocParams.pBufName        = pSurfaceName;
    allocParams.dwArraySize     = 1;

    // Delete resource if already allocated
    m_osInterface->pfnFreeResource(m_osInterface, &(pSurface->OsResource));

    // Allocate surface
    CODECHAL_PUBLIC_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        &pSurface->OsResource));

    if (!m_osInterface->apoMosEnabled && !m_osInterface->apoMosForLegacyRuntime)
    {
        MOS_SURFACE details;
        MOS_ZeroMemory(&details, sizeof(details));
        details.Format = Format_Invalid;

        MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, &pSurface->OsResource, &details));

        pSurface->dwWidth                     = details.dwWidth;
        pSurface->dwHeight                    = details.dwHeight;
        pSurface->dwPitch                     = details.dwPitch;
        pSurface->dwDepth                     = details.dwDepth;
        pSurface->dwQPitch                    = details.dwQPitch;
        pSurface->bArraySpacing               = details.bArraySpacing;
        pSurface->bCompressible               = details.bCompressible;
        pSurface->CompressionMode             = details.CompressionMode;
        pSurface->bIsCompressed               = details.bIsCompressed;
        pSurface->Format                      = details.Format;
        pSurface->TileType                    = details.TileType;
        pSurface->dwOffset                    = details.RenderOffset.YUV.Y.BaseOffset;
        pSurface->YPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.Y.BaseOffset;
        pSurface->YPlaneOffset.iXOffset       = details.RenderOffset.YUV.Y.XOffset;
        pSurface->YPlaneOffset.iYOffset =
            (pSurface->YPlaneOffset.iSurfaceOffset - pSurface->dwOffset) / pSurface->dwPitch +
            details.RenderOffset.YUV.Y.YOffset;
        pSurface->UPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.U.BaseOffset;
        pSurface->UPlaneOffset.iXOffset       = details.RenderOffset.YUV.U.XOffset;
        pSurface->UPlaneOffset.iYOffset =
            (pSurface->UPlaneOffset.iSurfaceOffset - pSurface->dwOffset) / pSurface->dwPitch +
            details.RenderOffset.YUV.U.YOffset;
        pSurface->UPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.U;
        pSurface->VPlaneOffset.iSurfaceOffset     = details.RenderOffset.YUV.V.BaseOffset;
        pSurface->VPlaneOffset.iXOffset           = details.RenderOffset.YUV.V.XOffset;
        pSurface->VPlaneOffset.iYOffset =
            (pSurface->VPlaneOffset.iSurfaceOffset - pSurface->dwOffset) / pSurface->dwPitch +
            details.RenderOffset.YUV.V.YOffset;
        pSurface->VPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.V;
    }
    else
    {
        pSurface->Format = Format_Invalid;
        MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, &pSurface->OsResource, pSurface));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::Dump2DBufferInBinary(
    uint8_t *data,
    uint32_t width,
    uint32_t height,
    uint32_t pitch)
{
    MEDIA_DEBUG_CHK_NULL(data);

    const char *filePath = m_outputFileName.c_str();

    if (width == 0 || height == 0 || pitch == 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    std::ofstream ofs(filePath, std::ios_base::out | std::ios_base::binary);
    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }

    for (uint32_t h = 0; h < height; h++)
    {
        ofs.write((char *)data, width);
        data += pitch;
    }

    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpBufferInBinary(uint8_t *data, uint32_t size)
{
    MEDIA_DEBUG_CHK_NULL(data);

    const char *filePath = m_outputFileName.c_str();

    if (size == 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    std::ofstream ofs(filePath, std::ios_base::out | std::ios_base::binary);
    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }

    ofs.write((char *)data, size);
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::DumpBufferInHexDwords(uint8_t *data, uint32_t size)
{
    MEDIA_DEBUG_CHK_NULL(data);

    const char *filePath = m_outputFileName.c_str();

    if (size == 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    std::ofstream ofs(filePath);

    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }

    uint32_t dwordSize  = size / sizeof(uint32_t);
    uint32_t remainSize = size % sizeof(uint32_t);

    uint32_t *dwordData = (uint32_t *)data;
    uint32_t  i;
    for (i = 0; i < dwordSize; i++)
    {
        ofs << std::hex << std::setw(8) << std::setfill('0') << +dwordData[i]
            << ((i + 1) % 4 == 0 ? "\n" : " ");
    }

    if (remainSize > 0)
    {
        uint32_t lastWord = dwordData[i] & (0xFFFFFFFF << ((8 - remainSize * 2) * 4));
        ofs << std::hex << std::setw(8) << std::setfill('0') << +lastWord << std::endl;
    }

    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::InitCRCTable(uint32_t crcTable[256])
{
    uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t c = i;
        for (size_t j = 0; j < 8; j++)
        {
            if (c & 1)
            {
                c = polynomial ^ (c >> 1);
            }
            else
            {
                c >>= 1;
            }
        }
        crcTable[i] = c;
    }
    return MOS_STATUS_SUCCESS;
}

uint32_t MediaDebugInterface::CalculateCRC(const void *buf, size_t len, uint32_t initial)
{
    uint32_t       c = initial ^ 0xFFFFFFFF;
    const uint8_t *u = static_cast<const uint8_t *>(buf);
    for (size_t i = 0; i < len; ++i)
    {
        c = m_crcTable[(c ^ u[i]) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFF;
}

#endif  // USE_MEDIA_DEBUG_TOOL
