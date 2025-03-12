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
//! \file      codec_hw_xe3_lpm_base.h
//! \brief     This modules implements HW interface layer to be used on Xe3_LPM platforms on all operating systems/DDIs, across CODECHAL components.
//!
#ifndef __CODEC_HW_XE3_LPM_BASE_H__
#define __CODEC_HW_XE3_LPM_BASE_H__

#include "codec_hw_next.h"
#include "mhw_vdbox_vvcp_itf.h"

#define CODECHAL_NUM_MEDIA_STATES_XE3_LPM (CODECHAL_NUM_MEDIA_STATES + 4)

//!  Codechal hw interface Xe3_LPM
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for M15 platforms
*/
class CodechalHwInterfaceXe3_Lpm_Base : public CodechalHwInterfaceNext
{
protected:
    static const CODECHAL_SSEU_SETTING m_defaultSsEuLutM16[CODECHAL_NUM_MEDIA_STATES_XE3_LPM];

public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceXe3_Lpm_Base(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfacesNext *mhwInterfacesNext,
        bool              disableScalability = false);

    //!
    //! \brief    Copy constructor
    //!
    CodechalHwInterfaceXe3_Lpm_Base(const CodechalHwInterfaceXe3_Lpm_Base&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalHwInterfaceXe3_Lpm_Base& operator=(const CodechalHwInterfaceXe3_Lpm_Base&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceXe3_Lpm_Base();

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
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

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
        uint32_t                        *patchListSize);

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
    //! \brief    Calculates the maximum size for VVCP picture level commands
    //! \details  Client facing function to calculate the maximum size for VVCP picture level commands
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
    MOS_STATUS GetVvcpStateCommandSize(
        uint32_t                        mode,
        uint32_t                       *commandsSize,
        uint32_t                       *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

    //!
    //! \brief    Calculates the size for VVCP slice level commands
    //! \details  Client facing function to calculate the maximum size for VVCP picture level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] sliceCommandsSize
    //!           The maximum command buffer size for slice
    //! \param    [out] slicePatchListSize
    //!           The maximum command patch list size for slice
    //! \param    [out] tileCommandsSize
    //!           The maximum command buffer size for tile
    //! \param    [out] tilePatchListSize
    //!           The maximum command patch list size for tile
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetVvcpPrimitiveCommandSize(
        uint32_t  mode,
        uint32_t *sliceCommandsSize,
        uint32_t *slicePatchListSize,
        uint32_t *tileCommandsSize,
        uint32_t *tilePatchListSize);

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


MEDIA_CLASS_DEFINE_END(CodechalHwInterfaceXe3_Lpm_Base)
};
#endif // __CODEC_HW_XE3_LPM_BASE_H__