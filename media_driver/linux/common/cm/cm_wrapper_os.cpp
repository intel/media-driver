/*
* Copyright (c) 2007-2017, Intel Corporation
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
//! \file      cm_wrapper_os.cpp
//! \brief     Contains implementations of Linux-dependent functions for executing
//!            commands from cmrtlib.
//!

#include "cm_wrapper.h"

#include "cm_device_rt.h"
#include "cm_surface_2d_rt.h"
#include "media_libva_util.h"
#include "cm_extension_creator.h"
#include "mos_os_specific.h"

extern MOS_FORMAT Mos_Specific_FmtOsToMos(
    MOS_OS_FORMAT     format);

extern MOS_OS_FORMAT Mos_Specific_FmtMosToOs(
    MOS_FORMAT     format);

using CMRT_UMD::CmDeviceRT;
//!
//! \brief    Create Cm Device from VA Driver Context.
//! \details  Create a CmCtx and a associated MOS_CONTEXT. Put the CmCtx into
//!           the heap of VA Context.
//! \param    vaDriverCtx
//!           [in] pointer to va drv conetext.
//! \param    device
//!           [in,out] reference to cm device pointer.
//! \param    devOption
//!           [in] cm device creation option.
//! \return   int32_t
//!           CM_SUCCESS if success, else fail reason.
//!
int32_t CreateCmDeviceFromVA(VADriverContextP vaDriverCtx,
                             CmDevice* &device,
                             uint32_t devOption)
{
    int32_t                           hRes = CM_SUCCESS;
    PDDI_MEDIA_CONTEXT                mediaCtx;
    PCM_CONTEXT                       cmCtx;
    uint32_t                          ctxIndex;
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vaCtxHeapElement;
    VAContextID                       vaContextID;

    mediaCtx = DdiMedia_GetMediaContext(vaDriverCtx);

    // allocate cmCtx
    cmCtx = (PCM_CONTEXT)MOS_AllocAndZeroMemory(sizeof(CM_CONTEXT));
    CM_CHK_NULL_RETURN_WITH_MSG(cmCtx, CM_INVALID_UMD_CONTEXT, "Null cmCtx!");

    // init cmCtx
    cmCtx->mosCtx.bufmgr          = mediaCtx->pDrmBufMgr;
    cmCtx->mosCtx.m_gpuContextMgr = mediaCtx->m_gpuContextMgr;
    cmCtx->mosCtx.m_cmdBufMgr     = mediaCtx->m_cmdBufMgr;
    cmCtx->mosCtx.fd              = mediaCtx->fd;
    cmCtx->mosCtx.wRevision       = 0;
    cmCtx->mosCtx.iDeviceId       = mediaCtx->iDeviceId;
    cmCtx->mosCtx.SkuTable        = mediaCtx->SkuTable;
    cmCtx->mosCtx.WaTable         = mediaCtx->WaTable;
    cmCtx->mosCtx.gtSystemInfo    = *(mediaCtx->pGtSystemInfo);
    cmCtx->mosCtx.platform        = mediaCtx->platform;
    cmCtx->mosCtx.pPerfData       = (PERF_DATA *)MOS_AllocAndZeroMemory(sizeof(PERF_DATA));
    cmCtx->mosCtx.pGmmClientContext = mediaCtx->pGmmClientContext;
    cmCtx->mosCtx.m_osDeviceContext = mediaCtx->m_osDeviceContext;
    if(cmCtx->mosCtx.pPerfData == nullptr)
    {
        MOS_FreeMemAndSetNull(cmCtx); // free cm ctx
        CM_ASSERTMESSAGE("Failed to allocate perfData in mos context \n");
        return CM_OUT_OF_HOST_MEMORY;
    }

    // Create Cm Device
    hRes = CreateCmDevice(&(cmCtx->mosCtx), device, devOption);
    if(hRes != CM_SUCCESS)
    {
        MOS_FreeMemAndSetNull(cmCtx); // free cm ctx
        CM_ASSERTMESSAGE("Failed to call CmDevice::Create Error %d \n",hRes);
        return hRes;
    }
    CmDeviceRT* deviceRT = static_cast<CmDeviceRT*>(device);
    DdiMediaUtil_LockMutex(&mediaCtx->CmMutex);

    // get Free Cm context index
    vaCtxHeapElement = DdiMediaUtil_AllocPVAContextFromHeap(mediaCtx->pCmCtxHeap);
    if (nullptr == vaCtxHeapElement)
    {
        CmDeviceRT::Destroy(deviceRT); // destroy cm device
        device = nullptr;
        MOS_FreeMemAndSetNull(cmCtx); // free cm ctx
        DdiMediaUtil_UnLockMutex(&mediaCtx->CmMutex);
        CM_ASSERTMESSAGE("CM Context number exceeds maximum.");
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    // store cmCtx in pMedia
    vaCtxHeapElement->pVaContext    = (void *)cmCtx;
    vaContextID = (VAContextID)(vaCtxHeapElement->uiVaContextID + DDI_MEDIA_VACONTEXTID_OFFSET_CM);

    //Set VaCtx ID to Cm device
    deviceRT->SetVaCtxID(vaContextID);

    // increate CM context number
    mediaCtx->uiNumCMs++;

    DdiMediaUtil_UnLockMutex(&mediaCtx->CmMutex);

    return hRes;
}

//!
//! \brief    Destroy Cm Device and free heap in VA context.
//! \param    vaDriverCtx
//!           [in] pointer to va drv conetext.
//! \param    device
//!           [in] pointer to cm device to release.
//! \return   int32_t
//!           CM_SUCCESS if success, else fail reason.
//!
int32_t DestroyCmDeviceFromVA(VADriverContextP vaDriverCtx, CmDevice *device)
{
    int32_t               hr          = CM_SUCCESS;
    uint32_t              index;
    PDDI_MEDIA_CONTEXT    mediaCtx;
    PCM_CONTEXT           cmCtx;
    VAContextID           vaContextID;
    uint32_t              ctxType;
    VAStatus              vaStatus;

    if (nullptr == vaDriverCtx)
    {
        CM_ASSERTMESSAGE("Pointer to VADriverContext is invalid.");
        return CM_NULL_POINTER;
    }
    CmDeviceRT* deviceRT = static_cast<CmDeviceRT*>(device);
    if (nullptr == deviceRT) 
    {
        return CM_SUCCESS;
    }
    //Get VaCtx ID in MediaCtx
    deviceRT->GetVaCtxID(vaContextID);

    // Get Cm context index
    index = vaContextID & DDI_MEDIA_MASK_VACONTEXTID;

    //Get Cm Context
    cmCtx    = (PCM_CONTEXT)DdiMedia_GetContextFromContextID(vaDriverCtx, vaContextID, &ctxType);
    CM_CHK_NULL_RETURN_WITH_MSG(cmCtx, VA_STATUS_ERROR_INVALID_CONTEXT, "Null cmCtx.");

    CM_CHK_CMSTATUS_GOTOFINISH(DestroyCmDevice(device));

    // remove from context array
    mediaCtx = DdiMedia_GetMediaContext(vaDriverCtx);
    DdiMediaUtil_LockMutex(&mediaCtx->CmMutex);

    MOS_FreeMemAndSetNull(cmCtx->mosCtx.pPerfData);
    // destroy Cm context
    MOS_FreeMemAndSetNull(cmCtx);

    DdiMediaUtil_ReleasePVAContextFromHeap(mediaCtx->pCmCtxHeap, index);

    mediaCtx->uiNumCMs--;

    DdiMediaUtil_UnLockMutex(&mediaCtx->CmMutex);

finish:
    return hr;
}

extern MOS_FORMAT   VpGetFormatFromMediaFormat(DDI_MEDIA_FORMAT mf);
//*-----------------------------------------------------------------------------
//| Purpose:    Get resource information from LibVA-created surface and fill into OsResource
//              vaSurfaceID is the index to VA's surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmFillMosResource( VASurfaceID        vaSurfaceID,
                       VADriverContext*   vaDriverCtx,
                       PMOS_RESOURCE      osResource)
{
    PDDI_MEDIA_CONTEXT    mediaCtx;
    DDI_MEDIA_SURFACE     *surface;
    CmDevice              *device;

    CM_CHK_NULL_RETURN_WITH_MSG(vaDriverCtx, CM_INVALID_UMD_CONTEXT, "Null umdCtx");

    mediaCtx = DdiMedia_GetMediaContext(vaDriverCtx);
    CM_CHK_NULL_RETURN_WITH_MSG(mediaCtx, CM_INVALID_UMD_CONTEXT, "Null mediaCtx");

    CM_CHK_NULL_RETURN_WITH_MSG(mediaCtx->pSurfaceHeap, CM_INVALID_UMD_CONTEXT, "Null mediaCtx->pSurfaceHeap");
    CM_CHK_COND_RETURN(((uint32_t)vaSurfaceID >= mediaCtx->pSurfaceHeap->uiAllocatedHeapElements), CM_INVALID_LIBVA_SURFACE, "Invalid surface");
    surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, vaSurfaceID);
    CM_CHK_NULL_RETURN_WITH_MSG(surface, CM_INVALID_LIBVA_SURFACE, "Null surface");
    CM_ASSERT(surface->iPitch == GFX_ULONG_CAST(surface->pGmmResourceInfo->GetRenderPitch()));
    CM_CHK_NULL_RETURN_WITH_MSG(surface->bo, CM_INVALID_LIBVA_SURFACE, "Null BO");
    CM_CHK_NULL_RETURN_WITH_MSG(surface->pGmmResourceInfo, CM_INVALID_LIBVA_SURFACE, "Null GMMResInfo");

    // Resets the Resource
    Mos_ResetResource(osResource);

    osResource->iWidth   = surface->iWidth;
    osResource->iHeight  = surface->iHeight;
    osResource->iDepth   = 1;
    osResource->iPitch   = surface->iPitch;

    osResource->iCount   = 0;
    osResource->bufname  = (char *)"Libva2DSurface";

    osResource->Format   = VpGetFormatFromMediaFormat(surface->format);
    osResource->bo       = surface->bo;

    osResource->TileType = LinuxToMosTileType(surface->TileType);
    osResource->isTiled  = surface->isTiled;

    osResource->bMapped  = surface->bMapped;
    osResource->pData    = (uint8_t*) surface->bo->virt;

    osResource->pGmmResInfo = surface->pGmmResourceInfo;

    // for wrapper to new MOS MODS interface
    osResource->bConvertedFromDDIResource = true;

    return CM_SUCCESS;
}

MOS_FORMAT CmOSFmtToMosFmt(CM_OSAL_SURFACE_FORMAT format)
{
    //CM_SURFACE_FORMAT_R8U/R16U are not valid va surface format.
    //These two formats are MDF specific, so we add the mapping here.
    switch(format)
    {
      case CM_SURFACE_FORMAT_R8U:  return Format_R8U;
      case CM_SURFACE_FORMAT_R16U: return Format_R16U;
      default:
        return Mos_Specific_FmtOsToMos(format);
    }
}

CM_OSAL_SURFACE_FORMAT  CmMosFmtToOSFmt(MOS_FORMAT format)
{
    //CM_SURFACE_FORMAT_R8U/R16U are not valid va surface format.
    //These two formats are MDF specific, so we add the mapping here.
    switch(format)
    {
        case Format_R8U:   return CM_SURFACE_FORMAT_R8U;
        case Format_R16U:  return CM_SURFACE_FORMAT_R16U;
        default:
           return Mos_Specific_FmtMosToOs(format);
    }
}

using CMRT_UMD::CmSurface2DRT;
using CMRT_UMD::CmWrapperEx;
//*-----------------------------------------------------------------------------
//| Purpose:    CMRT thin layer library supported function execution
//| Return:     CM_SUCCESS if successful
//*-----------------------------------------------------------------------------
int32_t CmThinExecute(VADriverContextP vaDriverCtx,
                      void *deviceHandle,
                      uint32_t inputFunctionId,
                      void *inputData,
                      uint32_t inputDataLen)
{
    CmDevice             *device           = nullptr;
    CmDeviceRT           *deviceRT         = nullptr;
    VADriverContextP     hUMDevice          = nullptr;
    void                 *cmPrivateInputData = nullptr;
    uint32_t             cmPrivateInputDataSize = 0 ;
    CMRT_UMD::CmSurface2D *pCmSurface2d       = nullptr;
    SurfaceIndex         *surfaceIndex      = nullptr;
    CM_FUNCTION_ID       cmFunctionID;
    int32_t              hr                  = CM_SUCCESS;
    int32_t              cmRet               = CM_INVALID_PRIVATE_DATA;

    hUMDevice               = vaDriverCtx;
    cmPrivateInputData     = inputData;
    cmPrivateInputDataSize  = inputDataLen;
    cmFunctionID            = (CM_FUNCTION_ID)inputFunctionId;
    device                 = (CmDevice *)deviceHandle;
    deviceRT               = static_cast<CmDeviceRT*>(device);
    switch(cmFunctionID)
    {
        case CM_FN_CREATECMDEVICE:
            PCM_CREATECMDEVICE_PARAM cmDeviceParam;
            cmDeviceParam = (PCM_CREATECMDEVICE_PARAM)(cmPrivateInputData);
            //Create Cm Device
            cmRet = CreateCmDeviceFromVA(vaDriverCtx, device, cmDeviceParam->devCreateOption);
            if ( cmRet == CM_SUCCESS)
            {
                CM_CHK_NULL_RETURN_WITH_MSG(device, VA_STATUS_ERROR_INVALID_CONTEXT, "Null device.");
                deviceRT = static_cast<CmDeviceRT*>(device);
                deviceRT->RegisterCallBack(cmDeviceParam->callbackReleaseVaSurf);
                cmDeviceParam->driverStoreEnabled = deviceRT->GetDriverStoreFlag();
            }
            //Fill the output message
            cmDeviceParam->deviceHandle = device;
            cmDeviceParam->returnValue  = cmRet;
            cmDeviceParam->version      = CM_VERSION;
            break;

        case CM_FN_DESTROYCMDEVICE:
            PCM_DESTROYCMDEVICE_PARAM cmDevDestroyParam;
            cmDevDestroyParam = (PCM_DESTROYCMDEVICE_PARAM)(cmPrivateInputData);
            device            = (CmDevice *)(cmDevDestroyParam->deviceHandle);
            cmRet = DestroyCmDeviceFromVA(vaDriverCtx,device);
            //Fill the output message
            cmDevDestroyParam->deviceHandle = nullptr;
            cmDevDestroyParam->returnValue    = cmRet;
            break;

        case CM_FN_CMDEVICE_CREATESURFACE2D:
            PCM_CREATESURFACE2D_PARAM   cmCreate2DParam;
            MOS_RESOURCE                mosResource ;
            MOS_ZeroMemory(&mosResource, sizeof(MOS_RESOURCE));
            cmCreate2DParam    = (PCM_CREATESURFACE2D_PARAM)(cmPrivateInputData);
            if ( cmCreate2DParam->isLibvaCreated )
            {
                //LibVA-created Surface2D
                cmRet = CmFillMosResource(cmCreate2DParam->vaSurfaceID,
                                       vaDriverCtx,
                                       &mosResource);

                if( cmRet != CM_SUCCESS)
                {
                    CM_ASSERTMESSAGE("Error: Failed to fill MOS resource.");
                    cmCreate2DParam->returnValue          = cmRet;
                    return cmRet;
                }

                cmRet = deviceRT->CreateSurface2D(&mosResource, cmCreate2DParam->isCmCreated, pCmSurface2d);
                if( cmRet != CM_SUCCESS)
                {
                    CM_ASSERTMESSAGE("Error: Failed to create surface 2D from MOS resource.");
                    cmCreate2DParam->returnValue          = cmRet;
                    return cmRet;
                }

                CmSurface2DRT *surface2dRT = static_cast<CmSurface2DRT *>(pCmSurface2d);
                surface2dRT->SetVaSurfaceID(cmCreate2DParam->vaSurfaceID, cmCreate2DParam->vaDisplay);
            }
            else
            {
                // CM Created Surface2D
                cmRet = device->CreateSurface2D(
                        cmCreate2DParam->width,
                        cmCreate2DParam->height,
                        CmOSFmtToMosFmt(cmCreate2DParam->format),
                        pCmSurface2d);
            }
            //Create Surface Index
            if( cmRet == CM_SUCCESS)
            {
                cmCreate2DParam->cmSurface2DHandle    = pCmSurface2d;
            }

            //Fill output message
            cmCreate2DParam->returnValue          = cmRet;
            break;

        default:
            hr = CmThinExecuteInternal(device, cmFunctionID, cmPrivateInputData, cmPrivateInputDataSize);
            if (hr == CM_INVALID_PRIVATE_DATA)
        {
                CmWrapperEx *wrapperEx = CmExtensionCreator<CmWrapperEx>::CreateClass();
                if (wrapperEx != nullptr)
                {
                    wrapperEx->Initialize((void *)vaDriverCtx);
                    hr = wrapperEx->Execute(device,cmFunctionID, cmPrivateInputData, cmPrivateInputDataSize);
                    MOS_Delete(wrapperEx);
        }
                else
                {
                    hr = CM_OUT_OF_HOST_MEMORY;
                }
            }
    }

    return hr;
}
