/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file      codec_hw_xe_lpm_plus_base.h
//! \brief     This modules implements HW interface layer to be used on Xe_LPM_plus+ platforms on all operating systems/DDIs, across CODECHAL components.
//!
#ifndef __CODECHAL_HW_XE_LPM_PLUS_BASE_H__
#define __CODECHAL_HW_XE_LPM_PLUS_BASE_H__

#include "codec_hw_next.h"

#define CODECHAL_NUM_MEDIA_STATES_XE_LPM_PLUS_BASE (CODECHAL_NUM_MEDIA_STATES + 4)

//!  Codechal hw interface Xe_LPM_plus base
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for Gen12 platforms
*/
class CodechalHwInterfaceXe_Lpm_Plus_Base : public CodechalHwInterfaceNext
{
protected:
    static const CODECHAL_SSEU_SETTING m_defaultSsEuLutXeLpmPlus[CODECHAL_NUM_MEDIA_STATES_XE_LPM_PLUS_BASE];

public:
    CodechalHwInterfaceXe_Lpm_Plus_Base(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfacesNext *mhwInterfacesNext,
        bool              disableScalability = false);

    //!
    //! \brief    Copy constructor
    //!
    CodechalHwInterfaceXe_Lpm_Plus_Base(const CodechalHwInterfaceXe_Lpm_Plus_Base&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalHwInterfaceXe_Lpm_Plus_Base& operator=(const CodechalHwInterfaceXe_Lpm_Plus_Base&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceXe_Lpm_Plus_Base();

    //!
    //! \brief    Calculates the maximum size for AVP picture level commands
    //! \details  Client facing function to calculate the maximum size for AVP picture level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] params
    //!           Indicate the command size parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetAvpStateCommandSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) override;

    //!
    //! \brief    Calculates maximum size for AVP tile level commands
    //! \details  Client facing function to calculate maximum size for AVP tile level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!            The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetAvpPrimitiveCommandSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize) override;

    //!
    //! \brief    Set Rowstore Cache offsets for Gen12 specific interfaces
    //! \details  Set Rowstore Cache offsets in sub interfaces in codechal hw G12 interface
    //!
    //! \param    [in] rowstoreParams
    //!           parameters to set rowstore cache offsets
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetRowstoreCachingOffsets(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams) override;

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

    //!
    //! \brief    Initialize the codechal hw interface
    //! \details  Initialize the interface before using
    //!
    //! \param    [in] settings
    //!           Settings for initialization
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Initialize(
        CodechalSetting *settings) override;

        //!
    //! \brief    Calculates the maximum size for HCP picture level commands
    //! \details  Client facing function to calculate the maximum size for HCP picture level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] params
    //!           Indicate the command size parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetHcpStateCommandSize(
        uint32_t                        mode,
        uint32_t *                      commandsSize,
        uint32_t *                      patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

    //!
    //! \brief    Calculates maximum size for HCP slice/MB level commands
    //! \details  Client facing function to calculate maximum size for HCP slice/MB level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!            The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] modeSpecific
    //!           Indicate the long or short format
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetHcpPrimitiveCommandSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      modeSpecific) override;


MEDIA_CLASS_DEFINE_END(CodechalHwInterfaceXe_Lpm_Plus_Base)
};
#endif // __CODECHAL_HW_XE_LPM_PLUS_BASE_H__
