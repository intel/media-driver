/*
* Copyright (c) 2018-2021, Intel Corporation
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
#if !EMUL
#include "encode_scalability_singlepipe.h"
#include "encode_scalability_multipipe.h"
#include "decode_scalability_option.h"
#endif
#include "vp_scalability_multipipe_next.h"
#include "vp_scalability_singlepipe_next.h"

template<typename T>
MediaScalability *MediaScalabilityFactory<T>::CreateScalability(uint8_t componentType, T params, void *hwInterface, MediaContext *mediaContext, MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
{
    if (params == nullptr)
    {
        return nullptr;
    }

    //Create SinglePipe/MultiPipe scalability.
    return CreateScalabilityCmdBuf(componentType, params, hwInterface, mediaContext, gpuCtxCreateOption);
}

template<typename T>
MediaScalability *MediaScalabilityFactory<T>::CreateScalabilityCmdBuf(uint8_t componentType, T params, void *hwInterface, MediaContext *mediaContext, MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
{
    switch (componentType)
    {
#if !EMUL
    case scalabilityEncoder:
        return CreateEncodeScalability(params, hwInterface, mediaContext, gpuCtxCreateOption);
    case scalabilityDecoder:
        return CreateDecodeScalability(params, hwInterface, mediaContext, gpuCtxCreateOption);
#endif
    case scalabilityVp:
        return CreateVpScalability(params, hwInterface, mediaContext, gpuCtxCreateOption);
    default:
        return nullptr;
    }
}
#if !EMUL
template<typename T>
MediaScalability *MediaScalabilityFactory<T>::CreateEncodeScalability(T params, void *hwInterface, MediaContext *mediaContext, MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
{
    MediaScalability *scalabilityHandle = nullptr;
    if (params == nullptr || hwInterface == nullptr)
    {
        return nullptr;
    }

    encode::EncodeScalabilityOption *option = nullptr;
    if (std::is_same<decltype(params), ScalabilityPars*>::value)
    {
        option = MOS_New(encode::EncodeScalabilityOption);
        if (option != nullptr)
        {
            auto scalabPars = reinterpret_cast<ScalabilityPars *>(params);
            option->SetScalabilityOption(scalabPars);
        }
    }
    else
    {
        auto scalabOption = reinterpret_cast<MediaScalabilityOption *>(params);
        option = dynamic_cast<encode::EncodeScalabilityOption *>(scalabOption);
    }

    if (option == nullptr)
    {
        return nullptr;
    }

    //Create scalability handle refer to scalability option.

    if (option->GetNumPipe() == 1)
    {
        scalabilityHandle = MOS_New(encode::EncodeScalabilitySinglePipe, hwInterface, mediaContext, scalabilityEncoder);
    }
    else
    {
        scalabilityHandle = MOS_New(encode::EncodeScalabilityMultiPipe, hwInterface, mediaContext, scalabilityEncoder);
    }

    if (scalabilityHandle == nullptr)
    {
        if (std::is_same<decltype(params), ScalabilityPars *>::value)
        {
            MOS_Delete(option);
        }
        return nullptr;
    }

    if (MOS_STATUS_SUCCESS != scalabilityHandle->Initialize(*option))
    {
        SCALABILITY_ASSERTMESSAGE("Scalability Initialize failed!");
        MOS_Delete(scalabilityHandle);
        if (std::is_same<decltype(params), ScalabilityPars *>::value)
        {
            MOS_Delete(option);
        }
        return nullptr;
    }
    if (gpuCtxCreateOption)
    {
        scalabilityHandle->GetGpuCtxCreationOption(gpuCtxCreateOption);
    }

    if (std::is_same<decltype(params), ScalabilityPars *>::value)
    {
        MOS_Delete(option);
    }

    return scalabilityHandle;
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
            MOS_STATUS status = option->SetScalabilityOption(scalabPars);
            if (status != MOS_STATUS_SUCCESS)
            {
                SCALABILITY_ASSERTMESSAGE("option->SetScalabilityOption failed w/ status == %d!", status);
                MOS_Delete(option);
                return nullptr;
            }
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
    CodechalHwInterfaceNext *codecHwInterface  = ((CodechalHwInterfaceNext *)hwInterface);
    if (codecHwInterface->pfnCreateDecodeSinglePipe == nullptr || codecHwInterface->pfnCreateDecodeMultiPipe == nullptr)
    {
        SCALABILITY_ASSERTMESSAGE("Scalability pointer is null!");
        if (std::is_same<decltype(params), ScalabilityPars *>::value)
        {
            MOS_Delete(option);
        }
        return nullptr;
    }
    if (option->GetNumPipe() == 1)
    {
        if ((codecHwInterface->pfnCreateDecodeSinglePipe(hwInterface, mediaContext, scalabilityDecoder)) != MOS_STATUS_SUCCESS)
        {
            SCALABILITY_ASSERTMESSAGE("Scalability Creation failed!");
            if (std::is_same<decltype(params), ScalabilityPars *>::value)
            {
                MOS_Delete(option);
            }
            return nullptr;
        }
        scalabilityHandle = codecHwInterface->m_singlePipeScalability;
    }
    else
    {
        if ((codecHwInterface->pfnCreateDecodeMultiPipe(hwInterface, mediaContext, scalabilityDecoder)) != MOS_STATUS_SUCCESS)
        {
            SCALABILITY_ASSERTMESSAGE("Scalability Creation failed!");
            if (std::is_same<decltype(params), ScalabilityPars *>::value)
            {
                MOS_Delete(option);
            }
            return nullptr;
        }
        scalabilityHandle = codecHwInterface->m_multiPipeScalability;
    }

    if (scalabilityHandle == nullptr)
    {
        if (std::is_same<decltype(params), ScalabilityPars *>::value)
        {
            MOS_Delete(option);
        }
        return nullptr;
    }

    if (MOS_STATUS_SUCCESS != scalabilityHandle->Initialize(*option))
    {
        SCALABILITY_ASSERTMESSAGE("Scalability Initialize failed!");
        MOS_Delete(scalabilityHandle);
        if (std::is_same<decltype(params), ScalabilityPars *>::value)
        {
            MOS_Delete(option);
        }
        return nullptr;
    }
    if (gpuCtxCreateOption)
    {
        scalabilityHandle->GetGpuCtxCreationOption(gpuCtxCreateOption);
    }

    if (std::is_same<decltype(params), ScalabilityPars *>::value)
    {
        MOS_Delete(option);
    }

    return scalabilityHandle;
}
#endif
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
    PVP_MHWINTERFACE  vphwInterface     = (PVP_MHWINTERFACE)hwInterface;

    // Check CreateMultiPipe/CreateSinglePipe pointer
    if (vphwInterface->pfnCreateSinglePipe == nullptr || vphwInterface->pfnCreateMultiPipe == nullptr)
    {
        SCALABILITY_ASSERTMESSAGE("Scalability pointer is null!");
        if (std::is_same<decltype(params), ScalabilityPars *>::value)
        {
            MOS_Delete(option);
        }
        return nullptr;
    }

    if (option->GetNumPipe() == 1)
    {
        if ((vphwInterface->pfnCreateSinglePipe(hwInterface, mediaContext, scalabilityVp)) != MOS_STATUS_SUCCESS)
        {
            SCALABILITY_ASSERTMESSAGE("Scalability Creation failed!");
            if (std::is_same<decltype(params), ScalabilityPars *>::value)
            {
                MOS_Delete(option);
            }
            return nullptr;
        }
        scalabilityHandle = vphwInterface->m_singlePipeScalability;
    }
    else
    {
        if ((vphwInterface->pfnCreateMultiPipe(hwInterface, mediaContext, scalabilityVp)) != MOS_STATUS_SUCCESS)
        {
            SCALABILITY_ASSERTMESSAGE("Scalability Creation failed!");
            if (std::is_same<decltype(params), ScalabilityPars *>::value)
            {
                MOS_Delete(option);
            }
            return nullptr;
        }
        scalabilityHandle = vphwInterface->m_multiPipeScalability;
    }

    if (scalabilityHandle == nullptr)
    {
        if (std::is_same<decltype(params), ScalabilityPars *>::value)
        {
            MOS_Delete(option);
        }
        return nullptr;
    }

    if (MOS_STATUS_SUCCESS != scalabilityHandle->Initialize(*option))
    {
        SCALABILITY_ASSERTMESSAGE("Scalability Initialize failed!");
        MOS_Delete(scalabilityHandle);
        if (std::is_same<decltype(params), ScalabilityPars *>::value)
        {
            MOS_Delete(option);
        }
        return nullptr;
    }

    if (gpuCtxCreateOption)
    {
        scalabilityHandle->GetGpuCtxCreationOption(gpuCtxCreateOption);
    }

    if (std::is_same<decltype(params), ScalabilityPars *>::value)
    {
        MOS_Delete(option);
    }

    return scalabilityHandle;
}

template class MediaScalabilityFactory<ScalabilityPars*>;
template class MediaScalabilityFactory<MediaScalabilityOption*>;

