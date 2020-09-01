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
//! \file     media_scalability_factory.cpp
//! \brief    Defines the functions for media scalability factory
//!

#include "media_scalability_factory.h"
#include "media_scalability_singlepipe.h"
#include "media_scalability_mdf.h"
#include "vp_scalability_singlepipe.h"
#include "decode_scalability_singlepipe.h"

template<typename T>
MediaScalability *MediaScalabilityFactory<T>::CreateScalability(uint8_t componentType, T params, void *hwInterface, MediaContext *mediaContext, MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
{
    if (params == nullptr)
    {
        return nullptr;
    }

    if (std::is_same<decltype(params), ScalabilityPars*>::value)
    {
        auto scalabPars = reinterpret_cast<ScalabilityPars *>(params);
        if (scalabPars->enableMdf)
        {
            return CreateScalabilityMdf(params);
        }
    }

    //Create SinglePipe/MultiPipe scalability.
    return CreateScalabilityCmdBuf(componentType, params, hwInterface, mediaContext, gpuCtxCreateOption);
}

template<typename T>
MediaScalability *MediaScalabilityFactory<T>::CreateScalabilityMdf(T params)
{
    MediaScalability *scalabilityHandle = nullptr;
    scalabilityHandle = MOS_New(MediaScalabilityMdf);

    return scalabilityHandle;
}

template<typename T>
MediaScalability *MediaScalabilityFactory<T>::CreateScalabilityCmdBuf(uint8_t componentType, T params, void *hwInterface, MediaContext *mediaContext, MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
{
    switch (componentType)
    {
    case scalabilityEncoder:
        return CreateEncodeScalability(params, hwInterface, mediaContext, gpuCtxCreateOption);
    case scalabilityDecoder:
        return CreateDecodeScalability(params, hwInterface, mediaContext, gpuCtxCreateOption);
    case scalabilityVp:
        return CreateVpScalability(params, hwInterface, mediaContext, gpuCtxCreateOption);
    default:
        return nullptr;
    }
}

template<typename T>
MediaScalability *MediaScalabilityFactory<T>::CreateEncodeScalability(T params, void *hwInterface, MediaContext *mediaContext, MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
{
    return nullptr;
}

template<typename T>
MediaScalability *MediaScalabilityFactory<T>::CreateDecodeScalability(T params, void *hwInterface, MediaContext *mediaContext, MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
{
    if (params == nullptr || hwInterface == nullptr)
    {
        return nullptr;
    }

    decode::DecodeScalabilityOption *option = nullptr;
    if (std::is_same<decltype(params), ScalabilityPars*>::value)
    {
        option = MOS_New(decode::DecodeScalabilityOption);
        if (option != nullptr)
        {
            auto scalabPars = reinterpret_cast<ScalabilityPars *>(params);
            option->SetScalabilityOption(scalabPars);
        }
    }
    else
    {
        auto scalabOption = reinterpret_cast<MediaScalabilityOption *>(params);
        option = dynamic_cast<decode::DecodeScalabilityOption *>(scalabOption);
    }

    if (option == nullptr)
    {
        return nullptr;
    }

    //Create scalability handle refer to scalability option.
    MediaScalability *scalabilityHandle = nullptr;
    if (option->GetNumPipe() == 1)
    {
        scalabilityHandle = MOS_New(decode::DecodeScalabilitySinglePipe, hwInterface, mediaContext, scalabilityDecoder);
    }
    else
    {
        SCALABILITY_ASSERTMESSAGE("Scalability Initialize Failed!");
    }

    if (scalabilityHandle == nullptr)
    {
        return nullptr;
    }

    if (MOS_STATUS_SUCCESS != scalabilityHandle->Initialize(*option))
    {
        SCALABILITY_ASSERTMESSAGE("Scalability Initialize failed!");
        MOS_Delete(scalabilityHandle);
        return nullptr;
    }
    if (gpuCtxCreateOption)
    {
        scalabilityHandle->GetGpuCtxCreationOption(gpuCtxCreateOption);
    }

    if (std::is_same<decltype(params), ScalabilityPars*>::value)
    {
        MOS_Delete(option);
    }

    return scalabilityHandle;
}

template<typename T>
MediaScalability *MediaScalabilityFactory<T>::CreateVpScalability(T params, void *hwInterface, MediaContext *mediaContext, MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
{
    if (params == nullptr || hwInterface == nullptr)
    {
        return nullptr;
    }

    vp::VpScalabilityOption *option = nullptr;
    if (std::is_same<decltype(params), ScalabilityPars*>::value)
    {
        option = MOS_New(vp::VpScalabilityOption);
        if (option != nullptr)
        {
            auto scalabPars = reinterpret_cast<ScalabilityPars *>(params);
            option->SetScalabilityOption(scalabPars);
        }
    }
    else
    {
        auto scalabOption = reinterpret_cast<MediaScalabilityOption *>(params);
        option = dynamic_cast<vp::VpScalabilityOption *>(scalabOption);
    }

    if (option == nullptr)
    {
        return nullptr;
    }

    // will add scalability multi-pipe when 2 or more Vebox/SFC are supported
    MediaScalability *scalabilityHandle = nullptr;
    if (option->GetNumPipe() == 1)
    {
        scalabilityHandle = MOS_New(vp::VpScalabilitySinglePipe, hwInterface, mediaContext, scalabilityVp);
    }
    else
    {
        SCALABILITY_ASSERTMESSAGE("Scalability Initialize failed! Current platform only support SinglePipe Scalability");
    }

    if (scalabilityHandle == nullptr)
    {
        return nullptr;
    }

    if (MOS_STATUS_SUCCESS != scalabilityHandle->Initialize(*option))
    {
        SCALABILITY_ASSERTMESSAGE("Scalability Initialize failed!");
        MOS_Delete(scalabilityHandle);
        return nullptr;
    }

    if (gpuCtxCreateOption)
    {
        scalabilityHandle->GetGpuCtxCreationOption(gpuCtxCreateOption);
    }

    if (std::is_same<decltype(params), ScalabilityPars*>::value)
    {
        MOS_Delete(option);
    }

    return scalabilityHandle;
}

template class MediaScalabilityFactory<ScalabilityPars*>;

