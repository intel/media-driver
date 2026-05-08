/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     mhw_vdbox_mfx_cmdpar_ext.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_VDBOX_MFX_CMDPAR_EXT_H__
#define __MHW_VDBOX_MFX_CMDPAR_EXT_H__

namespace mhw
{
namespace vdbox
{
namespace mfx
{

enum SURFACE_FORMAT_EXT
{
    SURFACE_FORMAT_P010             = 13,  //!< P010 for 420 10 bits
    SURFACE_FORMAT_Y216             = 14,  //!< Y216 for 422 10 bit (upto 16 bit)
    SURFACE_FORMAT_Y210V            = 14,  //!< Y210 Variant
};

#define mfxMpeg2PicStatePar0 OldbpquantEncoderThisFieldShouldBeSetToZero

#define MFX_AVC_IMG_STATE_EXT        \
    uint8_t minFrameWSize = 0;       \
    uint8_t loadSlicePointerFlag = 0;\
    uint8_t tqchromadisable = 1;     \
    uint8_t pakQpShift = 0;          \
    uint8_t nonfirstpassflag = 0;    \
    uint8_t minframewsizeunits = 0;  \
    uint8_t granularCrcPoison = 0; \
    uint8_t poisonNthGranularCrc = 0; \
    uint8_t granularCrcStreamoutEnable = 0; \
    uint8_t granularCrcStreaminEnable  = 0

#define MFX_AVC_SLICE_STATE_EXT                \
    uint8_t rateControlCounterEnable = 0;      \
    uint8_t rcTriggleMode = 0;                 \
    uint8_t rcStableTolerance = 0;             \
    uint8_t rcPanicEnable = 0;                 \
    uint8_t mbTypeDirectConversionDis = 0;     \
    uint8_t mbTypeSkipConversionDis = 0;       \
    uint8_t compressBitstreamOutputDisFlag = 0;\
    uint8_t streamID10 = 0;                    \
    uint8_t magnitudeQpMaxNegativeModifier = 0;\
    uint8_t magnitudeQpMaxPositiveModifier = 0;\
    uint8_t shrinkParamShrinkResistance = 0;   \
    uint8_t shrinkParamShrinkInit = 0;         \
    uint8_t growParamGrowResistance = 0;       \
    uint8_t correct6 = 0;                      \
    uint8_t correct5 = 0;                      \
    uint8_t correct4 = 0;                      \
    uint8_t correct3 = 0;                      \
    uint8_t correct2 = 0;                      \
    uint8_t correct1 = 0

#define MFX_SURFACE_STATE_EXT              \
    uint8_t variantSurfaceFormatEnable = 0;\

#define MFX_PIPE_BUF_ADDR_STATE_EXT        \
    PMOS_RESOURCE granularCRCStreaminBuffer; \
    PMOS_RESOURCE granularCRCStreamoutBuffer;


    }  // namespace mfx
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_MFX_CMDPAR_EXT_H__