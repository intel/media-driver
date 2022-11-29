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
//! \file     ddi_coedc_base_specific.cpp
//! \brief    Defines base class for softlet DDI codec encode/decoder.
//!

#include "ddi_codec_base_specific.h"
#include "media_libva_util_next.h"

namespace codec
{
int32_t DdiCodecBase::GetRenderTargetID(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, DDI_MEDIA_SURFACE *surface)
{
    if((nullptr == surface) || (nullptr == rtTbl))
    {
        return DDI_CODEC_INVALID_FRAME_INDEX;
    }

    if (0 == rtTbl->iNumRenderTargets)
    {
        return DDI_CODEC_INVALID_FRAME_INDEX;
    }

    for(int32_t i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i++)
    {
        if(rtTbl->pRT[i] == surface)
        {
            return i;
        }
    }
    return DDI_CODEC_INVALID_FRAME_INDEX;
}

VAStatus DdiCodecBase::RegisterRTSurfaces(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, DDI_MEDIA_SURFACE *surface)
{
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(rtTbl, "nullptr rtTbl", VA_STATUS_ERROR_INVALID_PARAMETER);

    int32_t i = 0;
    uint32_t emptyEntry = DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT + 1;
    for (i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i++)
    {
        if (rtTbl->pRT[i] == surface)
        {
            //pCurrRT has already been registered
            emptyEntry = DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT + 1;
            break;
        }
        else if ((rtTbl->pRT[i] == nullptr) && (emptyEntry == (DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT + 1)))
        {
            //find the first empty entry
            emptyEntry = i;
        }
    }

    if (emptyEntry < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
    {
        i = emptyEntry;
    }
    //if pCurrRT has not registered in pRT, add it into the array
    if (i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
    {
        if (emptyEntry < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
        {
            rtTbl->pRT[emptyEntry] = surface;
            rtTbl->ucRTFlag[emptyEntry] = SURFACE_STATE_ACTIVE_IN_CURFRAME;
            rtTbl->iNumRenderTargets++;
        }
        else
        {
            rtTbl->ucRTFlag[i] = SURFACE_STATE_ACTIVE_IN_CURFRAME;
        }
    }
    else
    {
        uint32_t j = 0;
        for(j = 0; j < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; j ++)
        {
            if(rtTbl->ucRTFlag[j] == SURFACE_STATE_INACTIVE)
            {
                rtTbl->pRT[j] = surface;
                rtTbl->ucRTFlag[j] = SURFACE_STATE_ACTIVE_IN_CURFRAME;
                break;
            }
        }
        if(j == DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
        {
            DDI_VERBOSEMESSAGE("RT table is full, and have no one can be resued");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiCodecBase::ClearRefList(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, bool withDpb)
{
    DDI_CHK_NULL(rtTbl, "nullptr rtTbl", VA_STATUS_ERROR_INVALID_PARAMETER);

    if(withDpb)
    {
        for(int32_t i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i ++)
        {
            if(rtTbl->ucRTFlag[i] == SURFACE_STATE_ACTIVE_IN_LASTFRAME)
            {
                rtTbl->ucRTFlag[i] = SURFACE_STATE_INACTIVE;
            }
            else if(rtTbl->ucRTFlag[i] == SURFACE_STATE_ACTIVE_IN_CURFRAME)
            {
                rtTbl->ucRTFlag[i] = SURFACE_STATE_ACTIVE_IN_LASTFRAME;
            }
        }
    }
    else
    {
        for(int32_t i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i ++)
        {
            if(rtTbl->ucRTFlag[i])
            {
               rtTbl->ucRTFlag[i] --;
            }
        }
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiCodecBase::UpdateRegisteredRTSurfaceFlag(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, DDI_MEDIA_SURFACE *surface)
{
    DDI_CHK_NULL(rtTbl, "nullptr rtTbl", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_PARAMETER);

    for(int32_t i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i ++)
    {
        if(rtTbl->pRT[i] == surface)
        {
            rtTbl->ucRTFlag[i] = SURFACE_STATE_ACTIVE_IN_CURFRAME;
            return VA_STATUS_SUCCESS;
        }
    }
    DDI_VERBOSEMESSAGE("frame was not registered in the RTtbl");
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

VAStatus DdiCodecBase::UnRegisterRTSurfaces(DDI_CODEC_RENDER_TARGET_TABLE *rtTbl, DDI_MEDIA_SURFACE *surface)
{
    DDI_CHK_NULL(rtTbl, "nullptr rtTbl", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t i;

    for (i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i++)
    {
        if (rtTbl->pRT[i] == surface)
        {
            rtTbl->pRT[i] = nullptr;
            rtTbl->ucRTFlag[i] = SURFACE_STATE_INACTIVE;
            rtTbl->iNumRenderTargets--;
            break;
        }
    }
    if (i == DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
    {
        DDI_VERBOSEMESSAGE("The surface to be unregistered can not find in RTtbl!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    return VA_STATUS_SUCCESS;
}

}
