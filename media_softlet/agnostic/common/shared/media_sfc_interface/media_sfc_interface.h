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
//! \file     media_sfc_interface.h
//! \brief    Common interface and structure used in sfc interface
//! \details  Common interface and structure used in sfc interface which are platform independent
//!
#ifndef __MEDIA_SFC_INTERFACE_H__
#define __MEDIA_SFC_INTERFACE_H__

#include <stdint.h>
#include "mos_os_specific.h"
#include "media_defs.h"
#include "media_common_defs.h"
#include "mos_defs.h"
#include "mos_defs_specific.h"
#include "mos_os.h"
#include "mos_resource_defs.h"

class MediaSfcRender;
class MediaMemComp;

struct VEBOX_SFC_PARAMS
{
    // Input
    struct
    {
        PMOS_SURFACE                surface;
        MEDIA_CSPACE                colorSpace;         //!< Color Space
        uint32_t                    chromaSiting;       //!< ChromaSiting
        RECT                        rcSrc;              //!< rectangle on input surface before scaling.
        MEDIA_ROTATION              rotation;           //!< rotation setting
    } input;

    // Output
    struct
    {
        PMOS_SURFACE                surface;
        MEDIA_CSPACE                colorSpace;         //!< Color Space
        uint32_t                    chromaSiting;       //!< ChromaSiting
        RECT                        rcDst;              //!< rectangle on output surface after scaling.
    } output;
};

struct SCALABILITY_PARAMS
{
    int32_t             numPipe     = 1;                //!< Number of pipes for scalability
    int32_t             curPipe     = 0;                //!< Current pipe index
    // Scalability parameters for HCP-to-SFC mode
    uint32_t            engineMode  = 0;                //!< 0 - single, 1 - left most column, 2 - right most column, 3 - middle column
    uint32_t            tileType    = 0;                //!< Real tile = 0, virtual tile = 1
    uint32_t            srcStartX   = 0;                //!< Source surface column horizontal start position in pixel
    uint32_t            srcEndX     = 0;                //!< Source surface column horizontal end position in pixel
    uint32_t            dstStartX   = 0;                //!< Output surface column horizontal start position in pixel
    uint32_t            dstEndX     = 0;                //!< Output surface column horizontal end position in pixel
};

struct FIELD_PARAMS
{
    bool isFieldToInterleaved   = false;                //!< true if input being field frame and output being interlaved frame.
    bool isBottomField          = false;                //!< true if input frame being bottom field. Valid only when true == isFieldToInterleaved.
    bool isBottomFirst          = false;                //!< true if output frame being interlaved frame with bottom first.
                                                        //!< Valid only when true == isFieldToInterleaved.
};

struct VIDEO_PARAMS
{
    CODECHAL_STANDARD   codecStandard;
    union
    {
        struct
        {
            CodecDecodeJpegChromaType       jpegChromaType;
        } jpeg;
        struct
        {
            bool                            deblockingEnabled;
        } avc, vp8;
        struct
        {
            uint32_t                        lcuSize;
        } hevc;
        struct
        {
            uint32_t                        lcuSize;
            bool                            lossless;
            bool                            superResInuse;
            bool                            intraBC;
            uint32_t                        tileCols;
            uint32_t                        tileRows;
        } av1;
    };

    FIELD_PARAMS fieldParams = {};

    // scalability parameters
    SCALABILITY_PARAMS scalabilityParams = {};
};

struct VDBOX_SFC_PARAMS
{
    // Input
    struct
    {
        uint32_t                    width;              //!< Effective width of SFC input, which may
                                                        //!< be smaller than frame width.
        uint32_t                    height;             //!< Effective height of SFC input, which may
                                                        //!< be smaller than frame height.
        uint32_t                    effectiveWidth;     //!< exclude right padding area on input surface,
                                                        //!< which may be smaller than Effective width.
        uint32_t                    effectiveHeight;    //!< exclude bottom padding area on input surface,
                                                        //!< which may be smaller than Effective height.
        MOS_FORMAT                  format;             //!< Format of SFC input
        MEDIA_CSPACE                colorSpace;         //!< Color Space
        uint32_t                    chromaSiting;       //!< ChromaSiting
        bool                        mirrorEnabled;      //!< Is mirror needed.
    } input;

    // Output
    struct
    {
        PMOS_SURFACE                surface;
        PMOS_BUFFER                 histogramBuf;       //!< Histogram output buffer
        MEDIA_CSPACE                colorSpace;         //!< Color Space
        uint32_t                    chromaSiting;       //!< ChromaSiting
        RECT                        rcDst;              //!< rectangle on output surface after scaling.
    } output;

    VIDEO_PARAMS                    videoParams = {};        //!< standard related params.
    CODECHAL_SCALING_MODE           scalingMode = CODECHAL_SCALING_BILINEAR;
};

union MEDIA_SFC_INTERFACE_MODE
{
    struct
    {
        uint32_t veboxSfcEnabled : 1;
        uint32_t vdboxSfcEnabled : 1;
    };
    uint32_t value = 3;
};

class MediaSfcInterface
{
public:
    //!
    //! \brief    MediaSfcInterface constructor
    //! \details  Initialize the MediaSfcInterface members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //! \param    mmc
    //!           [in] Pointer to MediaMemComp.
    //!
    MediaSfcInterface(PMOS_INTERFACE osInterface, MediaMemComp *mmc = nullptr);

    virtual ~MediaSfcInterface();

    virtual void Destroy();

    //!
    //! \brief    Check whether the Parameter for VDBOX-SFC supported
    //! \details  Only valid when MEDIA_SFC_INTERFACE_MODE::vdboxSfcEnabled being 1.
    //! \param    param
    //!           [in] Pointer to VDBOX_SFC_PARAMS.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if supported, otherwise failed
    //!
    MOS_STATUS IsParameterSupported(VDBOX_SFC_PARAMS &param);

    //!
    //! \brief    Check whether the Parameter for VEBOX-SFC supported
    //! \details  Only valid when MEDIA_SFC_INTERFACE_MODE::veboxSfcEnabled being 1.
    //! \param    param
    //!           [in] Pointer to VEBOX_SFC_PARAMS.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if supported, otherwise failed
    //!
    MOS_STATUS IsParameterSupported(VEBOX_SFC_PARAMS &param);

    //!
    //! \brief    Render Vdbox-SFC States
    //! \details  Only valid when MEDIA_SFC_INTERFACE_MODE::vdboxSfcEnabled being 1.
    //! \param    cmdBuffer
    //!           [in/out] Command Buffer to be Filled.
    //! \param    param
    //!           [in] Pointer to VDBOX_SFC_PARAMS.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if supported, otherwise failed
    //!
    MOS_STATUS Render(MOS_COMMAND_BUFFER *cmdBuffer, VDBOX_SFC_PARAMS &param);

    //!
    //! \brief    Render Vebox-SFC States
    //! \details  Only valid when MEDIA_SFC_INTERFACE_MODE::veboxSfcEnabled being 1.
    //! \param    param
    //!           [in] Pointer to VEBOX_SFC_PARAMS.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if supported, otherwise failed
    //!
    MOS_STATUS Render(VEBOX_SFC_PARAMS &param);

    //!
    //! \brief    Sfc Command Size
    //! \details  Calculate Command size of SFC commands.
    //! \return   uint32_t
    //!           Return calculated size
    //!
    uint32_t GetSfcCommandSize();

    //!
    //! \brief    MediaSfcInterface initialize
    //! \details  Initialize the MediaSfcInterface.
    //! \param    mode
    //!           [in] 1: VEBOX-SFC only, 2: VDBOX-SFC only, 3: Both VEBOX-SFC and VDBOX-SFC.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize(MEDIA_SFC_INTERFACE_MODE mode);

    bool IsRenderInitialized();

protected:
    PMOS_INTERFACE m_osInterface    = nullptr;
    MediaSfcRender *m_sfcRender     = nullptr;
    MediaMemComp   *m_mmc           = nullptr;
MEDIA_CLASS_DEFINE_END(MediaSfcInterface)
};

#endif // __MEDIA_SFC_INTERFACE_H__
