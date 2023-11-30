/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     media_debug_interface.h
//! \brief    Defines the debug interface shared by all of Media.
//! \details  The debug interface dumps output from Media based on input config file.
//!
#ifndef __MEDIA_DEBUG_INTERFACE_H__
#define __MEDIA_DEBUG_INTERFACE_H__

#if ((_DEBUG || _RELEASE_INTERNAL))
#define USE_MEDIA_DEBUG_TOOL 1
#define MEDIA_DEBUG_TOOL(expr) expr;

#include "mhw_utilities.h"
#include "mhw_state_heap.h"
#include "mhw_cp_interface.h"
#include "media_debug_config_manager.h"
#include "media_debug_utils.h"
#include "media_copy_wrapper.h"
#include <sstream>
#include <fstream>
#include <functional>

using GoldenReferences = std::vector<std::vector<uint32_t>>;
class MediaDebugInterface
{
public:
    MediaDebugInterface();
    virtual ~MediaDebugInterface();

    MOS_STATUS InitDumpLocation();

    MOS_STATUS SetOutputFilePath();

    virtual MOS_STATUS DumpUncompressedYUVSurface(
        PMOS_SURFACE           surface);

    MOS_STATUS DumpMosSpecificResourceInfoToOfs(
        PMOS_RESOURCE  pOsResource,
        std::ofstream &ofs);

    MOS_STATUS DumpBufferInHexDwords(
        uint8_t *data,
        uint32_t size);

    virtual MOS_STATUS DumpBltOutput(
        PMOS_SURFACE surface,
        const char * attrName);

    MOS_STATUS DeleteCfgLinkNode(uint32_t frameIdx);

    virtual MOS_STATUS InitCRCTable(uint32_t crcTable[256]);
    
    virtual uint32_t CalculateCRC(
        const void *buf,
        size_t      len,
        uint32_t    initial=0 );

    virtual MOS_STATUS CaptureGoldenReference(uint8_t *buf, uint32_t size, uint32_t hwCrcValue=0);

    virtual MOS_STATUS DumpGoldenReference();

    virtual MOS_STATUS LoadGoldenReference();

    MOS_STATUS SetSWCrcMode(bool swCrc);

    MOS_STATUS SubmitDummyWorkload(MOS_COMMAND_BUFFER *pcmdBuffer, int32_t bNullRendering);

    MOS_STATUS LockResource(uint32_t *semaData, PMOS_RESOURCE reSemaphore);

    MOS_STATUS DumpToFile(const GoldenReferences &goldenReferences);

    bool       IsHwDebugHooksEnable() { return m_enableHwDebugHooks; }

    void       PackGoldenReferences(std::initializer_list<std::vector<uint32_t>> goldenReferences);

    MOS_STATUS FillSemaResource(std::vector<uint32_t *> &vResource, std::vector<uint32_t> &data);

    MOS_STATUS LockSemaResource(std::vector<uint32_t *> &vSemaData, std::vector<MOS_RESOURCE> &vResource);

    int32_t GetStopFrameNumber() { return m_stopFrameNumber; }

    std::vector<uint32_t> GetCrcGoldenReference() {return m_crcGoldenReference; }
    std::string m_ddiFileName;
    std::string m_outputFilePath;
    std::string m_crcGoldenRefFileName;

    MOS_SURFACE       m_temp2DSurfForCopy = {};
    PMOS_INTERFACE    m_osInterface       = nullptr;
    MhwCpInterface   *m_cpInterface       = nullptr;
    void             *m_miInterface       = nullptr;

    MediaDbgFunction  m_mediafunction     = MEDIA_FUNCTION_DEFAULT;
    uint32_t          m_scaledBottomFieldOffset = 0;
    uint16_t          m_frameType               = 0;
    uint32_t          m_sliceId                 = 0;  // used for constructing debug file name
    char              m_fileName[MOS_MAX_PATH_LENGTH + 1];
    char              m_path[MOS_MAX_PATH_LENGTH + 1];
    bool              m_secondField              = false;
    bool              m_hybridPakP1              = false;
    bool              m_hybridVp8EncodeBrcEnable = false;
    bool              m_hybridVp9EncodeEnable    = false;
    bool              m_vdboxContextCreated      = false;
    bool              m_enableHwDebugHooks       = false;
    bool              m_goldenReferenceExist     = false;
    bool              m_swCRC = false;
    uint16_t          m_preIndex                 = 0;
    uint16_t          m_refIndex                 = 0;
    size_t            m_DumpInputNum             = 0;
    size_t            m_bufferDumpFrameNum       = 0;
    uint32_t          m_decodeSurfDumpFrameNum   = 0;
    uint32_t          m_streamId                 = 0;
    uint32_t          m_crcTable[256]            ={0};
    int32_t           m_stopFrameNumber          = -1;
    std::vector<uint32_t> m_crcGoldenReference   = {};
    GoldenReferences      m_goldenReferences     = {};
    uint32_t*         m_semaData                 = nullptr;
    MediaUserSettingSharedPtr m_userSettingPtr   = nullptr;

protected:
    MOS_STATUS ReAllocateSurface(
        PMOS_SURFACE    pSurface,
        PMOS_SURFACE    pSrcSurf,
        PCCHAR          pSurfaceName,
        MOS_GFXRES_TYPE DefaultResType,
        bool            useLinearResource = false);

    MOS_STATUS CopySurfaceData_Vdbox(
        uint32_t      dwDataSize,
        PMOS_RESOURCE presSourceSurface,
        PMOS_RESOURCE presCopiedSurface);

    MOS_STATUS DumpBufferInBinary(
        uint8_t *data,
        uint32_t size);

    MOS_STATUS Dump2DBufferInBinary(
        uint8_t *data,
        uint32_t width,
        uint32_t height,
        uint32_t pitch);

    virtual MOS_STATUS InitializeUserSetting() { return MOS_STATUS_SUCCESS; };

    virtual std::string SetOutputPathKey()  = 0;
    virtual std::string InitDefaultOutput() = 0;

    std::string          m_outputFileName;
    MediaDebugConfigMgr *m_configMgr = nullptr;

    MEDIA_CLASS_DEFINE_END(MediaDebugInterface)
};

#else
#define USE_MEDIA_DEBUG_TOOL 0
#define MEDIA_DEBUG_TOOL(expr) ;

#endif  // (_DEBUG || _RELEASE_INTERNAL)
#endif /* __MEDIA_DEBUG_INTERFACE_H__ */