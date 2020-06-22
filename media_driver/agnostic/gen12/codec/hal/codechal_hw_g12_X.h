/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file      codechal_hw_g12_X.h
//! \brief         This modules implements HW interface layer to be used on Gen12 platforms on all operating systems/DDIs, across CODECHAL components.
//!
#ifndef __CODECHAL_HW_G12_X_H__
#define __CODECHAL_HW_G12_X_H__

#include "codechal_hw.h"
#include "mhw_mi_hwcmd_g12_X.h"
#include "mhw_render_hwcmd_g12_X.h"

//!  Codechal hw interface Gen12
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for Gen12 platforms
*/
class CodechalHwInterfaceG12 : public CodechalHwInterface
{
protected:
    static const CODECHAL_SSEU_SETTING m_defaultSsEuLutG12[CODECHAL_NUM_MEDIA_STATES];

public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceG12(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfaces     *mhwInterfaces);

    //!
    //! \brief    Copy constructor
    //!
    CodechalHwInterfaceG12(const CodechalHwInterfaceG12&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalHwInterfaceG12& operator=(const CodechalHwInterfaceG12&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceG12(){};

    MOS_STATUS InitL3CacheSettings() override;

    MOS_STATUS GetStreamoutCommandSize(
        uint32_t                       *commandsSize,
        uint32_t                       *patchListSize) override;
    //!
    //! \brief    Set Cacheability Settings
    //! \details  Set Cacheability Settings in sub interfaces in codechal hw interface
    //!
    //! \param    [in] cacheabilitySettings
    //!           cacheability Settings to set into sub mhw intefaces in hw interface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCacheabilitySettings(
        MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) override;

    //!
    //! \brief    Send conditional batch buffer end cmd
    //! \details  Send conditional batch buffer end cmd
    //!
    //! \param    [in] resource
    //!           Reource used in conditional batch buffer end cmd
    //! \param    [in] offset
    //!           Reource offset used in mi atomic dword cmd
    //! \param    [in] compData
    //!           Compare data
    //! \param    [in] disableCompMask
    //!           Indicate disabling compare mask
    //! \param    [in] enableEndCurrentBatchBuffLevel
    //!           End Current Batch Buffer Level
    //! \param    [in] compareOperation
    //!           Compare operation
    //! \param    [in,out] cmdBuffer
    //!           command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendCondBbEndCmd(
        PMOS_RESOURCE              resource,
        uint32_t                   offset,
        uint32_t                   compData,
        bool                       disableCompMask,
        bool                       enableEndCurrentBatchBuffLevel,
        uint32_t                   compareOperation,
        PMOS_COMMAND_BUFFER        cmdBuffer);
};

#endif // __CODECHAL_HW_G12_X_H__
