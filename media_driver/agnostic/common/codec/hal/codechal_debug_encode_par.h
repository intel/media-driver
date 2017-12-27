/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_debug_encode_par.h
//! \brief    Defines the debug interface shared by all of CodecHal for encode
//!           PAR file dump.
//!

#ifndef __CODECHAL_DEBUG_ENCODE_PAR_H__
#define __CODECHAL_DEBUG_ENCODE_PAR_H__

#include "codechal.h"
#include "codechal_debug.h"
#include "codechal_encoder_base.h"

#if USE_CODECHAL_DEBUG_TOOL

struct EncodeCommonPar
{
    // DSParams
    uint32_t                    mbFlatnessThreshold;

    // HME Params
    uint8_t                     superCombineDist;
    uint8_t                     meMethod;
    bool                        superHME;
    bool                        ultraHME;
    bool                        streamInEnable;
    uint8_t                     streamInL0FromNewRef;
    uint8_t                     streamInL1FromNewRef;
};

class CodechalDebugEncodePar
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalDebugEncodePar(CodechalEncoderState *encoder);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalDebugEncodePar();

    //!
    //! \brief    Populate DS parameters
    //!
    //! \param    [in] *cmd
    //!           pointer to curbe data
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS PopulateDsParam(
        void *cmd) { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Populate HME parameters
    //!
    //! \param    [in] is16xMeEnabled
    //!           16x ME enabled flag
    //! \param    [in] is32xMeEnabled
    //!           32x ME enabled flag
    //! \param    [in] meMethod
    //!           ME method
    //! \param    [in] *cmd
    //!           pointer to curbe data
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS PopulateHmeParam(
        bool    is16xMeEnabled,
        bool    is32xMeEnabled,
        uint8_t meMethod,
        void    *curbe) { return MOS_STATUS_SUCCESS; }

    EncodeCommonPar             *commonPar = nullptr;                         //!< Pointer to common PAR;

    // Control flags
    bool                        isConstDumped = false;                        //!< Is const data dumped;

protected:
    //!
    //! \brief    Initialize PAR file
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS Initialize();

    //!
    //! \brief    Destroy PAR file
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS Destroy();

    CodechalEncoderState        *m_encoder = nullptr;                         //!< Pointer to ENCODER base class;
    MOS_INTERFACE               *m_osInterface = nullptr;                     //!< OS interface
    CodechalHwInterface         *m_hwInterface = nullptr;                     //!< HW interface
    CodechalDebugInterface      *m_debugInterface = nullptr;                  //!< Debug interface
};

#endif // USE_CODECHAL_DEBUG_TOOL

#endif // __CODECHAL_DEBUG_ENCODE_PAR_H__
