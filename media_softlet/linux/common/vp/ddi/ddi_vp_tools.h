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
//! \file     ddi_vp_tools.h
//! \brief    ddi vp tool functions implementation
//!

#ifndef __DDI_VP_TOOLS_H__
#define __DDI_VP_TOOLS_H__

#include <va/va.h>
#include <va/va_vpp.h>
#include "ddi_vp_functions.h"

#define LIBVA_VP_CONFIG_NOT_REPORTED  0xffffffff

class DdiVpTools
{
public:
#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief  Allocate and initialize DDI dump parameters
    //!
    //! \param  [in] vpCtx
    //!          VP context
    //!
    //! \return VAStatus
    //! VA_STATUS_SUCCESS if DDI Params is initialized
    static VAStatus InitDumpConfig(PDDI_VP_CONTEXT vpCtx);

    //!
    //! \brief  Destory DDI dump parameters
    //!
    //! \param  [in] vpCtx
    //!          VP context
    //!
    //! \return
    //!
    static void DestoryDumpConfig(PDDI_VP_CONTEXT  vpCtx);

    //!
    //! \brief   compare current dump parameters with the previous
    //!
    //! \param   [in] curParams
    //!          current dump parameters
    //! \param   [in] preParams
    //!          previous dump parameters
    //! \return  bool
    //!          return true, if current parameters and previous are same
    //!          return false, if current parameters and previous are different
    //!
    static bool VpCmpDDIDumpParam(
        PDDI_VP_DUMP_PARAM curParams,
        PDDI_VP_DUMP_PARAM preParams);

    //!
    //! \brief   read a config "env" for vpddi.conf or from environment setting
    //!
    //! \param   env
    //!          [in] Pipeline parameters from application (VAProcPipelineParameterBuffer)
    //! \param   envValue
    //!          [out]
    //! \return  int
    //!          return 0, if the "env" is set, and the value is copied into envValue
    //!          return 1, if the env is not set
    //!
    static int  VpParseLogConfig(
        const char *env,
        char       *envValue);

    //!
    //! \brief   dump deinterlacing parameter
    //!
    //! \param   fpLog
    //!          [in] file pointer pointed to dumpped file
    //! \param   deint
    //!          [in] pointed to deinterlacing parameter
    //!
    static void VpDumpDeinterlacingParameterBuffer(
        FILE                                      *fpLog,
        VAProcFilterParameterBufferDeinterlacing  *deint);

    //!
    //! \brief   dump color balance parameter
    //!
    //! \param   fpLog
    //!          [in] file pointer pointed to dumpped file
    //! \param   colorbalance
    //!          [in] pointed to colorbalance parameter
    //! \param   elementNum
    //!          [in] number of elements
    //!
    static void VpDumpColorBalanceParameterBuffer(
        FILE                                     *fpLog,
        VAProcFilterParameterBufferColorBalance  *colorbalance,
        uint32_t                                 elementNum);

    //!
    //! \brief   dump TCC parameter
    //!
    //! \param   fpLog
    //!          [in] file pointer pointed to dumpped file
    //! \param   filterParam
    //!          [in] pointed to TCC parameter
    //! \param   elementNum
    //!          [in] number of elements
    //!
    static void VpDumpTotalColorCorrectionParameterBuffer(
        FILE                                             *fpLog,
        VAProcFilterParameterBufferTotalColorCorrection  *filterParam,
        uint32_t                                         elementNum);

    //!
    //! \brief   dump filter parameter
    //!
    //! \param   fpLog
    //!          [in] file pointer pointed to dumpped file
    //! \param   buffer
    //!          [in] pointed to filter parameter
    //!
    static void VpDumpFilterParameterBuffer(
        FILE                            *fpLog,
        VAProcFilterParameterBuffer     *buffer);

    //! \brief   dump filters parameter of surface parameters
    //!
    //! \param   [in] fpLog
    //!          file pointer pointed to dumpped file
    //! \param   [in] pipelineParameterBuffer
    //!          VAProcPipelineParameterBuffer
    //! \param   [in] srcFormat
    //!          MOS_FORMAT
    //!
    static void VpDumpProcFiltersParameterBufferSurface(
        FILE                          *fpLog,
        VAProcPipelineParameterBuffer *pipelineParameterBuffer,
        MOS_FORMAT                    srcFormat);

    //! \brief   dump filters parameter
    //!
    //! \param   [in] fpLog
    //!          file pointer pointed to dumpped file
    //! \param   [in] vaDrvCtx
    //!          driver context
    //! \param   [in] filters
    //!          pointed to filters
    //! \param   [in] filtersNum
    //!          number of filters
    //!
    static void VpDumpProcFiltersParameterBuffer(
        FILE                *fpLog,
        VADriverContextP    vaDrvCtx,
        VABufferID          *filters,
        unsigned int        filtersNum);

    //! \brief   dump filters parameter of Forward References
    //!
    //! \param   [in] fpLog
    //!          file pointer pointed to dumpped file
    //! \param   [in] forwardReferences
    //!          pointed to forward References
    //! \param   [in] forwardReferencesNum
    //!          number of forward References
    //!
    static void VpDumpProcFiltersParameterBufferFWDReference(
        FILE        *fpLog,
        VASurfaceID *forwardReferences,
        uint32_t    forwardReferencesNum);

    //! \brief   dump filters parameter of Backward References
    //!
    //! \param   [in] fpLog
    //!          file pointer pointed to dumpped file
    //! \param   [in] backwardReferences
    //!          pointed to backward References
    //! \param   [in] filtersNum
    //!          number of backward References
    //!
    static void VpDumpProcFiltersParameterBufferBKWReference(
        FILE        *fpLog,
        VASurfaceID *backwardReferences,
        uint32_t    backwardReferencesNum);

    //!
    //! \brief   dump procpipeline parameters
    //!
    //! \param   [in] vaDrvCtx
    //!          driver context
    //! \param   [in] vpCtx
    //!          vp context
    //! \return  VAStatus
    //!          return VA_STATUS_SUCCESS, if params is dumped to file.
    //!
    static VAStatus VpDumpProcPipelineParams(
        VADriverContextP        vaDrvCtx,
        PDDI_VP_CONTEXT         vpCtx);

#endif //#if (_DEBUG || _RELEASE_INTERNAL)

MEDIA_CLASS_DEFINE_END(DdiVpTools)
};

#endif //__DDI_VP_TOOLS_H__