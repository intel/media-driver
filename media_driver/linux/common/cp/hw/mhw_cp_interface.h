/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     mhw_cp_interface.h
//! \brief    MHW interface for content protection 
//! \details  Impelements the functionalities across all platforms for content protection
//!

#ifndef __MHW_CP_INTERFACE_H__
#define __MHW_CP_INTERFACE_H__

#include "mhw_mi.h"
#include "mos_os.h"
#include "mos_util_debug.h"
#include "codec_def_common.h"
#include "cplib.h"

class MhwMiInterface;

typedef int32_t CP_MODE;
#define CP_TYPE_NONE 0

enum _CP_SECURITY_TYPE: int32_t;

typedef enum _CP_SECURITY_TYPE CP_SECURITY_TYPE;

typedef struct _MHW_CP_SLICE_INFO_PARAMS
{
    PMOS_RESOURCE presDataBuffer        = nullptr;
    uint32_t      dwDataLength[2]       = {0};       // 1 is for DECE mode, 0 is for others
    uint32_t      dwDataStartOffset[2]  = {0};       // 1 is for DECE mode, 0 is for others
    uint32_t      dwSliceIndex          = 0;
    bool          bLastPass             = false;
    uint32_t      dwTotalBytesConsumed  = 0;
} MHW_CP_SLICE_INFO_PARAMS, *PMHW_CP_SLICE_INFO_PARAMS;

typedef struct _MHW_CP_COPY_PARAMS
{
    PMOS_RESOURCE presSrc;
    PMOS_RESOURCE presDst;
    uint32_t      size;
    uint16_t      lengthOfTable;
    bool          isEncodeInUse;
} MHW_CP_COPY_PARAMS, *PMHW_CP_COPY_PARAMS;

class MhwCpInterface
{
public:
    MhwCpInterface(){}
    virtual ~MhwCpInterface(){}

    //!
    /// \brief       Add necessary commands and the beginning of the command buffer.
    /// \details     Based on Gen info provided in CpFactory() and the HW unit
    ///              info provided in PMOS_INTERFACE, the function forward the
    ///              request to the right CpHwUnit object.
    ///
    /// \param       osInterface  [in] to obtain HW unit being used info
    /// \param       cmdBuffer    [in] the command buffer being built
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    //!
    virtual MOS_STATUS AddProlog(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief       Check support for HW Counter Auto Increment
    /// \details     From KBL+ HW Counter increment is valid for wireless cast
    ///
    /// \param       pOsInterface  [in] to obtain HW unit being used info
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    ///////////////////////////////////////////////////////////////////////////
    virtual bool IsHWCounterAutoIncrementEnforced(
        PMOS_INTERFACE osInterface);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief       Add necessary commands and the end of the command buffer.
    /// \details     Based on Gen info provided in CpFactory() and the HW unit
    ///              info provided in PMOS_INTERFACE, the function forward the
    ///              request to the right CpHwUnit object.
    ///
    /// \param       osInterface  [in] to obtain HW unit being used info
    /// \param       cmdBuffer    [in] the command buffer being built
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    ///////////////////////////////////////////////////////////////////////////
    virtual MOS_STATUS AddEpilog(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer);
        
    ///////////////////////////////////////////////////////////////////////////
    /// \brief       Add check to exit if expected secure mode is not active
    /// \details     Add check to exit if expected secure mode is not active
    ///
    /// \param       osInterface  [in] to obtain HW unit being used info
    /// \param       cmdBuffer    [in] the command buffer being built
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    ///////////////////////////////////////////////////////////////////////////
    virtual MOS_STATUS AddCheckForEarlyExit(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer);

    virtual MOS_STATUS UpdateStatusReportNum(
        uint32_t            cencBufIndex,
        uint32_t            statusReportNum,
        uint8_t*            lockAddress,
        PMOS_RESOURCE       resource,
        PMOS_COMMAND_BUFFER cmdBuffer);

    virtual MOS_STATUS CheckStatusReportNum(
        void*                       mfxRegisters,
        uint32_t                    cencBufIndex,
        PMOS_RESOURCE               resource,
        PMOS_COMMAND_BUFFER         cmdBuffer);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief       Set encypted Copy Related Params For Scalable Encode
    /// \details
    /// \param       cmdBuffer      [in] the command buffer to add the cmd
    /// \param       params         [in] Params structure used to populate the HW command
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    ///////////////////////////////////////////////////////////////////////////
    virtual MOS_STATUS SetCpCopy(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_CP_COPY_PARAMS params);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief       Set Inline Status Read For Encode.
    /// \details
    /// \param       osInterface    [in] pointer to OS interface
    /// \param       cmdBuffer      [in] the command buffer to add the cmd
    /// \param       resource       [in] destinate resource to get result
    /// \param       currentIndex   [in] current index of encode 
    /// \param       writeOffset    [in] offset for dummy write
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    ///////////////////////////////////////////////////////////////////////////
    virtual MOS_STATUS SetMfxInlineStatusRead(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_RESOURCE       resource,
        uint16_t            currentIndex,
        uint32_t            writeOffset);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief       Set the protection bits of MI_FLUSH_DW based on secure status
    /// \details     The function post-process a MI_FLUSH_DW instance to toggle
    ///              its protected flush bit (bit 22) and AppType bit (bit 7
    ///              applicable only for gen8- ) based on secure mode
    ///              status and apptype(display/transcode)
    ///
    /// \param       osInterface  [in] to obtain secure status
    /// \param       cmd          [in] pointer to the MI_FLUSH_DW instance
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    /// \note        Here it's a pure virtual function only, to be implemented
    ///              in gen specific files
    ///////////////////////////////////////////////////////////////////////////
    virtual MOS_STATUS SetProtectionSettingsForMiFlushDw(
        PMOS_INTERFACE osInterface,
        void           *cmd);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief       Set the protection bits of MFX_PIPE_MODE_SELECT based on secure status
    /// \details     The function post-process a MFX_PIPE_MODE_SELECT instance to set
    ///              encryption control fields based on secure encryption parameter
    ///
    /// \param       data          [in] pointer to the encryption control instance
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    ///////////////////////////////////////////////////////////////////////////
    virtual MOS_STATUS SetProtectionSettingsForMfxWait(
        PMOS_INTERFACE osInterface,
        void           *cmd);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief       Set the protection bits of HCP_PIPE_MODE_SELECT based on secure status
    /// \details     The function post-process a HCP_PIPE_MODE_SELECT instance to set
    ///              encryption control fields based on secure encryption parameter
    ///
    /// \param       data           [in] pointer to the encryption control instance
    /// \param       scalableEncode [in] true if two pass Scalable encode workload
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    ///////////////////////////////////////////////////////////////////////////
    virtual MOS_STATUS SetProtectionSettingsForMfxPipeModeSelect(uint32_t *data);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief       Set the protection bits of HUC_PIPE_MODE_SELECT based on secure status
    /// \details     The function is to set encryption control fields based on secure status
    ///
    /// \param       data          [in] pointer to the encryption control instance
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    ///////////////////////////////////////////////////////////////////////////
    virtual MOS_STATUS SetProtectionSettingsForHcpPipeModeSelect(
        uint32_t *data, 
        bool scalableEncode = false);

    virtual MOS_STATUS SetProtectionSettingsForHucPipeModeSelect(uint32_t *data);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief       Set the protection setting for avc mfx command based on secure status
    /// \details     The function sets paramater based on secure status then sets
    ///              mfx state accordingly
    ///
    /// \param       isDecodeInUse       [in] indicate if decode in use
    /// \param       cmdBuffer         [in] the command buffer to add the cmd
    /// \param       presDataBuffer     [in] pointer to PMOS_RESOURCE
    /// \param       sliceInfoParam    [in] pointer to MHW_CP_SLICE_INFO_PARAMS
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    ///////////////////////////////////////////////////////////////////////////
    virtual MOS_STATUS SetMfxProtectionState(
        bool                        isDecodeInUse,
        PMOS_COMMAND_BUFFER         cmdBuffer,
        PMHW_BATCH_BUFFER           batchBuffer,
        PMHW_CP_SLICE_INFO_PARAMS   sliceInfoParam);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief       Register the MHW_MI_INTERFACE instance
    /// \details     Save the pointer to the instance for reusing send MI_FLUSH_DW
    ///              function in MHW_MI_INTERFACE.  this function is expected to be
    ///              used only by Mhw_CommonMi_InitInterface(), because by the time
    ///              MHW_MI_INTERFACE is being init, there is already an instance of
    ///              MHW_secure.  We want to reuse the same instance and MHW_COMMON_MI
    ///              init function can help register MHW_MI_INTERFACE to the MHW_secure instance.
    ///              if a client doesn't need MHW_MI_INTERFACE, then this function is
    ///              not needed either.
    ///
    /// \param       miInterface [in] pointer to the MHW_MI_INTERFACE
    ///                                    just created in Mhw_RenderEngine_Create()
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    ///////////////////////////////////////////////////////////////////////////
    virtual MOS_STATUS RegisterMiInterface(
        MhwMiInterface* miInterface);
        
    ////////////////////////////////////////////////////////////////////////////
    /// \brief       get the state level command size and patch list size for CP
    /// \details     The function calculate the cp relate comamnd size in state
    ///              level
    ///
    /// \param       cmdSize        [in] command size
    /// \param       patchListSize  [in] patch list size
    ///////////////////////////////////////////////////////////////////////////
    virtual void GetCpStateLevelCmdSize(
        uint32_t& cmdSize, 
        uint32_t& patchListSize);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief       get the slice level command size and patch list size for CP
    /// \details     The function calculate the cp relate comamnd size in slcie
    ///              level
    ///
    /// \param       cmdSize        [in] command size
    /// \param       patchListSize  [in] patch list size
    ///////////////////////////////////////////////////////////////////////////
    virtual void GetCpSliceLevelCmdSize(
        uint32_t& cmdSize, 
        uint32_t& patchListSize);
        
    /////////////////////////////////////////////////////////////////////////////
    /// \brief       Register encryption params struct from user.
    /// \details     MHW_CP need to keep a address of encryption params from user
    ///              so that could update encryption params each frame.
    /////////////////////////////////////////////////////////////////////////////
    virtual void RegisterParams(void* params);
    
    /////////////////////////////////////////////////////////////////////////////
    /// \brief       Update internal encryption params struct.
    /// \details     Update internal encryption params struct from registered
    ///              params pointer from user.
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    /////////////////////////////////////////////////////////////////////////////
    virtual MOS_STATUS UpdateParams(bool isInput);

    virtual CP_MODE GetHostEncryptMode() const;

    virtual void SetCpSecurityType(
        CP_SECURITY_TYPE type = static_cast<CP_SECURITY_TYPE>(1));

    /////////////////////////////////////////////////////////////////////////////
    /// \brief       get the address of internal counter value
    /// \details 
    /// \param       ctr [in, out] pointer to counter
    ///
    /// \return      If success, return MOS_STATUS_SUCCESS
    ///              If fail, return other MOS errors
    /////////////////////////////////////////////////////////////////////////////
    virtual MOS_STATUS GetCounterValue(uint32_t* ctr){return MOS_STATUS_SUCCESS;}

};

//!
//! \brief    Create MhwCpInterface Object according CPLIB loading status
//!           Must use Delete_MhwCpInterface to delete created Object to avoid ULT Memory Leak errors
//!
//! \return   Return CP Wrapper Object if CPLIB not loaded
//!
MhwCpInterface* Create_MhwCpInterface(PMOS_INTERFACE osInterface);

//!
//! \brief    Delete the MhwCpInterface Object according CPLIB loading status
//!
//! \param    [in] *pMhwCpInterface 
//!           MhwCpInterface
//!
void Delete_MhwCpInterface(MhwCpInterface* pMhwCpInterface);
#endif
