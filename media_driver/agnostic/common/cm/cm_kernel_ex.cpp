/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file      cm_kernel_ex.cpp
//! \brief     Contains Class CmKernelEx definitions
//!

#include "cm_kernel_ex.h"
#include "cm_surface.h"
#include "cm_surface_manager.h"
#include "cm_surface_sampler8x8.h"
#include "cm_surface_sampler.h"
#include "cm_mem.h"
#include "cm_surface_2d_rt.h"
#include "cm_surface_2d_up_rt.h"
#include "cm_surface_3d_rt.h"
#include "cm_buffer_rt.h"
#include "cm_device_rt.h"
#include "cm_hal.h"
#include "cm_surface_state.h"
#include "cm_surface_state_manager.h"
#include "cm_surface_vme.h"
#include "cm_ssh.h"
#include "cm_thread_space_rt.h"
#include "cm_surface_sampler.h"
#include "cm_media_state.h"

#include "mhw_state_heap.h"

using namespace CMRT_UMD;

CmKernelEx::~CmKernelEx()
{
    if (m_dummyThreadSpace)
    {
        m_device->DestroyThreadSpace(m_dummyThreadSpace);
    }
    if (m_dummyThreadGroupSpace)
    {
        m_device->DestroyThreadGroupSpace(m_dummyThreadGroupSpace);
    }
    MOS_DeleteArray(m_indexMap);
    MOS_DeleteArray(m_flatArgs);
    MOS_DeleteArray(m_propertyIndexes);
    MOS_DeleteArray(m_cmSurfIndexes);
    MOS_DeleteArray(m_data);
    MOS_DeleteArray(m_surfaceInArg);
    MOS_DeleteArray(m_curbe);
}

int32_t CmKernelEx::Initialize(const char *kernelName, const char *options)
{
    int ret = CmKernelRT::Initialize(kernelName, options);
    if (ret != CM_SUCCESS)
    {
        return ret;
    }

    m_indexMap = MOS_NewArray(uint32_t, (m_argCount+1));
    CM_CHK_NULL_RETURN_CMERROR(m_indexMap);
    MOS_ZeroMemory(m_indexMap, (m_argCount+1)*sizeof(uint32_t));
    m_flatArgCount= 0;
    bool isGpgpuKernel = false;
    uint32_t minPayload = 0;
    for (uint32_t i = 0; i < m_argCount; i++)
    {
        if (ArgArraySupported(m_args[i].unitKind))
        {
            int numSurfaces = m_args[i].unitSize/sizeof(int);
            m_flatArgCount += numSurfaces;
        }
        else
        {
            ++m_flatArgCount;
        }

        if (!isGpgpuKernel &&
            ( m_args[i].unitKind == CM_ARGUMENT_IMPLICT_LOCALSIZE
            ||m_args[i].unitKind == CM_ARGUMENT_IMPLICT_GROUPSIZE
            ||m_args[i].unitKind == CM_ARGUMENT_IMPLICIT_LOCALID))
        {
            isGpgpuKernel = true;
        }
        if (i == 0 || (m_args[i].unitKind != CM_ARGUMENT_IMPLICIT_LOCALID && minPayload > m_args[i].unitOffsetInPayload))
        {
            minPayload = m_args[i].unitOffsetInPayload;
        }
    }

    if (!isGpgpuKernel)
    {
        minPayload = CM_PAYLOAD_OFFSET;
    }

    if (m_flatArgCount == 0)
    {
        return CM_SUCCESS;
    }

    m_flatArgs = MOS_NewArray(_CmArg, m_flatArgCount);
    CM_CHK_NULL_RETURN_CMERROR(m_flatArgs);
    MOS_ZeroMemory(m_flatArgs, m_flatArgCount * sizeof(_CmArg));
    m_propertyIndexes = MOS_NewArray(uint8_t, m_flatArgCount);
    CM_CHK_NULL_RETURN_CMERROR(m_propertyIndexes);
    MOS_ZeroMemory(m_propertyIndexes, m_flatArgCount);
    m_cmSurfIndexes = MOS_NewArray(uint32_t, m_flatArgCount);
    CM_CHK_NULL_RETURN_CMERROR(m_cmSurfIndexes);
    MOS_ZeroMemory(m_cmSurfIndexes, m_flatArgCount * sizeof(uint32_t));

    int j = 0;
    uint32_t offset = 0; //offset in the local buffer
    int localIDIndex = -1;
    for (uint32_t i = 0; i < m_argCount; i++)
    {
        if (ArgArraySupported(m_args[i].unitKind))
        {
            m_indexMap[i] = j;
            int numSurfaces = m_args[i].unitSize/sizeof(int);
            for (int k = 0; k < numSurfaces; k ++)
            {
                m_flatArgs[j].isaKind = m_args[i].unitKind;
                m_flatArgs[j].kind = m_args[i].unitKind;
                m_flatArgs[j].unitSize = sizeof(void *); // we can either store the pointer to CmSurfaceState or pointer to mos_resource here
                m_flatArgs[j].payloadOffset = m_args[i].unitOffsetInPayload + k*4 - minPayload; //each bte index has 4 bytes
                m_flatArgs[j].offset = offset;
                m_flatArgs[j].sizeInCurbe = 4;
                offset += m_flatArgs[j].unitSize;

                // update curbe size
                if (m_explicitCurbeSize < (uint32_t)(m_flatArgs[j].payloadOffset + m_flatArgs[j].sizeInCurbe))
                {
                    m_explicitCurbeSize = m_flatArgs[j].payloadOffset + m_flatArgs[j].sizeInCurbe;
                }
                ++ j;
            }
        }
        else
        {
            m_indexMap[i] = j;
            m_flatArgs[j].isaKind = m_args[i].unitKind;
            m_flatArgs[j].kind = m_args[i].unitKind;
            m_flatArgs[j].unitSize = m_args[i].unitSize;
            m_flatArgs[j].payloadOffset = m_args[i].unitOffsetInPayload - minPayload;
            m_flatArgs[j].offset = offset;
            m_flatArgs[j].sizeInCurbe = m_flatArgs[j].unitSize;
            offset += m_flatArgs[j].unitSize;

            // update curbe size
            if (m_args[i].unitKind == CM_ARGUMENT_IMPLICIT_LOCALID)
            {
                localIDIndex = j;
            }
            else
            {
                if (m_explicitCurbeSize < (uint32_t)(m_flatArgs[j].payloadOffset + m_flatArgs[j].sizeInCurbe))
                {
                    m_explicitCurbeSize = m_flatArgs[j].payloadOffset + m_flatArgs[j].sizeInCurbe;
                }
            }
            ++ j;
        }
        m_indexMap[m_argCount] = j;
    }

    // adjust the payload of local id
    if (localIDIndex >= 0)
    {
        m_flatArgs[localIDIndex].payloadOffset = MOS_ALIGN_CEIL(m_explicitCurbeSize, 32);
    }

    m_data = MOS_NewArray(uint8_t, offset);
    CM_CHK_NULL_RETURN_CMERROR(m_data);
    m_surfaceInArg = MOS_NewArray(uint8_t, offset);
    CM_CHK_NULL_RETURN_CMERROR(m_surfaceInArg);
    MOS_ZeroMemory(m_data, sizeof(uint8_t)*offset);
    MOS_ZeroMemory(m_surfaceInArg, sizeof(uint8_t)*offset);

    m_hashValue = m_kernelInfo->hashValue;

    return CM_SUCCESS;
}

MOS_STATUS CmKernelEx::AllocateCurbe()
{
    MOS_DeleteArray(m_curbe);
    if (m_explicitCurbeSize > 0)
    {
        m_curbeSize = MOS_ALIGN_CEIL(m_explicitCurbeSize, 64);
        m_curbeSizePerThread = m_curbeSize;
        m_curbeSizeCrossThread = 0;
        m_curbe = MOS_NewArray(uint8_t, m_curbeSize);
        CM_CHK_NULL_RETURN_MOSERROR(m_curbe);
        MOS_ZeroMemory(m_curbe, m_curbeSize);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmKernelEx::AllocateCurbeAndFillImplicitArgs(CmThreadGroupSpace *globalGroupSpace)
{
    CmThreadGroupSpace *tgs = (globalGroupSpace == nullptr)?m_threadGroupSpace:globalGroupSpace;

    uint32_t thrdSpaceWidth = 0;
    uint32_t thrdSpaceHeight = 0;
    uint32_t thrdSpaceDepth = 0;
    uint32_t grpSpaceWidth = 0;
    uint32_t grpSpaceHeight = 0;
    uint32_t grpSpaceDepth = 0;

    if (tgs)
    {
        tgs->GetThreadGroupSpaceSize(thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth);
    }

    MOS_DeleteArray(m_curbe);
    m_curbeSizePerThread = (m_explicitCurbeSize%32 == 4)? 64:32;
    m_curbeSizeCrossThread = MOS_ALIGN_CEIL(m_explicitCurbeSize, 32);
    m_curbeSize = m_curbeSizeCrossThread + m_curbeSizePerThread * thrdSpaceWidth * thrdSpaceHeight * thrdSpaceDepth;
    m_curbeSize = MOS_ALIGN_CEIL(m_curbeSize, 64);
    m_curbe = MOS_NewArray(uint8_t, m_curbeSize);
    CM_CHK_NULL_RETURN_MOSERROR(m_curbe);
    MOS_ZeroMemory(m_curbe, m_curbeSize);

    int localIdPayload = -1;
    int groupSizePayload = -1;
    int localSizePayload = -1;

    for (uint32_t i = 0; i < m_flatArgCount; i++)
    {
        if (m_flatArgs[i].kind == ARG_KIND_IMPLICT_LOCALSIZE)
            localSizePayload = m_flatArgs[i].payloadOffset;
        if (m_flatArgs[i].kind == ARG_KIND_IMPLICT_GROUPSIZE)
            groupSizePayload = m_flatArgs[i].payloadOffset;
        if (m_flatArgs[i].kind == ARG_KIND_IMPLICIT_LOCALID)
            localIdPayload = m_flatArgs[i].payloadOffset;
    }

    // set group size implicit args
    if (groupSizePayload >= 0)
    {
        *(uint32_t *)(m_curbe + groupSizePayload) = grpSpaceWidth;
        *(uint32_t *)(m_curbe + groupSizePayload + 4) = grpSpaceHeight;
        *(uint32_t *)(m_curbe + groupSizePayload + 8) = grpSpaceDepth;
    }

    // set local size implicit args
    if (localSizePayload >= 0)
    {
        *(uint32_t *)(m_curbe + localSizePayload) = thrdSpaceWidth;
        *(uint32_t *)(m_curbe + localSizePayload + 4) = thrdSpaceHeight;
        *(uint32_t *)(m_curbe + localSizePayload + 8) = thrdSpaceDepth;
    }

    // set local id data per thread
    if (localIdPayload >= 0)
    {
        int offset = localIdPayload;
        for (uint32_t idZ = 0; idZ < thrdSpaceDepth; idZ++)
        {
            for (uint32_t idY = 0; idY < thrdSpaceHeight; idY++)
            {
                for (uint32_t idX = 0; idX < thrdSpaceWidth; idX++)
                {
                    *(uint32_t *)(m_curbe + offset) = idX;
                    *(uint32_t *)(m_curbe + offset + 4) = idY;
                    *(uint32_t *)(m_curbe + offset + 8) = idZ;
                    offset += m_curbeSizePerThread;
                }
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

bool CmKernelEx::IsSurface(uint16_t kind)
{
    switch (kind)
    {
        case ARG_KIND_SURFACE:
        case ARG_KIND_SURFACE_1D:
        case ARG_KIND_SURFACE_2D:
        case ARG_KIND_SURFACE_2D_UP:
        case ARG_KIND_SURFACE_SAMPLER:
        case ARG_KIND_SURFACE2DUP_SAMPLER:
        case ARG_KIND_SURFACE_3D:
        case ARG_KIND_SURFACE_SAMPLER8X8_AVS:
        case ARG_KIND_SURFACE_SAMPLER8X8_VA:
        case ARG_KIND_SURFACE_2D_SCOREBOARD:
        case ARG_KIND_STATE_BUFFER:
        case ARG_KIND_SURFACE_VME:
            return true;
        default:
            return false;
    }
    return false;
}

int32_t CmKernelEx::SetKernelArg(uint32_t index, size_t size, const void * value)
{
    if (!m_blCreatingGPUCopyKernel) // gpucopy kernels only executed by fastpath, no need to set legacy kernels
    {
        CmKernelRT::SetKernelArg(index, size, value);
    }
    if( index >= m_argCount )
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg count.");
        return CM_INVALID_ARG_INDEX;

    }

    if( !value)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg value.");
        return CM_INVALID_ARG_VALUE;
    }

    if( size == 0)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
        return CM_INVALID_ARG_SIZE;
    }

    uint32_t start = m_indexMap[index];
    uint32_t len = m_indexMap[index + 1] - start;

    if (IsSurface(m_flatArgs[start].isaKind))
    {
        CMRT_UMD::SurfaceIndex *surfIndexes = (CMRT_UMD::SurfaceIndex *)value;
        if (surfIndexes == (CMRT_UMD::SurfaceIndex *)CM_NULL_SURFACE)
        {
            for (uint32_t i = 0; i < len; i++)
            {
                *(void **)(m_data + m_flatArgs[start + i].offset) = nullptr;
                *(void **)(m_surfaceInArg + m_flatArgs[start + i].offset) = nullptr;
                m_flatArgs[start + i].isSet = true;
            }
            return CM_SUCCESS;
        }
        // sanity check
        if (len * sizeof(CMRT_UMD::SurfaceIndex) != size)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
            return CM_INVALID_ARG_SIZE;
        }

        for (uint32_t i = 0; i < len; i++)
        {
            uint32_t index = surfIndexes[i].get_data();

            m_flatArgs[start + i].isSet = true;
            if (index == CM_NULL_SURFACE)
            {
                *(void **)(m_data + m_flatArgs[start+i].offset) = nullptr;
                *(void **)(m_surfaceInArg + m_flatArgs[start+i].offset) = nullptr;
            }
            else
            {
                CmSurface* surface = nullptr;
                m_surfaceMgr->GetSurface(index, surface);
                if (nullptr == surface)
                {
                    *(void **)(m_data + m_flatArgs[start+i].offset) = nullptr;
                    *(void **)(m_surfaceInArg + m_flatArgs[start+i].offset) = nullptr;
                }
                else
                {
                    m_flatArgs[start + i].kind = ToArgKind(surface);

                    // get the CmSurfaceState from the surface index, this will be changed if surfmgr optimized
                    // most likely, this will be moved to CmSurface
                    CmSurfaceState *temp = GetSurfaceState(surface, index);
                    *(CmSurfaceState **)(m_data + m_flatArgs[start + i].offset) = temp;
                    *(CmSurface **)(m_surfaceInArg + m_flatArgs[start+i].offset) = surface;
                    m_propertyIndexes[start + i] = surface->GetPropertyIndex();
                    m_cmSurfIndexes[start + i] = index;
                }
            }
        }
    }
    else if (m_flatArgs[start].isaKind == ARG_KIND_SAMPLER) // only support 3D sampler and AVS sampler in fastpath
    {
        CMRT_UMD::SamplerIndex *samplerIndexes = (CMRT_UMD::SamplerIndex *)value;
        // sanity check
        if (len * sizeof(CMRT_UMD::SurfaceIndex) != size)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
            return CM_INVALID_ARG_SIZE;
        }

        for (uint32_t i = 0; i < len; i++)
        {
            uint32_t index = samplerIndexes[i].get_data();
            MHW_SAMPLER_STATE_PARAM *temp = (MHW_SAMPLER_STATE_PARAM *)GetSamplerParam(index);
            *(MHW_SAMPLER_STATE_PARAM **)(m_data + m_flatArgs[start + i].offset) = temp;
        }
    }
    else
    {
        if (size != m_flatArgs[start].unitSize)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
            return CM_INVALID_ARG_SIZE;
        }
        CmSafeMemCopy((void *)(m_data + m_flatArgs[start].offset), value, size);
    }
    return CM_SUCCESS;
}

CM_ARG_KIND CmKernelEx::ToArgKind(CmSurface *surface)
{
    switch(surface->Type())
    {
        case CM_ENUM_CLASS_TYPE_CMBUFFER_RT:
            return ARG_KIND_SURFACE_1D;
        case CM_ENUM_CLASS_TYPE_CMSURFACE2D:
            return ARG_KIND_SURFACE_2D;
        case CM_ENUM_CLASS_TYPE_CMSURFACE2DUP:
            return ARG_KIND_SURFACE_2D_UP;
        case CM_ENUM_CLASS_TYPE_CMSURFACE3D:
            return ARG_KIND_SURFACE_3D;
        case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER:
        {
            CmSurfaceSampler* surfSampler = static_cast <CmSurfaceSampler *> (surface);
            SAMPLER_SURFACE_TYPE type;
            surfSampler->GetSurfaceType(type);
            if (type == SAMPLER_SURFACE_TYPE_2D)
            {
                return ARG_KIND_SURFACE_SAMPLER;
            }
            else if (type == SAMPLER_SURFACE_TYPE_2DUP)
            {
                return ARG_KIND_SURFACE2DUP_SAMPLER;
            }
            else
            {
                return ARG_KIND_SURFACE_3D;
            }
        }
        case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER8X8:
        {
            CmSurfaceSampler8x8* surfSampler8x8 = static_cast <CmSurfaceSampler8x8 *> (surface);
            if (surfSampler8x8->GetSampler8x8SurfaceType() == CM_VA_SURFACE)
            {
                return ARG_KIND_SURFACE_SAMPLER8X8_VA;
            }
            else
            {
                return ARG_KIND_SURFACE_SAMPLER8X8_AVS;
            }
        }
        case CM_ENUM_CLASS_TYPE_CMSURFACEVME:
            return ARG_KIND_SURFACE_VME;
        case CM_ENUM_CLASS_TYPE_CM_STATE_BUFFER:
            return ARG_KIND_STATE_BUFFER;
        default:
            return ARG_KIND_GENERAL;
    }
}

CmSurfaceState* CmKernelEx::GetSurfaceState(CmSurface *surface, uint32_t index)
{
    CM_HAL_STATE *cmHalState = ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState;
    uint32_t surfaceArraySize = 0;
    m_surfaceMgr->GetSurfaceArraySize(surfaceArraySize);
    CM_CHK_COND_RETURN((surfaceArraySize == 0), nullptr, "Surface Array is empty.");
    uint32_t aliasIndex = index/surfaceArraySize;

    switch (surface->Type())
    {
        case CM_ENUM_CLASS_TYPE_CMSURFACE2D:
        {
            CmSurface2DRT* surf2D = static_cast<CmSurface2DRT*>(surface);
            uint32_t halIndex = 0;
            surf2D->GetIndexFor2D(halIndex);
            PCM_HAL_SURFACE2D_SURFACE_STATE_PARAM surfStateParam = nullptr;
            if (aliasIndex > 0 || cmHalState->umdSurf2DTable[halIndex].surfStateSet)
            {
                surfStateParam = &(cmHalState->umdSurf2DTable[halIndex].surfaceStateParam[aliasIndex]);
            }
            return cmHalState->umdSurf2DTable[halIndex].surfStateMgr->GetSurfaceState(0, 0, surfStateParam);
        }
        case CM_ENUM_CLASS_TYPE_CMSURFACE2DUP:
        {
            CmSurface2DUPRT* surf2DUP = static_cast<CmSurface2DUPRT*>(surface);
            uint32_t halIndex = 0;
            surf2DUP->GetHandle(halIndex);
            return cmHalState->surf2DUPTable[halIndex].surfStateMgr->GetSurfaceState();
        }
        case CM_ENUM_CLASS_TYPE_CMBUFFER_RT:
        {
            CmBuffer_RT* surf1D = static_cast<CmBuffer_RT*>(surface);
            uint32_t halIndex = 0;
            surf1D->GetHandle(halIndex);
            CM_HAL_BUFFER_SURFACE_STATE_ENTRY *surfStateParam = nullptr;
            if (aliasIndex > 0 || cmHalState->bufferTable[halIndex].surfStateSet)
            {
                surfStateParam = &(cmHalState->bufferTable[halIndex].surfaceStateEntry[aliasIndex]);
            }
            return cmHalState->bufferTable[halIndex].surfStateMgr->GetSurfaceState(surfStateParam);
        }
        case CM_ENUM_CLASS_TYPE_CMSURFACE3D:
        {
            CmSurface3DRT *surf3D = static_cast<CmSurface3DRT *>(surface);
            uint32_t halIndex = 0;
            surf3D->GetHandle(halIndex);
            return cmHalState->surf3DTable[halIndex].surfStateMgr->GetSurfaceState(0, 1);
        }
        case CM_ENUM_CLASS_TYPE_CMSURFACEVME:
        {
            CmSurfaceVme *surfVme = static_cast<CmSurfaceVme*>(surface);
            CmSurfaceStateVME *surfState = surfVme->GetSurfaceState();
            if (surfState == nullptr)
            {
                int argSize = surfVme->GetVmeCmArgSize();
                int surfCount = surfVme->GetTotalSurfacesCount();

                uint8_t *vmeValue = MOS_NewArray(uint8_t, argSize);
                if (vmeValue == nullptr)
                {
                    return nullptr;
                }
                uint16_t surfIndexes[17];
                SetArgsSingleVme(surfVme, vmeValue, surfIndexes);
                surfState = MOS_New(CmSurfaceStateVME, cmHalState);
                if (surfState == nullptr)
                {
                    return nullptr;
                }
                surfState->Initialize((CM_HAL_VME_ARG_VALUE *)vmeValue);

                surfVme->SetSurfState(cmHalState->advExecutor, vmeValue, surfState); // set for destroy later
            }
            return surfState;
        }
        case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER:
        {
            uint32_t halIndex = 0;
            uint16_t cmIndex = 0;
            CmSurfaceSampler* surfSampler = static_cast <CmSurfaceSampler *> (surface);
            surfSampler->GetHandle(halIndex);
            surfSampler->GetCmIndexCurrent(cmIndex);
            SAMPLER_SURFACE_TYPE type;
            surfSampler->GetSurfaceType(type);
            switch (type)
            {
                case SAMPLER_SURFACE_TYPE_2D:
                {
                    // re-calculate the aliasIndex
                    aliasIndex = cmIndex/surfaceArraySize;

                    PCM_HAL_SURFACE2D_SURFACE_STATE_PARAM surfStateParam = nullptr;
                    if (aliasIndex > 0 || cmHalState->umdSurf2DTable[halIndex].surfStateSet)
                    {
                        surfStateParam = &(cmHalState->umdSurf2DTable[halIndex].surfaceStateParam[aliasIndex]);
                    }
                    return cmHalState->umdSurf2DTable[halIndex].surfStateMgr->GetSurfaceState(0, 1, surfStateParam);
                }
                case SAMPLER_SURFACE_TYPE_2DUP:
                {
                    return cmHalState->surf2DUPTable[halIndex].surfStateMgr->GetSurfaceState(0, 1);
                }
                case SAMPLER_SURFACE_TYPE_3D:
                {
                    return cmHalState->surf3DTable[halIndex].surfStateMgr->GetSurfaceState(0, 1);
                }
                default:
                {
                }
            }
        }
        case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER8X8:
        {
            CmSurfaceSampler8x8* surfSampler8x8 = static_cast <CmSurfaceSampler8x8 *> (surface);
            uint32_t halIndex = 0;
            uint16_t cmIndex = 0;

            surfSampler8x8->GetIndexCurrent(halIndex);
            surfSampler8x8->GetCmIndex(cmIndex);
            // re-calculate the aliasIndex
            aliasIndex = cmIndex/surfaceArraySize;

            PCM_HAL_SURFACE2D_SURFACE_STATE_PARAM surfStateParam = nullptr;
            if (aliasIndex > 0 || cmHalState->umdSurf2DTable[halIndex].surfStateSet)
            {
                surfStateParam = &(cmHalState->umdSurf2DTable[halIndex].surfaceStateParam[aliasIndex]);
            }
            return cmHalState->umdSurf2DTable[halIndex].surfStateMgr->GetSurfaceState(1, 1, surfStateParam);
        }
        default: //not implemented yet
            return nullptr;

    }
    return nullptr;
}

uint32_t CmKernelEx::GetMaxBteNum()
{
    uint32_t bteCount = 0;
    for (uint32_t i = 0; i < m_flatArgCount; i++)
    {
        if (IsSurface(m_flatArgs[i].kind))
        {
            CmSurfaceState *surfState = *(CmSurfaceState **)(m_data + m_flatArgs[i].offset);
            if (surfState == nullptr) //CM_NULL_SURFACE
            {
                continue;
            }
            bteCount += surfState->GetNumBte();
        }
    }
    return bteCount;
}

MOS_STATUS CmKernelEx::UpdateCurbe(CmSSH *ssh, CmMediaState *mediaState, uint32_t kernelIdx)
{
    for (uint32_t i = 0; i < m_flatArgCount; i++)
    {
        if (IsSurface(m_flatArgs[i].kind))
        {
            CmSurface *surface = *(CmSurface **)(m_surfaceInArg + m_flatArgs[i].offset);
            if (surface != nullptr && m_propertyIndexes[i] != surface->GetPropertyIndex())
            {
                // need to update the surface state
                CmSurfaceState *temp = GetSurfaceState(surface, m_cmSurfIndexes[i]);
                m_propertyIndexes[i] = surface->GetPropertyIndex();
                *(CmSurfaceState **)(m_data + m_flatArgs[i].offset) = temp;
            }
            CmSurfaceState *surfState = *(CmSurfaceState **)(m_data + m_flatArgs[i].offset);
            if (surfState == nullptr)
            {
                continue;
            }
            uint32_t bteIdx = ssh->AddSurfaceState(surfState);
            *(uint32_t *)(m_curbe + m_flatArgs[i].payloadOffset) = bteIdx;
        }
        else if (m_flatArgs[i].kind == ARG_KIND_SAMPLER)
        {
            MHW_SAMPLER_STATE_PARAM *param = *(MHW_SAMPLER_STATE_PARAM **)(m_data + m_flatArgs[i].offset);
            uint32_t bteIdx = mediaState->AddSampler(param, kernelIdx);
            *(uint32_t *)(m_curbe + m_flatArgs[i].payloadOffset) = bteIdx;
        }
        else if (m_flatArgs[i].kind != ARG_KIND_IMPLICT_LOCALSIZE
                 && m_flatArgs[i].kind != ARG_KIND_IMPLICT_GROUPSIZE
                 && m_flatArgs[i].kind != ARG_KIND_IMPLICIT_LOCALID)
        {
            MOS_SecureMemcpy(m_curbe + m_flatArgs[i].payloadOffset, m_flatArgs[i].sizeInCurbe,
                m_data + m_flatArgs[i].offset, m_flatArgs[i].unitSize);
        }
    }

    // dump
    /*
    for (int i = 0; i < m_curbeSize/4; i++)
    {
        printf("0x%x, ", *((uint32_t *)m_curbe + i));
    }
    printf("\n");
    */
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmKernelEx::UpdateFastTracker(uint32_t trackerIndex, uint32_t tracker)
{
    for (uint32_t i = 0; i < m_flatArgCount; i++)
    {
        if (IsSurface(m_flatArgs[i].kind))
        {
            CmSurface *surface = *(CmSurface **)(m_surfaceInArg + m_flatArgs[i].offset);
            if (surface == nullptr)
            {
                continue;
            }
            surface->SetFastTracker(trackerIndex, tracker);
        }
    }
    return MOS_STATUS_SUCCESS;
}


MOS_STATUS CmKernelEx::UpdateSWSBArgs(CmThreadSpaceRT *threadSpace)
{
    CmThreadSpaceRT *ts = (threadSpace == nullptr)?m_threadSpace:threadSpace;
    if (ts == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }
    int ret = ts->SetDependencyArgToKernel(this);
    return (ret == 0)? MOS_STATUS_SUCCESS : MOS_STATUS_UNKNOWN;
}

int32_t CmKernelEx::SetStaticBuffer(uint32_t index, const void *value)
{
    CM_CHK_CMSTATUS_RETURN(CmKernelRT::SetStaticBuffer(index, value));

    if(index >= CM_GLOBAL_SURFACE_NUMBER)
    {
        CM_ASSERTMESSAGE("Error: Surface Index exceeds max global surface number.");
        return CM_INVALID_GLOBAL_BUFFER_INDEX;
    }

    if(!value)
    {
        CM_ASSERTMESSAGE("Error: Invalid StaticBuffer arg value.");
        return CM_INVALID_BUFFER_HANDLER;
    }

    SurfaceIndex* surfIndex = (SurfaceIndex* )value;
    uint32_t indexData = surfIndex->get_data();

    CmSurface* surface = nullptr;
    m_surfaceMgr->GetSurface(indexData, surface);
    if (surface != nullptr)
    {
        // for gen9+ platforms, index + 1 is the BTI
        m_reservedSurfaceBteIndexes[index + CM_GLOBAL_SURFACE_INDEX_START_GEN9_PLUS]
                                                = GetSurfaceState(surface, indexData);
    }
    return CM_SUCCESS;
}

int32_t CmKernelEx::SetSurfaceBTI(SurfaceIndex *surfIndex, uint32_t bti)
{
    CM_CHK_CMSTATUS_RETURN(CmKernelRT::SetSurfaceBTI(surfIndex, bti));

    CM_CHK_NULL_RETURN_CMERROR(surfIndex);
    uint32_t index = surfIndex->get_data();

    CmSurface* surface = nullptr;
    m_surfaceMgr->GetSurface(index, surface);
    if (surface != nullptr)
    {
        m_reservedSurfaceBteIndexes[bti] = GetSurfaceState(surface, index);
    }
    return CM_SUCCESS;
}

int32_t CmKernelEx::SetSamplerBTI(SamplerIndex* sampler, uint32_t nIndex)
{
    CM_CHK_CMSTATUS_RETURN(CmKernelRT::SetSamplerBTI(sampler, nIndex));

    uint32_t index = sampler->get_data();
    m_reservedSamplerBteIndexes[nIndex] = (MHW_SAMPLER_STATE_PARAM *)GetSamplerParam(index);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmKernelEx::LoadReservedSurfaces(CmSSH *ssh)
{
    for (auto it = m_reservedSurfaceBteIndexes.begin(); it != m_reservedSurfaceBteIndexes.end(); ++ it)
    {
        ssh->AddSurfaceState(it->second, it->first);
    }

    // reset the table in legacy kernel for bti reuse
    if (m_usKernelPayloadSurfaceCount)
    {
        CmSafeMemSet(m_IndirectSurfaceInfoArray, 0, m_usKernelPayloadSurfaceCount * sizeof(CM_INDIRECT_SURFACE_INFO));
        m_usKernelPayloadSurfaceCount = 0;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmKernelEx::LoadReservedSamplers(CmMediaState *mediaState, uint32_t kernelIdx)
{
    for (auto it = m_reservedSamplerBteIndexes.begin(); it != m_reservedSamplerBteIndexes.end(); ++ it)
    {
        mediaState->AddSampler((MHW_SAMPLER_STATE_PARAM *)it->second, kernelIdx, it->first);
    }
    return MOS_STATUS_SUCCESS;
}

void* CmKernelEx::GetSamplerParam(uint32_t index)
{
    CM_HAL_STATE *cmHalState = ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState;
    return (void *)&cmHalState->samplerTable[index];
}

MOS_STATUS CmKernelEx::GetSamplerCount(uint32_t *count3D, uint32_t *countAVS)
{
    *count3D = 0;
    *countAVS = 0;
    for (uint32_t i = 0; i < m_flatArgCount; i++)
    {
        if (m_flatArgs[i].kind == ARG_KIND_SAMPLER)
        {
            MHW_SAMPLER_STATE_PARAM *temp = *(MHW_SAMPLER_STATE_PARAM **)(m_data + m_flatArgs[i].offset);
            if (temp->SamplerType == MHW_SAMPLER_TYPE_3D)
            {
                ++ (*count3D);
            }
            else if (temp->SamplerType == MHW_SAMPLER_TYPE_AVS)
            {
                ++ (*countAVS);
            }
            else
            {
                // only support 3D and AVS samplers by now in fast path
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

CmThreadSpaceRT* CmKernelEx::GetThreadSpaceEx()
{
    if (m_threadSpace)
    {
        return m_threadSpace;
    }
    if (m_dummyThreadSpace)
    {
        m_device->DestroyThreadSpace(m_dummyThreadSpace);
    }
    if (m_threadCount)
    {
        m_device->CreateThreadSpace(m_threadCount, 1, m_dummyThreadSpace);
    }
    return static_cast<CmThreadSpaceRT *>(m_dummyThreadSpace);
}

CmThreadGroupSpace* CmKernelEx::GetThreadGroupSpaceEx()
{
    if (m_threadGroupSpace)
    {
        return m_threadGroupSpace;
    }
    if (m_dummyThreadGroupSpace)
    {
        m_device->DestroyThreadGroupSpace(m_dummyThreadGroupSpace);
    }

    if (m_threadCount)
    {
        m_device->CreateThreadGroupSpace(1, 1, m_threadCount, 1, m_dummyThreadGroupSpace);
    }
    return m_dummyThreadGroupSpace;
}

void CmKernelEx::SurfaceDumpEx(uint32_t kernelNumber, int32_t taskId)
{
    for(uint32_t argIdx = 0; argIdx < m_argCount; argIdx++)
    {
        uint32_t start = m_indexMap[argIdx];
        uint32_t len = m_indexMap[argIdx + 1] - start;

        for (uint32_t v = 0; v < len; v ++)
        {
            uint32_t i = start + v;
            if (IsSurface(m_flatArgs[i].kind))
            {
                CmSurface *surface = *(CmSurface **)(m_surfaceInArg + m_flatArgs[i].offset);
                if (surface == nullptr)
                {
                    continue;
                }
                surface->DumpContent(kernelNumber, m_kernelInfo->kernelName, taskId, argIdx, v);
            }
        }
    }
}

bool CmKernelEx::IsFastPathSupported()
{
    // current fast path doesn't support media object
    bool specialDependency = false;
    if (m_threadSpace)
    {
        CM_DEPENDENCY_PATTERN dependencyPatternType = CM_NONE_DEPENDENCY;
        m_threadSpace->GetDependencyPatternType(dependencyPatternType);
        specialDependency = (dependencyPatternType == CM_WAVEFRONT26Z || dependencyPatternType == CM_WAVEFRONT26ZI);
    }

    return !(m_perThreadArgExists || specialDependency);
}

