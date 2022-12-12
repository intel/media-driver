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
//! \file     mhw_vdbox_avp_g12_X.h
//! \brief    Defines functions for constructing Vdbox AVP commands on Gen12-based platforms
//!

#ifndef __MHW_VDBOX_AVP_G12_X_H__
#define __MHW_VDBOX_AVP_G12_X_H__

#include "mhw_vdbox_avp_generic.h"
#include "mhw_vdbox_avp_hwcmd_g12_X.h"
#include "mhw_vdbox_g12_X.h"

//!  MHW Vdbox Avp interface for Gen12
/*!
This class defines the Avp command construction functions for Gen12 platform
*/
class MhwVdboxAvpInterfaceG12 : public MhwVdboxAvpInterfaceGeneric<mhw_vdbox_avp_g12_X>
{
protected:
    #define PATCH_LIST_COMMAND(x)  (x##_NUMBER_OF_ADDRESSES)
    bool m_scalabilitySupported = false; //!< Indicate if scalability supported
    bool m_disableLstCmd        = false; //!< Indicate if lst cmd is used
    enum CommandsNumberOfAddresses
    {
        MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES              =  1, //  2 DW for  1 address field
        MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES                  =  1, //  2 DW for  1 address field
        MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES                        =  1, //  2 DW for  1 address field
        MI_CONDITIONAL_BATCH_BUFFER_END_CMD_NUMBER_OF_ADDRESSES    =  1, //  2 DW for  1 address field
        MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES              =  1, //  2 DW for  1 address field
        MI_COPY_MEM_MEM_CMD_NUMBER_OF_ADDRESSES                    =  4, //  4 DW for  2 address fields
        MI_SEMAPHORE_WAIT_CMD_NUMBER_OF_ADDRESSES                  =  1, //  2 DW for  1 address fields
        MI_ATOMIC_CMD_NUMBER_OF_ADDRESSES                          =  1, //  2 DW for  1 address field

        MFX_WAIT_CMD_NUMBER_OF_ADDRESSES                           =  0, //  0 DW for    address fields

        AVP_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES               =  0,  //  0 DW for    address fields
        AVP_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES                  =  0,  //  0 DW for    address fields
        AVP_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES            =  56, //           56 address fields
        AVP_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES        =  2,  //            2 address fields
        AVP_PIC_STATE_CMD_NUMBER_OF_ADDRESSES                      =  0,  //  0 DW for    address fields
        AVP_REF_IDX_STATE_CMD_NUMBER_OF_ADDRESSES                  =  0,  //  0 DW for    address fields
        AVP_SEGMENT_STATE_CMD_NUMBER_OF_ADDRESSES                  =  0,  //  0 DW for    address fields
        AVP_TILE_CODING_CMD_NUMBER_OF_ADDRESSES                    =  0,  //  0 DW for    address fields
        AVP_TILE_CODING_CMD_LST_NUMBER_OF_ADDRESSES                =  0,  //  0 DW for    address fields
        AVP_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES                     =  0,  //  0 DW for    address fields
        AVP_INLOOP_FILTER_STATE_CMD_NUMBER_OF_ADDRESSES            =  0,  //  0 DW for    address fields
        AVP_INTER_PRED_STATE_CMD_NUMBER_OF_ADDRESSES               =  0,  //  0 DW for    address fields
        AVP_PAK_INSERT_OBJECT_CMD_NUMBER_OF_ADDRESSES              =  0,   //  0 DW for    address fields
        VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES          =  12,  // 12 DW for 12 address fields
        VD_PIPELINE_FLUSH_CMD_NUMBER_OF_ADDRESSES                  =  0,  //  0 DW for  0 address fields
    };

public:
    //!
    //! \brief  Constructor
    //!
    MhwVdboxAvpInterfaceG12(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse)
        : MhwVdboxAvpInterfaceGeneric(osInterface, miInterface, cpInterface, decodeInUse)
    {
        MHW_FUNCTION_ENTER;

        InitRowstoreUserFeatureSettings();

        MOS_USER_FEATURE_VALUE_DATA userFeatureData;

        memset(&userFeatureData, 0, sizeof(userFeatureData));
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    #if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_AVP_SCALABILITY_DECODE_ID,
            &userFeatureData,
            this->m_osInterface->pOsContext);
    #endif // _DEBUG || _RELEASE_INTERNAL
        m_scalabilitySupported = userFeatureData.i32Data ? true : false;

        memset(&userFeatureData, 0, sizeof(userFeatureData));
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AV1_DECODE_ON_SIMULATION_ID,
            &userFeatureData,
            this->m_osInterface->pOsContext);
        m_disableLstCmd = userFeatureData.i32Data ? true : false;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxAvpInterfaceG12();

    void InitRowstoreUserFeatureSettings();

        //!
    //! \brief    Judge if scalability is supported
    //!
    //! \return   bool
    //!           true if supported, else false
    //!
    inline bool IsScalabilitySupported()
    {
        return m_scalabilitySupported;
    }

    MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams);

    //!
    //! \brief    Calculates maximum size for AVP state level commands
    //! \details  Client facing function to calculate maximum size for AVP state level commands
    //! \param    [out] commandsSize
    //!            The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetAvpStateCommandSize(
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

    //!
    //! \brief    Calculates maximum size for AVP primitive level commands
    //! \details  Client facing function to calculate maximum size for AVP primitive level commands
    //! \param    [out] commandsSize
    //!            The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetAvpPrimitiveCommandSize(
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize);

    //!
    //! \brief    Get AV1 Buffer size
    //!
    //! \param    [in] bufferType
    //!           Buffer type to get size
    //! \param    [in] avpBufSizeParam
    //!           buffer size params
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetAv1BufferSize(
        MhwVdboxAvpInternalBufferType       bufferType,
        MhwVdboxAvpBufferSizeParams         *avpBufSizeParam);

    //!
    //! \brief    check if AV1 Buffer reallocation is needed
    //! \details  function to check if AV1 Buffer reallocation is needed
    //!
    //! \param    [in] bufferType
    //!           Buffer type to query
    //! \param    [in] reallocParam
    //!           reallocate params
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS IsAv1BufferReallocNeeded(
        MhwVdboxAvpInternalBufferType       bufferType,
        MhwVdboxAvpBufferReallocParams      *reallocParam);

    //!
    //! \brief    Adds AVP pipe mode select command
    //! \details  function to add AVP surface state command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAvpPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params);

    //!
    //! \brief    Adds AVP surface state command for decoder
    //! \details  function to add AVP surface state command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAvpDecodeSurfaceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS        params);

    //!
    //! \brief    Adds AVP Pipe Buf Addr command
    //! \details  function to add AVP Pipe Buf Addr command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        MhwVdboxAvpPipeBufAddrParams     *params);

    //!
    //! \brief    Adds AVP Ind Obj Base Address command
    //! \details  function to add AVP Ind Obj Base Address command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddAvpIndObjBaseAddrCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  params);

    //!
    //! \brief    Adds AVP Pic State command for decoder
    //! \details  function to add AVP Segment State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAvpDecodePicStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        MhwVdboxAvpPicStateParams        *params);

    //!
    //! \brief    Adds AVP Segment State command
    //! \details  function to add AVP Segment State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAvpSegmentStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        MhwVdboxAvpSegmentStateParams    *params);

    //!
    //! \brief    Adds AVP tile coding command in command buffer
    //! \details  function to add AVP tile coding command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAvpTileCodingCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        MhwVdboxAvpTileCodingParams     *params);

    //!
    //! \brief    Adds AVP tile coding command for decoder
    //! \details  function to add AVP tile coding command in command buffer or Batch buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAvpDecodeTileCodingCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        MhwVdboxAvpTileCodingParams     *params);

    //!
    //! \brief    Adds AVP tile coding command for Large Scale Tile decoding
    //! \details  function to add AVP tile coding command in command buffer or Batch buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAvpDecodeTileCodingCmdLst(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        MhwVdboxAvpTileCodingParams     *params);

    //!
    //! \brief    Adds AVP BSD Object command in command buffer
    //! \details  function to add AVP BSD Object command in command buffer or Batch buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAvpBsdObjectCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        MhwVdboxAvpBsdParams            *params);

    //!
    //! \brief    Adds AVP Inloop Filter State command
    //! \details  function to add AVP Inloop Filter State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAvpInloopFilterStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        MhwVdboxAvpPicStateParams       *params);


    //!
    //! \brief    Adds AVP Inter Prediction State command
    //! \details  function to add AVP Inter Prediction State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAvpInterPredStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        MhwVdboxAvpPicStateParams            *params);
};

#endif
