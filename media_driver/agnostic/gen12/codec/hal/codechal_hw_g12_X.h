/*
* Copyright (c) 2017-2021, Intel Corporation
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
//! \brief     This modules implements HW interface layer to be used on Gen12 platforms on all operating systems/DDIs, across CODECHAL components.
//!
#ifndef __CODECHAL_HW_G12_X_H__
#define __CODECHAL_HW_G12_X_H__

#include "codechal_hw.h"
#include "mhw_mi_hwcmd_g12_X.h"
#include "mhw_render_hwcmd_g12_X.h"
#include "mhw_vdbox_avp_interface.h"
#include "media_sfc_interface_legacy.h"

#include "media_interfaces_mhw_next.h"

//!
//! \enum MediaStatesAv1FilmGrain
//!
enum
{
    CODECHAl_MEDIA_STATE_AV1_FILM_GRAIN_GRV = CODECHAL_MEDIA_STATE_SW_SCOREBOARD_INIT + 1,
    CODECHAl_MEDIA_STATE_AV1_FILM_GRAIN_RP1,
    CODECHAl_MEDIA_STATE_AV1_FILM_GRAIN_RP2,
    CODECHAl_MEDIA_STATE_AV1_FILM_GRAIN_AN,
};  //Please merge to CODECHAL_MEDIA_STATE_TYPE

#define CODECHAL_NUM_MEDIA_STATES_G12 (CODECHAL_NUM_MEDIA_STATES + 4)

//!  Codechal hw interface Gen12
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for Gen12 platforms
*/
class CodechalHwInterfaceG12 : public CodechalHwInterface
{
protected:
    static const CODECHAL_SSEU_SETTING m_defaultSsEuLutG12[CODECHAL_NUM_MEDIA_STATES_G12];

public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceG12(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfaces     *mhwInterfaces,
        bool              disableScalability = false);

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
    virtual ~CodechalHwInterfaceG12();

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
    virtual MOS_STATUS SetCacheabilitySettings(
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
    virtual MOS_STATUS GetAvpStateCommandSize(
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
    virtual MOS_STATUS GetAvpPrimitiveCommandSize(
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
    virtual MOS_STATUS SetRowstoreCachingOffsets(
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
    //! \brief    Read AVP status for status report
    //! \param    vdboxIndex
    //!           [in] the vdbox index
    //! \param    params
    //!           [in] the parameters for AVP status read
    //! \param    cmdBuffer
    //!           [in, out] the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadAvpStatus(
        MHW_VDBOX_NODE_IND           vdboxIndex,
        const EncodeStatusReadParams &params,
        PMOS_COMMAND_BUFFER          cmdBuffer);

    //!
    //! \brief    Read AVP specific image status for status report
    //! \param    vdboxIndex
    //!           [in] the vdbox index
    //! \param    params
    //!           [in] the parameters for AVP IMG status read
    //! \param    cmdBuffer
    //!           [in, out] the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadImageStatusForAvp(
        MHW_VDBOX_NODE_IND           vdboxIndex,
        const EncodeStatusReadParams &params,
        PMOS_COMMAND_BUFFER          cmdBuffer);


    //!
    //! \brief    Get film grain kernel info
    //! \details  Get kernel base and size
    //!
    //! \param    [out] kernelBase
    //!           base addr of film grain kernels
    //!
    //! \param    [out] kernelSize
    //!           size of film grain kernels
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetFilmGrainKernelInfo(
                uint8_t*    &kernelBase,
                uint32_t    &kernelSize) override;

private:
    //!
    //! \brief    Called by constructor
    //!
    void PrepareCmdSize(CODECHAL_FUNCTION codecFunction);

    //!
    //! \brief    Called by constructor
    //!
    void InternalInit(CODECHAL_FUNCTION codecFunction);
};
#endif // __CODECHAL_HW_G12_X_H__
