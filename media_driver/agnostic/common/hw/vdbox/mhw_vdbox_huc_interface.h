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

//! \file     mhw_vdbox_huc_interface.h
//! \brief    MHW interface for constructing HUC commands for the Vdbox engine
//! \details  Defines the interfaces for constructing MHW Vdbox HUC commands across all platforms 
//!

#ifndef _MHW_VDBOX_HUC_INTERFACE_H_
#define _MHW_VDBOX_HUC_INTERFACE_H_

#include "mhw_vdbox.h"
#include "mhw_mi.h"

typedef struct _MHW_VDBOX_HUC_STREAM_OBJ_PARAMS
{
    uint32_t                    dwIndStreamInLength;
    uint32_t                    dwIndStreamInStartAddrOffset;
    bool                        bHucProcessing;
    uint32_t                    dwIndStreamOutStartAddrOffset;
    uint8_t                     bStreamOutEnable;
    uint8_t                     bStreamInEnable;
    uint8_t                     bEmulPreventionByteRemoval;
    uint8_t                     bStartCodeSearchEngine;
    uint8_t                     bLengthModeEnabled;
    uint8_t                     ucStartCodeByte2;
    uint8_t                     ucStartCodeByte1;
    uint8_t                     ucStartCodeByte0;
} MHW_VDBOX_HUC_STREAM_OBJ_PARAMS, *PMHW_VDBOX_HUC_STREAM_OBJ_PARAMS;

// structure for HuC IMEM_STATE, DMEM_STATE commands
typedef struct _MHW_VDBOX_HUC_IMEM_STATE_PARAMS
{
    uint32_t                    dwKernelWopcmOffset;                            // 32KB-aligned kernel offset in WOPCM
    uint32_t                    dwKernelDescriptor;                             // kernel descriptor
    PMOS_RESOURCE               presHucBinaryImageBuffer;
} MHW_VDBOX_HUC_IMEM_STATE_PARAMS, *PMHW_VDBOX_HUC_IMEM_STATE_PARAMS;

typedef struct _MHW_VDBOX_HUC_DMEM_STATE_PARAMS
{
    uint32_t                    dwDataLength;                                   // length in bytes of the HUC data. Must be in increments of 64B
    uint32_t                    dwDmemOffset;                                   // DMEM offset in the HuC Kernel. This is different for ViperOS vs GEMS.
    PMOS_RESOURCE               presHucDataSource;                              // resource for HuC data source
} MHW_VDBOX_HUC_DMEM_STATE_PARAMS, *PMHW_VDBOX_HUC_DMEM_STATE_PARAMS;

// structure for HuC VIRTUAL_ADDR commands
typedef struct _MHW_VDBOX_HUC_REGION_PARAMS
{
    PMOS_RESOURCE               presRegion;
    uint32_t                    dwOffset;
    bool                        isWritable;
} MHW_VDBOX_HUC_REGION_PARAMS, *PMHW_VDBOX_HUC_REGION_PARAMS;

// structure for HuC VIRTUAL_ADDR commands
typedef struct _MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS
{
    MHW_VDBOX_HUC_REGION_PARAMS  regionParams[16];                                 // region [0~15] for VIRTUAL_ADDR command
} MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS, *PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS;



//!  MHW Vdbox Huc interface
/*!
This class defines the interfaces for constructing Vdbox Huc commands across all platforms
*/
class MhwVdboxHucInterface
{
protected:

    PMOS_INTERFACE              m_osInterface = nullptr; //!< Pointer to OS interface
    MhwMiInterface             *m_MiInterface = nullptr; //!< Pointer to MI interface
    MhwCpInterface             *m_cpInterface = nullptr; //!< Pointer to CP interface
    MEDIA_WA_TABLE              *m_waTable = nullptr; //!< Pointer to WA table

    static const uint32_t  m_hucStatusHevcS2lFailureMask = 0x8000;  //!< HuC Status HEVC short to long failure mask
                                                                    //!< bit15: uKernal uOS Status, FW will write 0 if has critical error

    static const uint32_t  m_hucStatus2ImemLoadedMask = 0x40;     //!< HuC Status 2 IMEM loaded mask
                                                                  //!< bit 6: Valid IMEM Loaded
    static const uint32_t  m_hucErrorFlagsMask = 0xFFFE;          //!< HuC error 2 flags mask

    static const uint32_t  m_hucStatusReEncodeMask = 0x80000000;  //! HUC PAK Integration kernel reEncode mask.

    MmioRegistersHuc       m_mmioRegisters[MHW_VDBOX_NODE_MAX] = { };  //!< HuC mmio registers

    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC] = { }; //!< Cacheability settings

    //!
    //! \brief    Constructor
    //!
    MhwVdboxHucInterface(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface);

    //!
    //! \brief    Add a resource to the command buffer 
    //! \details  Internal function to add either a graphics address of a resource or
    //!           add the resource to the patch list for the requested buffer
    //!
    //! \param    [in] osInterface
    //!           OS interface
    //! \param    [in] cmdBuffer
    //!           Command buffer to which resource is added
    //! \param    [in] params
    //!           Parameters necessary to add the resource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS(*AddResourceToCmd) (
        PMOS_INTERFACE osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_RESOURCE_PARAMS params);

public:
    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxHucInterface() {}

    //!
    //! \brief    Get Huc Status Hevc S2l Failure Mask
    //!
    //! \return   [out] uint32_t
    //!           Mask got.
    //!
    inline uint32_t GetHucStatusHevcS2lFailureMask()
    {
        return m_hucStatusHevcS2lFailureMask;
    }

    //!
    //! \brief    Get Huc Status2 Imem Loaded Mask
    //!
    //! \return   [out] uint32_t
    //!           Mask got.
    //!
    inline uint32_t GetHucStatus2ImemLoadedMask()
    {
        return m_hucStatus2ImemLoadedMask;
    }

    //!
    //! \brief    Get Huc Error Flags Mask
    //!
    //! \return   [out] uint32_t
    //!           Mask got.
    //!
    inline uint32_t GetHucErrorFlagsMask()
    {
        return m_hucErrorFlagsMask;
    }

    //!
    //! \brief    Get Huc Status ReEncode Mask
    //!
    //! \return   [out] uint32_t
    //!           Mask got.
    //!
    inline uint32_t GetHucStatusReEncodeMask()
    {
        return m_hucStatusReEncodeMask;
    }

    //!
    //! \brief    Get mmio registers
    //!
    //! \param    [in] index
    //!           mmio registers index.
    //!
    //! \return   [out] MmioRegistersHuc*
    //!           mmio registers got.
    //!
    inline MmioRegistersHuc* GetMmioRegisters(MHW_VDBOX_NODE_IND index)
    {
        if (index < MHW_VDBOX_NODE_MAX)
        {
            return &m_mmioRegisters[index];
        }
        else
        {
            MHW_ASSERT("index is out of range!");
            return &m_mmioRegisters[MHW_VDBOX_NODE_1];
        }
    }

    //!
    //! \brief    Calculates the maximum size for HUC picture level commands
    //! \details  Client facing function to calculate the maximum size for HUC picture level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] params
    //!           PM HW Vdbox state command size parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHucStateCommandSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) = 0;

    //!
    //! \brief    Calculates maximum size for HUC slice/MB level commands
    //! \details  Client facing function to calculate maximum size for HUC slice/MB level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!            The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHucPrimitiveCommandSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize) = 0;

    //!
    //! \brief    Adds Huc pipe mode select command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHucPipeModeSelectCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS   *params) = 0;

    //!
    //! \brief    Adds Huc IMEM State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHucImemStateCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_HUC_IMEM_STATE_PARAMS     *params) = 0;

    //!
    //! \brief    Adds Huc DMEM State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHucDmemStateCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_HUC_DMEM_STATE_PARAMS     *params) = 0;

    //!
    //! \brief    Adds Huc Virtual Addr State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHucVirtualAddrStateCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS   *params) = 0;

    //!
    //! \brief    Adds Huc Indirect Object Base Addr State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHucIndObjBaseAddrStateCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  *params) = 0;

    //!
    //! \brief    Adds Huc Stream Object command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHucStreamObjectCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_HUC_STREAM_OBJ_PARAMS     *params) = 0;

    //!
    //! \brief    Adds Huc Start command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] lastStreamObject
    //!           Set last stream object or not
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHucStartCmd(
        MOS_COMMAND_BUFFER             *cmdBuffer,
        bool                            lastStreamObject) = 0;

    //!
    //! \brief    Set cacheability settings
    //!
    //! \param    [in] cacheabilitySettings
    //!           Cacheability settings
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCacheabilitySettings(
        MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC])
    {
        MHW_FUNCTION_ENTER;

        uint32_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);
        MOS_STATUS eStatus = MOS_SecureMemcpy(m_cacheabilitySettings, size,
            cacheabilitySettings, size);

        return eStatus;
    }

    //!
    //! \brief    Get huc product family
    //!
    //! \return   uint32_t
    //!           Huc product family.
    //!
    virtual uint32_t GetHucProductFamily() = 0;
};

#endif
