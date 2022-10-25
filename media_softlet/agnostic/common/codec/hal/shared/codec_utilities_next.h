/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     codec_utilities_next.h
//! \brief    Defines the common functions for codec.
//! \details  This modules implements utilities which are shared by encoder and decoder
//!

#ifndef __CODEC_UTILITIES_NEXT_H__
#define __CODEC_UTILITIES_NEXT_H__
#include "mos_defs.h"
#include "mos_os_specific.h"
#include "codec_hw_next.h"

#define CODECHAL_SURFACE_PITCH_ALIGNMENT        128
class CodecUtilities
{
public:
    //!
    //! \brief    Allocate data list with specific type and length
    //!
    //! \param    [in,out] dataList
    //!           Pointer to a type * pointer. Specify the address of the memory handles
    //! \param    [in] length
    //!           Specify the number of data members
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    template <class type>
    static MOS_STATUS CodecHalAllocateDataList(type **dataList, uint32_t length)
    {
        type *ptr;
        ptr = (type *)MOS_AllocAndZeroMemory(sizeof(type) * length);
        if (ptr == nullptr)
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("No memory to allocate CodecHal data list.");
            return MOS_STATUS_NO_SPACE;
        }
        for (uint32_t i = 0; i < length; i++)
        {
            dataList[i] = &(ptr[i]);
        }
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Free data list
    //!
    //! \param    [in,out] dataList
    //!           Pointer to a type * pointer. Specify the address of the memory handles
    //! \param    [in] length
    //!           Specify the number of data members
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    template <class type>
    static MOS_STATUS CodecHalFreeDataList(type **dataList, uint32_t length)
    {
        type *ptr;
        ptr = dataList[0];
        if (ptr)
        {
            MOS_FreeMemory(ptr);
        }
        for (uint32_t i = 0; i < length; i++)
        {
            dataList[i] = nullptr;
        }

        return MOS_STATUS_SUCCESS;
    }

    static MOS_STATUS CodecHalGetResourceInfo(
    PMOS_INTERFACE osInterface,
    PMOS_SURFACE surface)
    {
        CODECHAL_PUBLIC_CHK_NULL_RETURN(surface);

        MOS_SURFACE details;
        MOS_ZeroMemory(&details, sizeof(details));
        details.Format = Format_Invalid;
        
        CODECHAL_PUBLIC_CHK_STATUS_RETURN(osInterface->pfnGetResourceInfo(osInterface, &surface->OsResource, &details));
        
        surface->Format        = details.Format;
        surface->dwWidth       = details.dwWidth;
        surface->dwHeight      = details.dwHeight;
        surface->dwPitch       = details.dwPitch;
        surface->dwDepth       = details.dwDepth;
        surface->dwQPitch      = details.dwQPitch;
        surface->bArraySpacing = details.bArraySpacing;
        surface->TileType      = details.TileType;
        surface->TileModeGMM   = details.TileModeGMM;
        surface->bGMMTileEnabled = details.bGMMTileEnabled;
        surface->dwOffset      = details.RenderOffset.YUV.Y.BaseOffset;
        surface->YPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.Y.BaseOffset;
        surface->YPlaneOffset.iXOffset = details.RenderOffset.YUV.Y.XOffset;
        surface->YPlaneOffset.iYOffset =
            (surface->YPlaneOffset.iSurfaceOffset - surface->dwOffset) / surface->dwPitch +
            details.RenderOffset.YUV.Y.YOffset;
        surface->UPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.U.BaseOffset;
        surface->UPlaneOffset.iXOffset       = details.RenderOffset.YUV.U.XOffset;
        surface->UPlaneOffset.iYOffset       =
            (surface->UPlaneOffset.iSurfaceOffset - surface->dwOffset) / surface->dwPitch +
            details.RenderOffset.YUV.U.YOffset;
        surface->UPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.U;
        surface->VPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.V.BaseOffset;
        surface->VPlaneOffset.iXOffset       = details.RenderOffset.YUV.V.XOffset;
        surface->VPlaneOffset.iYOffset       =
            (surface->VPlaneOffset.iSurfaceOffset - surface->dwOffset) / surface->dwPitch +
            details.RenderOffset.YUV.V.YOffset;
        surface->VPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.V;
        surface->bCompressible     = details.bCompressible;
        surface->CompressionMode   = details.CompressionMode;
        surface->bIsCompressed     = details.bIsCompressed;
        
        return MOS_STATUS_SUCCESS;
    }
    MEDIA_CLASS_DEFINE_END(CodecUtilities)
};

#endif