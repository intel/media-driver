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
//! \file      cm_surface_state.h
//! \brief     Contains Class CmSurfaceState  definitions 
//!
#pragma once

#include "cm_def.h"
#include "cm_hal.h"
#include "cm_device_rt.h"

class CmSSH;

class CmSurfaceState
{
public:
    CmSurfaceState(CM_HAL_STATE *cmhal);
    virtual ~CmSurfaceState() {}
    CM_RETURN_CODE Initialize(MOS_RESOURCE *resource);
    // Query apis for SSH
    virtual uint32_t GetNumBte() {return 1; }
    virtual uint32_t GetNumPlane() {return 1; }
    virtual int isAVS() {return 0; }
    virtual int isAVS(int surfIdx) {return 0; }
    virtual uint8_t *GetSurfaceState(int index) = 0; 
    virtual uint32_t GetSurfaceOffset(int index) {return 0; }
    virtual bool IsRenderTarget() {return true; }
    virtual MOS_RESOURCE *GetResource(uint32_t index)
    {
        if (m_resource == nullptr)
            return nullptr;
        m_resourceData = *m_resource;
        return &m_resourceData; 
    }

    virtual MOS_MEMCOMP_STATE GetMmcState(int index) {return MOS_MEMCOMP_DISABLED; }

    inline void SetMemoryObjectControl(uint16_t moc) {m_memoryObjectControl = moc; }
    inline MOS_RESOURCE *GetResource()
    {
        if (m_resource == nullptr)
            return nullptr;
        m_resourceData = *m_resource;
        return &m_resourceData; 
    }
    inline void Recorded(CmSSH *ssh, uint32_t btIdx, uint32_t bteIdx, uint32_t ssIdx)
    {
        m_ssh = ssh;
        m_btIdx = btIdx;
        m_bteIdx = bteIdx;
        m_ssIdx = ssIdx;
    }
    inline void GetRecordedInfo(CmSSH **ssh, int32_t *btIdx, int32_t *bteIdx, int32_t *ssIdx)
    {
        *ssh = m_ssh;
        *btIdx = m_btIdx;
        *bteIdx = m_bteIdx;
        *ssIdx = m_ssIdx;
    }

protected:
    uint32_t GetCacheabilityControl();

    CM_HAL_STATE *m_cmhal;
    RENDERHAL_INTERFACE *m_renderhal;

    // resource
    MOS_RESOURCE *m_resource;
    MOS_RESOURCE m_resourceData;
    
    // surface state related parameters
    uint16_t m_memoryObjectControl;

    // record in ssh
    CmSSH *m_ssh;
    int m_btIdx;
    int m_bteIdx;
    int m_ssIdx;

};


class CmSurfaceState2Dor3D : public CmSurfaceState
{
public:
    CmSurfaceState2Dor3D(CM_HAL_STATE *cmhal);
    CM_RETURN_CODE Initialize(MOS_RESOURCE *resource, bool isAvs = false, bool isSampler = false);
    MOS_STATUS GenerateSurfaceState(CM_HAL_SURFACE2D_SURFACE_STATE_PARAM *param = nullptr);

    // global parameters to generate default surface state
    inline void SetRenderTarget(bool flag) {m_isRenderTarget = flag; }
    inline void SetFrameType(CM_FRAME_TYPE frameType) {m_frameType = frameType; }
    inline void SetFormat(MOS_FORMAT format) { m_format = format; }
    inline void SetUserDimension(uint32_t width, uint32_t height, uint32_t depth)
    {
        m_userWidth = width;
        m_userHeight = height;
        m_userDepth = depth;
    }
    inline void SetRotationFlag(uint32_t rotation) { m_rotation = rotation; }
    inline void SetChromaSitting(int chromaSitting){ m_chromaSitting = chromaSitting; }

    // Query apis for SSH
    uint32_t GetNumPlane() {return m_numPlane; }
    uint32_t GetNumBte() {return m_numPlane; }
    int isAVS() {return m_avsUsed; }
    int isAVS(int surfIdx) {return m_planeParams[surfIdx].isAvs; }
    uint8_t *GetSurfaceState(int index) {return m_cmds + index * m_maxStateSize; }
    uint32_t GetSurfaceOffset(int index) {return m_surfOffsets[index]; }
    bool IsRenderTarget() {return m_isRenderTarget; }
    MOS_MEMCOMP_STATE GetMmcState(int index) {return m_mmcState; }

protected:
    struct CmSurfaceParamsPerPlane
    {
        uint32_t isAvs;
        uint32_t width;
        uint32_t height;
        uint32_t format;
        uint32_t pitch;
        uint32_t xoffset;
        uint32_t yoffset;
        uint8_t planeID;
    };
    MOS_STATUS RefreshSurfaceInfo(CM_HAL_SURFACE2D_SURFACE_STATE_PARAM *param = nullptr);
    MOS_STATUS UpdateSurfaceState();
    MOS_STATUS SetPerPlaneParam();
    int GetPlaneDefinitionRender();
    int GetPlaneDefinitionMedia();
    void GetDIUVOffSet();
    bool IsFormatMMCSupported(MOS_FORMAT format);
    uint8_t GetDirection();

    // surface state related parameters
    MOS_FORMAT m_format;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_depth;
    uint32_t m_pitch;
    uint32_t m_qPitch;
    uint32_t m_tile;
    MOS_TILE_MODE_GMM m_tileModeGMM;
    bool     m_bGMMTileEnabled;
    uint32_t m_isCompressed;
    uint32_t m_compressionMode;
    MOS_MEMCOMP_STATE m_mmcState;
    uint32_t m_compressionFormat;
    uint32_t m_rotation;
    int m_chromaSitting;
    uint32_t m_surfaceXOffset;
    uint32_t m_surfaceYOffset;
    CM_FRAME_TYPE m_frameType;
    bool m_isRenderTarget;
    CmSurfaceParamsPerPlane m_planeParams[4];
    uint32_t m_paletteID;

    // user defined dimension
    uint32_t m_userWidth;
    uint32_t m_userHeight;
    uint32_t m_userDepth;

    // offsets per plane
    uint32_t m_surfOffsets[4]; // these offsets shouldn't be changed by users, so not include them in CmSurfaceStateParamsPerPlane
    uint32_t m_xOffsets[4];
    uint32_t m_yOffsets[4];
    uint32_t m_lockOffsets[4];

    // surface state commands
    uint32_t m_maxStateSize;
    uint32_t m_avsUsed;
    uint32_t m_numPlane;
    uint8_t m_cmds[256];

    bool m_pixelPitch;
    bool m_isWidthInDWord;
    bool m_isVme;

    // avs parameters
    uint8_t m_direction;
    bool m_isHalfPitchChroma;
    bool m_isInterleaveChrome;
    uint16_t m_uXOffset;
    uint16_t m_uYOffset;
    uint16_t m_vXOffset;
    uint16_t m_vYOffset;
    bool m_isVaSurface;

};

class CmSurfaceStateBuffer : public CmSurfaceState
{
public:
    CmSurfaceStateBuffer(CM_HAL_STATE *cmhal);
    CM_RETURN_CODE Initialize(MOS_RESOURCE *resource, uint32_t size);
    MOS_STATUS GenerateSurfaceState(CM_HAL_BUFFER_SURFACE_STATE_ENTRY *param = nullptr);
    uint8_t *GetSurfaceState(int index) {return m_cmds; }
    uint32_t GetSurfaceOffset(int index) {return m_offset; }

protected:
    uint32_t m_size;
    uint32_t m_offset;
    
    uint8_t m_cmds[64];
    
};

class CmSurfaceStateVME : public CmSurfaceState
{
public:
    CmSurfaceStateVME(CM_HAL_STATE *cmhal);
    CM_RETURN_CODE Initialize(CM_HAL_VME_ARG_VALUE *vmeArg);
    uint8_t *GetSurfaceState(int index);
    uint32_t GetSurfaceOffset(int index) {return m_offsets[index]; }
    MOS_MEMCOMP_STATE GetMmcState(int index) {return m_mmcStates[index]; }
    MOS_RESOURCE *GetResource(uint32_t index) ;
    uint32_t GetNumPlane() {return m_forwardCount+m_backwardCount+1; }
    uint32_t GetNumBte() {return m_numBte; }
    int isAVS() {return 1; }
    int isAVS(int surfIdx) {return 1; }

protected:
    int GetCmHalSurfaceIndex(uint32_t index);
    
    uint32_t m_numBte;
    uint32_t m_forwardCount;
    uint32_t m_backwardCount;
    uint32_t m_curIndex;
    uint32_t *m_forwardIndexes;
    uint32_t *m_backwardIndexes;
    CM_HAL_SURFACE2D_SURFACE_STATE_PARAM m_surf2DParam;
    uint32_t m_offsets[CM_VME_FORWARD_ARRAY_LENGTH + CM_VME_BACKWARD_ARRAY_LENGTH + 1];
    MOS_MEMCOMP_STATE m_mmcStates[CM_VME_FORWARD_ARRAY_LENGTH + CM_VME_BACKWARD_ARRAY_LENGTH + 1];
};

