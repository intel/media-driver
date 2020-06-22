/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     media_libva_vp_tools.c
//! \brief    LibVA Video Processing tool functions implementation
//!

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <va/va.h>
#include <va/va_vpp.h>
#include <va/va_backend.h>

#include "vphal.h"
#include "vphal_ddi.h"

#include "media_libva.h"
#include "media_libva_util.h"
#include "media_libva_vp.h"
#include "media_libva_vp_tools.h"

#if (_DEBUG || _RELEASE_INTERNAL)

VAStatus VpInitDumpConfig(
    PDDI_VP_CONTEXT     pVpCtx)
{
    int32_t     uSurfIndex;
    VAStatus    vaStatus = VA_STATUS_SUCCESS;

    pVpCtx->pCurVpDumpDDIParam = nullptr;
    pVpCtx->pPreVpDumpDDIParam = nullptr;

    // allocate pCurVpDumpDDIParam
    pVpCtx->pCurVpDumpDDIParam = (PDDI_VP_DUMP_PARAM)MOS_AllocAndZeroMemory(sizeof(DDI_VP_DUMP_PARAM));
    if (pVpCtx->pCurVpDumpDDIParam)
    {
        for (uSurfIndex = 0; uSurfIndex < VPHAL_MAX_SOURCES; uSurfIndex++)
        {
            pVpCtx->pCurVpDumpDDIParam->pPipelineParamBuffers[uSurfIndex] = (VAProcPipelineParameterBuffer *)MOS_AllocAndZeroMemory(sizeof(VAProcPipelineParameterBuffer));
            if (nullptr == pVpCtx->pCurVpDumpDDIParam->pPipelineParamBuffers[uSurfIndex])
            {
                vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
                goto FINISH;
            }
        }
    }
    // allocate pPreVpDumpDDIParam
    pVpCtx->pPreVpDumpDDIParam = (PDDI_VP_DUMP_PARAM)MOS_AllocAndZeroMemory(sizeof(DDI_VP_DUMP_PARAM));
    if (pVpCtx->pPreVpDumpDDIParam)
    {
        for (uSurfIndex = 0; uSurfIndex < VPHAL_MAX_SOURCES; uSurfIndex++)
        {
            pVpCtx->pPreVpDumpDDIParam->pPipelineParamBuffers[uSurfIndex] = (VAProcPipelineParameterBuffer *)MOS_AllocAndZeroMemory(sizeof(VAProcPipelineParameterBuffer));
            if (nullptr == pVpCtx->pPreVpDumpDDIParam->pPipelineParamBuffers[uSurfIndex])
            {
                vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
                goto FINISH;
            }
        }
    }

FINISH:
    pVpCtx->fpDumpFile = nullptr;
    return vaStatus;
}

void VpDestoryDumpConfig(
    PDDI_VP_CONTEXT     pVpCtx)
{
    int32_t     uSurfIndex;
    VAStatus    vaStatus = VA_STATUS_SUCCESS;

    if (pVpCtx->pCurVpDumpDDIParam)
    {
        for (uint32_t uSurfIndex = 0; uSurfIndex < VPHAL_MAX_SOURCES; uSurfIndex++)
        {
            if (nullptr != pVpCtx->pCurVpDumpDDIParam->pPipelineParamBuffers[uSurfIndex])
            {
                MOS_FreeMemAndSetNull(pVpCtx->pCurVpDumpDDIParam->pPipelineParamBuffers[uSurfIndex]);
            }
        }
        MOS_FreeMemAndSetNull(pVpCtx->pCurVpDumpDDIParam);
    }

    if (nullptr != pVpCtx->pPreVpDumpDDIParam)
    {
        for (uint32_t uSurfIndex = 0; uSurfIndex < VPHAL_MAX_SOURCES; uSurfIndex++)
        {
            if (nullptr != pVpCtx->pPreVpDumpDDIParam->pPipelineParamBuffers[uSurfIndex])
            {
                MOS_FreeMemAndSetNull(pVpCtx->pPreVpDumpDDIParam->pPipelineParamBuffers[uSurfIndex]);
            }
        }
        MOS_FreeMemAndSetNull(pVpCtx->pPreVpDumpDDIParam);
    }

    if (nullptr != pVpCtx->fpDumpFile)
    {
        fclose(pVpCtx->fpDumpFile);
        pVpCtx->fpDumpFile = nullptr;
    }
}

//!
//! \brief   dump deinterlacing parameter
//! \param   fpLog
//!          [in] file pointer pointed to dumpped file
//! \param   deint
//!          [in] pointed to deinterlacing parameter
//!
void VpDumpDeinterlacingParameterBuffer(
    FILE                                      *fpLog,
    VAProcFilterParameterBufferDeinterlacing  *deint)
{
    if (deint && fpLog)
    {
        fprintf(fpLog, "\t    type = %d\n",      deint->type);
        fprintf(fpLog, "\t    algorithm = %d\n", deint->algorithm);
        fprintf(fpLog, "\t    flags = %d\n",     deint->flags);
    }
}

//!
//! \brief   dump color balance parameter
//! \param   fpLog
//!          [in] file pointer pointed to dumpped file
//! \param   colorbalance
//!          [in] pointed to colorbalance parameter
//! \param   uElementNum
//!          [in] number of elements
//!
void VpDumpColorBalanceParameterBuffer(
    FILE                                     *fpLog,
    VAProcFilterParameterBufferColorBalance  *colorbalance,
    uint32_t                                 uElementNum)
{
    for (uint32_t i = 0; i<uElementNum; i++)
    {
        if (fpLog && colorbalance)
        {
            fprintf(fpLog, "\t    type = %d\n",   colorbalance[i].type);
            fprintf(fpLog, "\t    attrib = %d\n", colorbalance[i].attrib);
            fprintf(fpLog, "\t    value = %f\n",  colorbalance[i].value);
        }
    }
}

//!
//! \brief   dump TCC parameter
//! \param   fpLog
//!          [in] file pointer pointed to dumpped file
//! \param   filter_param
//!          [in] pointed to TCC parameter
//! \param   uElementNum
//!          [in] number of elements
//!
void VpDumpTotalColorCorrectionParameterBuffer(
    FILE                                             *fpLog,
    VAProcFilterParameterBufferTotalColorCorrection  *filter_param,
    uint32_t                                         uElementNum)
{
    for (uint32_t i = 0; i<uElementNum; i++)
    {
        if (fpLog && filter_param)
        {
            fprintf(fpLog, "\t    type = %d\n",   filter_param[i].type);
            fprintf(fpLog, "\t    attrib = %d\n", filter_param[i].attrib);
            fprintf(fpLog, "\t    value = %f\n",  filter_param[i].value);
        }
    }
}

//!
//! \brief   dump filter parameter
//! \param   fpLog
//!          [in] file pointer pointed to dumpped file
//! \param   buffer
//!          [in] pointed to filter parameter
//!
void VpDumpFilterParameterBuffer(
    FILE                            *fpLog,
    VAProcFilterParameterBuffer     *buffer)
{
    if (fpLog && buffer)
    {
        fprintf(fpLog, "\t    type = %d\n",  buffer->type);
        fprintf(fpLog, "\t    value = %f\n", buffer->value);
    }
}

//!
//! \brief   dump filters parameter
//! \param   fpLog
//!          [in] file pointer pointed to dumpped file
//! \param   pVaDrvCtx
//!          [in] driver context
//! \param   filters
//!          [in] pointed to filters
//! \param   num_filters
//!          [in] number of filters
//!
void VpDumpProcFiltersParameterBuffer(
    FILE                *fpLog,
    VADriverContextP    pVaDrvCtx,
    VABufferID          *filters,
    unsigned int        num_filters)
{
    VABufferType        type;
    unsigned int        size;
    unsigned int        num_elements;
    PDDI_MEDIA_CONTEXT  pMediaCtx;
    void                *pData;
    PDDI_MEDIA_BUFFER   pFilterBuf;

    size                = 0;
    num_elements        = 0;
    pFilterBuf          = nullptr;
    pMediaCtx           = nullptr;
    pData               = nullptr;

    if (nullptr == fpLog || nullptr == pVaDrvCtx)
    {
        return;
    }

    pMediaCtx = DdiMedia_GetMediaContext(pVaDrvCtx);

    fprintf(fpLog, "\t  num_filters = %d\n", num_filters);
    fprintf(fpLog, "\t  filters = %p\n", filters);

    if (num_filters == 0 || filters == nullptr)
    {
        return;
    }

    // get buffer type information
    for (int i = 0; i < num_filters; i++)
    {
        VABufferID filter = filters[i];
        pFilterBuf = DdiMedia_GetBufferFromVABufferID(pMediaCtx, filter);

        // Map Buffer data to virtual addres space
        DdiMedia_MapBuffer(pVaDrvCtx, filter, &pData);

        VAProcFilterParameterBufferBase* filter_param = (VAProcFilterParameterBufferBase*) pData;
        if (nullptr != pFilterBuf)
        {
            fprintf(fpLog, "\t  num_elements = %d\n", pFilterBuf->uiNumElements);
        }

        if (nullptr != filter_param)
        {
            switch (filter_param->type)
            {
            case VAProcFilterDeinterlacing:
                VpDumpDeinterlacingParameterBuffer(fpLog, (VAProcFilterParameterBufferDeinterlacing *)pData);
                break;

            case VAProcFilterColorBalance:
                if (nullptr != pFilterBuf)
                {
                    VpDumpColorBalanceParameterBuffer(fpLog, (VAProcFilterParameterBufferColorBalance *)pData, pFilterBuf->uiNumElements);
                }
                break;

            case VAProcFilterTotalColorCorrection:
                if (nullptr != pFilterBuf)
                {
                    VpDumpTotalColorCorrectionParameterBuffer(fpLog, (VAProcFilterParameterBufferTotalColorCorrection *)pData, pFilterBuf->uiNumElements);
                }
                break;

            case VAProcFilterNoiseReduction:
                VpDumpFilterParameterBuffer(fpLog, (VAProcFilterParameterBuffer *)pData);
                break;

            case VAProcFilterSharpening:
                VpDumpFilterParameterBuffer(fpLog, (VAProcFilterParameterBuffer *)pData);
                break;

            case VAProcFilterSkinToneEnhancement:
                VpDumpFilterParameterBuffer(fpLog, (VAProcFilterParameterBuffer *)pData);
                break;

            default:
                fprintf(fpLog, "\t    type = %d\n", filter_param->type);
                break;
            }
        }
    }
}

//!
//! \brief   read a config "env" for vpddi.conf or from environment setting
//! \param   env
//!          [in] Pipeline parameters from application (VAProcPipelineParameterBuffer)
//! \param   env_value
//!          [out]
//! \return  int
//!          return 0, if the "env" is set, and the value is copied into env_value
//!          return 1, if the env is not set
//!
int  VpParseLogConfig(
    const char *env,
    char       *env_value)
{
    char *token, *value, *saveptr;
    char oneline[1024];
    FILE *fp = nullptr;

    if (env == nullptr)
    {
        return 1;
    }

    fp = fopen("/etc/vpddi.conf", "r");
    while (fp && (fgets(oneline, 1024, fp) != nullptr))
    {
        if (strlen(oneline) == 1)
        {
            continue;
        }

        token = strtok_r(oneline, "=\n", &saveptr);
        value = strtok_r(nullptr, "=\n", &saveptr);

        if (nullptr == token || nullptr == value)
        {
            continue;
        }

        if (strcmp(token, env) == 0)
        {
            if (env_value)
            {
                strncpy(env_value, value, 1024);
            }

            fclose(fp);
            fp = nullptr;
            return 0;
        }
    }
    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }

    // no setting in config file, use env setting
    value = getenv(env);
    if (value)
    {
        if (env_value)
        {
            strncpy(env_value, value, 1024);
        }

        return 0;
    }

    return 1;
}

//!
//! \brief   compare current dump parameters with the previous
//! \param   pCur
//!          [in] current dump parameters
//! \param   pPre
//!          [in] previous dump parameters
//! \return  bool
//!          return true, if current parameters and previous are same
//!          return false, if current parameters and previous are different
//!
bool VpCmpDDIDumpParam(
    PDDI_VP_DUMP_PARAM  pCur,
    PDDI_VP_DUMP_PARAM  pPre)
{
    bool bRet = true;

    if (!pCur || !pPre)
    {
        return false;
    }

    if (memcmp(pCur->SrcFormat, pPre->SrcFormat, sizeof(MOS_FORMAT)*VPHAL_MAX_SOURCES) != 0)
    {
         bRet = false;
    }

    if (memcmp(pCur->TargetFormat, pPre->TargetFormat, sizeof(MOS_FORMAT)*VPHAL_MAX_TARGETS) != 0)
    {
         bRet = false;
    }

    for (uint32_t uSurfIndex = 0; uSurfIndex < VPHAL_MAX_SOURCES; uSurfIndex++)
    {
        if (memcmp(pCur->pPipelineParamBuffers[uSurfIndex], pPre->pPipelineParamBuffers[uSurfIndex], sizeof(VAProcPipelineParameterBuffer)) != 0)
        {
            bRet = false;
        }
    }

    return bRet;
}

//!
//! \brief   initialize configuration values for Android 
//! \param   [in] pConfigValues
//!          vp config values
//!
void VpConfigValuesInit(
    PVP_CONFIG           pConfigValues)
{
    pConfigValues->dwVpPath                   = 0;
    pConfigValues->dwVpComponent              = 0;
    pConfigValues->dwReportedDeinterlaceMode  = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwReportedScalingMode      = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwReportedOutputPipeMode   = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwReportedVEFeatureInUse   = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwVPMMCInUseReported       = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwRTCompressibleReported   = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwRTCompressModeReported   = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwCapturePipeInUseReported = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwReportedCompositionMode  = LIBVA_VP_CONFIG_NOT_REPORTED;

    pConfigValues->dwFFDICompressibleReported    = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwFFDICompressModeReported    = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwFFDNCompressibleReported    = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwFFDNCompressModeReported    = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwSTMMCompressibleReported    = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwSTMMCompressModeReported    = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwScalerCompressibleReported  = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwScalerCompressModeReported  = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwPrimaryCompressibleReported = LIBVA_VP_CONFIG_NOT_REPORTED;
    pConfigValues->dwPrimaryCompressModeReported = LIBVA_VP_CONFIG_NOT_REPORTED;
}

void VpFeatureReport(
    PVP_CONFIG         pConfig)
{
    WriteUserFeature(__VPHAL_VEBOX_OUTPUTPIPE_MODE_ID,         pConfig->dwCurrentOutputPipeMode);
    WriteUserFeature(__VPHAL_VEBOX_FEATURE_INUSE_ID,           pConfig->dwCurrentVEFeatureInUse);
    WriteUserFeature(__VPHAL_ENABLE_MMC_IN_USE_ID,             pConfig->dwVPMMCInUse);

#ifdef _MMC_SUPPORTED
    //VP MMC In Use
    WriteUserFeature(__VPHAL_ENABLE_MMC_IN_USE_ID,             pConfig->dwVPMMCInUse);
    //VP Primary Surface Compress Mode Report
    WriteUserFeature(__VPHAL_PRIMARY_SURFACE_COMPRESS_MODE_ID, pConfig->dwPrimaryCompressMode);
    //VP Primary Surface Compressible
    WriteUserFeature(__VPHAL_PRIMARY_SURFACE_COMPRESSIBLE_ID,  pConfig->dwPrimaryCompressible);
    //VP RT Compress Mode
    WriteUserFeature(__VPHAL_RT_COMPRESS_MODE_ID,              pConfig->dwRTCompressMode);
    //VP RT Compressible
    WriteUserFeature(__VPHAL_RT_COMPRESSIBLE_ID,               pConfig->dwRTCompressible);
#endif
}

VAStatus    VpReportFeatureMode(PDDI_VP_CONTEXT pVpCtx)
{
    VphalState            *pVpHal;
    VP_CONFIG             ConfigValues;

    DDI_CHK_NULL(pVpCtx,         "Null pVpCtx.",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pVpCtx->pVpHal, "Null pVpHal.",   VA_STATUS_ERROR_INVALID_PARAMETER);

    VpConfigValuesInit(&ConfigValues);

    VpHal_DdiReportFeatureMode(pVpCtx->pVpHal, &ConfigValues);

    VpFeatureReport(&ConfigValues);

    return VA_STATUS_SUCCESS;
}

VAStatus VpDumpProcPipelineParams(
    VADriverContextP        pVaDrvCtx,
    PDDI_VP_CONTEXT         pVpCtx)
{
    PDDI_MEDIA_CONTEXT      pMediaCtx;
    FILE                    *fpLog;
    long                    suffix;
    struct timeval          time;
    VPHAL_RENDER_PARAMS     *pRenderParams;
    PDDI_MEDIA_SURFACE      pMediaSrcSurf;
    bool                    bDumpToFile;
    PDDI_VP_DUMP_PARAM      pCurDDIParam;
    PDDI_VP_DUMP_PARAM      pPreDDIParam;
    char                    env_value[1024];

    DDI_CHK_NULL(pVaDrvCtx,                  "Null pVaDrvCtx.",       VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pVpCtx,                     "Null pVpCtx.",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pVpCtx->pVpHalRenderParams, "Null Render Params.",   VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pVpCtx->pCurVpDumpDDIParam, "Null Dump Params.",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pVpCtx->pPreVpDumpDDIParam, "Null Dump Params.",     VA_STATUS_ERROR_INVALID_PARAMETER);

    for (uint32_t index = 0; index < pVpCtx->pVpHalRenderParams->uDstCount; index++)
    {
        pVpCtx->pCurVpDumpDDIParam->TargetFormat[index] = pVpCtx->pVpHalRenderParams->pTarget[index]->Format;
    }

    pCurDDIParam = pVpCtx->pCurVpDumpDDIParam;
    pPreDDIParam = pVpCtx->pPreVpDumpDDIParam;

    // If current ddi param is same with the previous, don't dump into file.
    bDumpToFile = VpCmpDDIDumpParam(pCurDDIParam, pPreDDIParam);
    if (bDumpToFile)
    {
       return VA_STATUS_SUCCESS;
    }

    fpLog              = nullptr;
    pMediaCtx          = DdiMedia_GetMediaContext(pVaDrvCtx);
    pRenderParams      = pVpCtx->pVpHalRenderParams;
    pMediaSrcSurf      = nullptr;

    gettimeofday(&time, nullptr);
    suffix = ((long)time.tv_sec) * 1000 + (long)time.tv_usec / 1000;

    if (pVpCtx->fpDumpFile == nullptr)
    {
        const char * pTraceName = "VPDDI_TRACE";
        if (VpParseLogConfig(pTraceName, &env_value[0]) == 0)
        {
            int32_t tmp = strnlen(env_value, sizeof(env_value));
            int32_t left = sizeof(env_value) - tmp;
            snprintf(env_value+tmp, left, "%ld", suffix);
            MOS_SecureFileOpen(&pVpCtx->fpDumpFile, env_value, "w");
        }
    }
    fpLog = pVpCtx->fpDumpFile;

    if (fpLog)
    {
        fprintf(fpLog, "\t-------------------------------\n");
        fprintf(fpLog, "\tTargetFormat Count = %d\n", pRenderParams->uDstCount);
        for (uint32_t uDstIndex = 0; uDstIndex < pRenderParams->uDstCount; uDstIndex++)
        {
            fprintf(fpLog, "\tTargetFormat = %d\n", (int32_t)(pCurDDIParam->TargetFormat[uDstIndex]));
        }
        for (uint32_t uSurfIndex = 0; uSurfIndex < pRenderParams->uSrcCount; uSurfIndex++)
        {
            VAProcPipelineParameterBuffer  *p = pCurDDIParam->pPipelineParamBuffers[uSurfIndex];
            if (p)
            {
                fprintf(fpLog, "\t--VAProcPipelineParameterBuffer\n");
                fprintf(fpLog, "\t  surface = 0x%08x\n", p->surface);
                fprintf(fpLog, "\t    Format = %d\n", (int32_t)(pCurDDIParam->SrcFormat[uSurfIndex]));

                if (p->surface_region)
                {
                    fprintf(fpLog, "\t  surface_region\n");
                    fprintf(fpLog, "\t    x = %d\n", p->surface_region->x);
                    fprintf(fpLog, "\t    y = %d\n", p->surface_region->y);
                    fprintf(fpLog, "\t    width = %d\n", p->surface_region->width);
                    fprintf(fpLog, "\t    height = %d\n", p->surface_region->height);
                }

                fprintf(fpLog, "\t  surface_color_standard = %d\n", p->surface_color_standard);

                if (p->output_region)
                {
                    fprintf(fpLog, "\t  output_region\n");
                    fprintf(fpLog, "\t    x = %d\n", p->output_region->x);
                    fprintf(fpLog, "\t    y = %d\n", p->output_region->y);
                    fprintf(fpLog, "\t    width = %d\n", p->output_region->width);
                    fprintf(fpLog, "\t    height = %d\n", p->output_region->height);
                }

                fprintf(fpLog, "\t  output_background_color = 0x%08x\n", p->output_background_color);
                fprintf(fpLog, "\t  output_color_standard = %d\n", p->output_color_standard);
                fprintf(fpLog, "\t  pipeline_flags = 0x%08x\n", p->pipeline_flags);
                fprintf(fpLog, "\t  filter_flags = 0x%08x\n", p->filter_flags);

                VpDumpProcFiltersParameterBuffer(fpLog, pVaDrvCtx, p->filters, p->num_filters);

                fprintf(fpLog, "\t  num_forward_references = 0x%08x\n", p->num_forward_references);

                if (p->num_forward_references)
                {
                    fprintf(fpLog, "\t  forward_references\n");

                    if (p->forward_references)
                    {
                        /* only dump the first 5 forward references */
                        for (int32_t i = 0; i < p->num_forward_references && i < 5; i++)
                        {
                            fprintf(fpLog, "\t    forward_references[%d] = 0x%08x\n", i, p->forward_references[i]);
                        }
                    }
                    else
                    {
                        for (int32_t i = 0; i < p->num_forward_references && i < 5; i++)
                        {
                            fprintf(fpLog, "\t    forward_references[%d] = (nullptr)\n", i);
                        }
                    }
                }

                fprintf(fpLog, "\t  num_backward_references = 0x%08x\n", p->num_backward_references);

                if (p->num_backward_references)
                {
                    fprintf(fpLog, "\t  backward_references\n");

                    if (p->backward_references)
                    {
                        /* only dump the first 5 backward references */
                        for (int32_t i = 0; i < p->num_backward_references && i < 5; i++)
                        {
                            fprintf(fpLog, "\t    backward_references[%d] = 0x%08x\n", i, p->backward_references[i]);
                        }
                    }
                    else
                    {
                        for (int32_t i = 0; i < p->num_backward_references && i < 5; i++)
                        {
                            fprintf(fpLog, "\t    backward_references[%d] = (nullptr)\n", i);
                        }
                    }
                }

                fprintf(fpLog, "\t  rotation_state = %d\n", p->rotation_state);
                if (p->blend_state)
                {
                    fprintf(fpLog, "\t  blend_state\n");
                    fprintf(fpLog, "\t    flags = %d\n", p->blend_state->flags);
                    fprintf(fpLog, "\t    global_alpha = %f\n", p->blend_state->global_alpha);
                    fprintf(fpLog, "\t    min_luma = %f\n", p->blend_state->min_luma);
                    fprintf(fpLog, "\t    max_luma = %f\n", p->blend_state->max_luma);
                }

                fprintf(fpLog, "\t  mirror_state = %d\n", p->mirror_state);
                //fprintf(fpLog, "\t  additional_outputs = 0x%08x\n", (unsigned int32_t)(*(p->additional_outputs)));
                fprintf(fpLog, "\t  num_additional_outputs = %d\n", p->num_additional_outputs);
#if (VA_MAJOR_VERSION < 1)
                fprintf(fpLog, "\t  chroma_siting_flag = %d\n", p->input_surface_flag & 0x3);
#else
                fprintf(fpLog, "\t  chroma_siting_flag = %d\n", p->input_color_properties.chroma_sample_location & 0x3);
#endif
            }
        }

        MOS_SecureMemcpy(pPreDDIParam->SrcFormat, sizeof(MOS_FORMAT)*VPHAL_MAX_SOURCES, pCurDDIParam->SrcFormat, sizeof(MOS_FORMAT)*VPHAL_MAX_SOURCES);
        MOS_SecureMemcpy(pPreDDIParam->TargetFormat, sizeof(MOS_FORMAT)*VPHAL_MAX_TARGETS, pCurDDIParam->TargetFormat, sizeof(MOS_FORMAT)*VPHAL_MAX_TARGETS);
        for (uint32_t uSurfIndex = 0; uSurfIndex < VPHAL_MAX_SOURCES; uSurfIndex++)
        {
            MOS_SecureMemcpy(pPreDDIParam->pPipelineParamBuffers[uSurfIndex], sizeof(VAProcPipelineParameterBuffer), pCurDDIParam->pPipelineParamBuffers[uSurfIndex], sizeof(VAProcPipelineParameterBuffer));
        }
    }

    return VA_STATUS_SUCCESS;
}

#endif //(_DEBUG || _RELEASE_INTERNAL)
