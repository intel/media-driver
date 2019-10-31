/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codechal_setting.h
//! \brief    Defines class CodechalSetting
//!
#ifndef __CODECHAL_SETTING_H__
#define __CODECHAL_SETTING_H__

#include "codec_def_common.h"
//!
//! \class  CodechalSetting 
//! \brief  Settings used to finalize the creation of the CodecHal device 
//!
class CodechalSetting
{
public:
    CODECHAL_FUNCTION       codecFunction = CODECHAL_FUNCTION_INVALID;                  //!< High level codec functionality requested.
    /*! \brief Width requested.
    *
    *   For encode this width must be the maximum width for the entire stream to be encoded, for decode dynamic allocation is supported and this width is the largest width recieved.
    */
    uint32_t                width = 0;
    /*! \brief Height requested.
    *
    *   For encode this height must be the maximum height for the entire stream to be encoded, for decode dynamic allocation is supported and this height is the largest height recieved.
    */
    uint32_t                height = 0;
    uint32_t                mode = 0;                           //!< Mode requested (high level combination between Standard and CodecFunction).
    uint32_t                standard = 0;                       //!< Codec standard requested.
    uint8_t                 lumaChromaDepth = 0;              //!< Applies currently to HEVC only, specifies bit depth as either 8 or 10 bits.
    uint8_t                 chromaFormat = 0;                 //!< Applies currently to HEVC/VP9 only, specifies chromaformat as 420/422/444.
    bool                    intelEntrypointInUse = false;          //!< Applies to decode only, application is using a Intel-specific entrypoint.
    bool                    shortFormatInUse = false;              //!< Applies to decode only, application is passing short format slice data.

    bool                    disableDecodeSyncLock = false;         //!< Flag to indicate if Decode O/P can be locked for sync.

    // Decode Downsampling
    bool                    downsamplingHinted = false;    //!< Applies to decode only, application may request field scaling.

    bool                    disableUltraHME = false;       //!< Applies currently to HEVC VDEnc only to disable UHME
    bool                    disableSuperHME = false;       //!< Applies currently to HEVC VDEnc only to disable SHME
    void                    *cpParams = nullptr;           //!< CP params
    bool                    isMfeEnabled = false;          //!< Flag to indicate if Mfe is enabled.

    // Decode SFC enabling
    bool                    sfcEnablingHinted = false;     //!< Applies to decode only, application may request field sfc.
    bool                    sfcInUseHinted = false;        //!< Applies to decode only, application may request sfc engine.
    bool                    enableCodecMmc = false;        //!< Applies to both of decode and encode, to indicate if codec MMC could be enabled by default

    //!
    //! \brief    Destructor 
    //!
    virtual ~CodechalSetting(){};

    //!
    //! \brief    Return the pointer to CP parameters 
    //!
    void *GetCpParams() { return cpParams; };

    //!
    //! \brief    Return the indicate if cenc advance is used or not 
    //!
    virtual bool CheckCencAdvance() {return false; };

    //!
    //! \brief    Return the decode enc type 
    //!
    virtual uint32_t DecodeEncType() {return 0; };

    //!
    //! \brief    Create CodechalSetting instance 
    //!
    static CodechalSetting *CreateCodechalSetting();
};

#endif
