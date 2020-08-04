/*
* Copyright (c) 2020, Intel Corporation
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

#include "mos_os_specific.h"
#include "codec_def_decode_jpeg.h"
#include "media_common_defs.h"

class MediaSfcRender;

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

struct VDBOX_SFC_PARAMS
{
    // Input
    struct
    {
        uint32_t                    width;
        uint32_t                    height;
        MOS_FORMAT                  format;
        MEDIA_CSPACE                colorSpace;         //!< Color Space
        uint32_t                    chromaSiting;       //!< ChromaSiting
        bool                        mirrorEnabled;      //!< Is mirror needed.
    } input;

    // Output
    struct
    {
        PMOS_SURFACE                surface;
        MEDIA_CSPACE                colorSpace;         //!< Color Space
        uint32_t                    chromaSiting;       //!< ChromaSiting
        RECT                        rcDst;              //!< rectangle on output surface after scaling.
    } output;

    CODECHAL_STANDARD               codecStandard;
    CodecDecodeJpegChromaType       jpegChromaType;
    uint32_t                        lcuSize;
    bool                            deblockingEnabled;
};

class MediaSfcInterface
{
public:
    //!
    //! \brief    MediaSfcInterface constructor
    //! \details  Initialize the MediaSfcInterface members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    MediaSfcInterface(PMOS_INTERFACE osInterface);

    virtual ~MediaSfcInterface();

    virtual void Destroy();

    MOS_STATUS Render(VEBOX_SFC_PARAMS &param);
    MOS_STATUS Render(MOS_COMMAND_BUFFER *cmdBuffer, VDBOX_SFC_PARAMS &param);
    //!
    //! \brief    BltState initialize
    //! \details  Initialize the BltState, create BLT context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize();

protected:
    PMOS_INTERFACE m_osInterface    = nullptr;
    MediaSfcRender *m_sfcRender     = nullptr;
};

#endif // __MEDIA_SFC_INTERFACE_H__
