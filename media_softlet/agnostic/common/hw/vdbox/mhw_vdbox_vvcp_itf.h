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
//! \file     mhw_vdbox_vvcp_itf.h
//! \brief    MHW VDBOX VVCP interface common base
//! \details
//!

#ifndef __MHW_VDBOX_VVCP_ITF_H__
#define __MHW_VDBOX_VVCP_ITF_H__

#include "mhw_itf.h"
#include "mhw_vdbox.h"
#include "mhw_cp_interface.h"
#include "mhw_vdbox_vvcp_cmdpar.h"

// add all VVCP cmds here
#define _VVCP_CMD_DEF(DEF)             \
    DEF(VVCP_VD_CONTROL_STATE);        \
    DEF(VVCP_PIPE_MODE_SELECT);        \
    DEF(VVCP_SURFACE_STATE);           \
    DEF(VVCP_PIPE_BUF_ADDR_STATE);     \
    DEF(VVCP_IND_OBJ_BASE_ADDR_STATE); \
    DEF(VVCP_PIC_STATE);               \
    DEF(VVCP_DPB_STATE);               \
    DEF(VVCP_SLICE_STATE);             \
    DEF(VVCP_BSD_OBJECT);              \
    DEF(VVCP_REF_IDX_STATE);           \
    DEF(VVCP_WEIGHTOFFSET_STATE);      \
    DEF(VVCP_TILE_CODING);

namespace mhw
{
namespace vdbox
{
namespace vvcp
{

//!
//! \struct   MmioRegistersVvcp
//! \brief    MMIO registers VVCP
//!
struct MmioRegistersVvcp
{
    //TODO: Enalbe VVC new mechanism for MMIO
};

class Itf
{
public:
    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;

        _VVCP_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~Itf() = default;

    //!
    //! \brief    Set cacheability settings
    //!
    //! \param    [in] settings
    //!           Cacheability settings
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) = 0;

    //!
    //! \brief    Get the required buffer size for VDBOX
    //! \details  Internal function to get required buffer size for VVC codec
    //!
    //! \param    [in] bufferType
    //!           VVC Buffer type
    //! \param    [in, out] vvcpBufSizeParam
    //!           VVCP buffer size parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetVvcpBufSize(VvcpBufferType bufferType, VvcpBufferSizePar *vvcpBufSizeParam) = 0;

    //!
    //! \brief    Judge if row store caching is supported, overall flag for VVCP
    //!
    //! \return   bool
    //!           true if supported, else false
    //!
    virtual bool IsRowStoreCachingSupported() = 0;

    //!
    //! \brief    Judge if row store cache of a particular buffer type is enabled
    //!
    //! \param    [in] bufferType
    //!           VVC Buffer type
    //! 
    //! \return   bool
    //!           true if enabled, else false
    //!
    virtual bool IsBufferRowstoreCacheEnabled(VvcpBufferType bufferType) = 0;

    virtual MOS_STATUS GetRowstoreCachingAddrs(PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams) = 0;

    //!
    //! \brief    Calculates the maximum size for VVCP picture level commands
    //! \details  Client facing function to calculate the maximum size for VVCP picture level commands
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] params
    //!           Indicate the command size parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetVvcpStateCmdSize(uint32_t *commandsSize, uint32_t *patchListSize, PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) = 0;

    virtual MOS_STATUS GetVvcpSliceLvlCmdSize(uint32_t *sliceLvlCmdSize) = 0;

    //!
    //! \brief    Calculates maximum size for VVCP primitive level commands
    //! \details  Client facing function to calculate maximum size for VVCP primitive level commands
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
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetVvcpPrimitiveCmdSize(
        uint32_t *sliceCommandsSize,
        uint32_t *slicePatchListSize,
        uint32_t *tileCommandsSize,
        uint32_t *tilePatchListSize) = 0;

    //!
    //! \brief    Store VVCP ALF APS data to the given ALF buffer
    //!
    //! \param    [in, out] buffer
    //!           Buffer to store ALF APS data
    //! \param    [in] alfApsArray
    //!           Pointer to VVC ALF data array
    //! \param    [in] activeAlfMask
    //!           Active ALF mask
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetAlfApsDataBuffer(uint32_t *buffer, CodecVvcAlfData *alfApsArray, uint8_t activeAlfMask) = 0;

    //!
    //! \brief    Get mmio registers
    //!
    //! \param    [in] index
    //!           mmio registers index
    //!
    //! \return   [out] MmioRegistersVvcp*
    //!           VVC mmio registers
    //!
    //virtual MmioRegistersVvcp* GetMmioRegisters(MHW_VDBOX_NODE_IND index)
    //{
    //    // TODO: new mechanism for VVC MMIO.
    //};

    //!
    //! \brief    Determines if the slice is P slice
    //! \param    [in] sliceType
    //!           slice type
    //! \return   bool
    //!           True if it's P slice, otherwise return false
    //!
    inline bool IsVvcPSlice(uint8_t sliceType)
    {
        return (sliceType < MHW_ARRAY_SIZE(m_vvcBsdSliceType)) ? (m_vvcBsdSliceType[sliceType] == vvcSliceP) : false;
    }

    //!
    //! \brief    Determines if the slice is B slice
    //! \param    [in] sliceType
    //!           slice type
    //! \return   bool
    //!           True if it's B slice, otherwise return false
    //!
    inline bool IsVvcBSlice(uint8_t sliceType)
    {
        return (sliceType < MHW_ARRAY_SIZE(m_vvcBsdSliceType)) ? (m_vvcBsdSliceType[sliceType] == vvcSliceB) : false;
    }

    //!
    //! \brief    Determines if the slice is I slice
    //! \param    [in] sliceType
    //!           slice type
    //! \return   bool
    //!           True if it's I slice, otherwise return false
    //!
    inline bool IsVvcISlice(uint8_t sliceType)
    {
        return (sliceType < MHW_ARRAY_SIZE(m_vvcBsdSliceType)) ? (m_vvcBsdSliceType[sliceType] == vvcSliceI) : false;
    }

    uint32_t GetMappedCoeff(uint32_t coeffValue)
    {
        uint32_t mappedCoeff = 0;
        if(alfCoeffMap.find(coeffValue) != alfCoeffMap.end())
        {
            mappedCoeff = alfCoeffMap.find(coeffValue)->second;
        }
        return mappedCoeff;
    }

    _VVCP_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);

private:
    const VvcSliceType m_vvcBsdSliceType[3] =
    {
        vvcSliceB,
        vvcSliceP,
        vvcSliceI
    };
    const std::map<uint32_t, uint32_t> alfCoeffMap = {
        {1, 1},
        {2, 2},
        {4, 3},
        {8, 4},
        {16, 5},
        {32, 6},
        {64, 7}
    };

    MEDIA_CLASS_DEFINE_END(mhw__vdbox__vvcp__Itf)
};

}  // namespace vvcp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VVCP_ITF_H__
