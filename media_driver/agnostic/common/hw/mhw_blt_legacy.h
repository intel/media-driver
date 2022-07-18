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
//! \file     mhw_blt_legacy.h
//! \brief    MHW interface for constructing commands for the BLT
//!
#ifndef __MHW_BLT_LEGACY_H__
#define __MHW_BLT_LEGACY_H__

#include "mhw_blt.h"

class MhwBltInterface
{
public:
    MhwBltInterface(PMOS_INTERFACE pOsInterface);

    virtual ~MhwBltInterface()
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief    Add fast copy blt
    //! \details  MHW function to add fast copy blt command
    //! \param    [in] pCmdBuffer
    //!           Pointer to Command buffer
    //! \param    [in] pFastCopyBltParam
    //!           Pointer to MHW_FAST_COPY_BLT_PARAM
    //! \param    [in] srcOffset
    //!           input surface's soruce offset
    //! \param    [in] outOffset
    //!           output surface's soruce offset
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddFastCopyBlt(
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_FAST_COPY_BLT_PARAM    pFastCopyBltParam,
        uint32_t                    srcOffset,
        uint32_t                    dstOffset);

    //!
    //! \brief    Add Block copy
    //! \details  MHW function to add block copy command
    //! \param    [in] pCmdBuffer
    //!           Pointer to Command buffer
    //! \param    [in] pFastCopyBltParam
    //!           Pointer to MHW_FAST_COPY_BLT_PARAM
    //! \param    [in] srcOffset
    //!           input surface's soruce offset
    //! \param    [in] outOffset
    //!           output surface's soruce offset
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddBlockCopyBlt(
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_FAST_COPY_BLT_PARAM    pFastCopyBltParam,
        uint32_t                    srcOffset,
        uint32_t                    dstOffset);

    MOS_STATUS (*pfnAddResourceToCmd)(
        PMOS_INTERFACE          pOsInterface,
        PMOS_COMMAND_BUFFER     pCmdBuffer,
        PMHW_RESOURCE_PARAMS    pParams);

    //!
    //! \brief    Get fastcopy tilling mode
    //! \details  Get fastcopy tilling mode
    //! \param    [in] TileType
    //!           Pointer to BLT_TILE_TYPE
    //! \return   uint32_t
    //!           0:1:2:3
    //!
    virtual uint32_t GetFastTilingMode(
        BLT_TILE_TYPE              TileType);

    virtual std::shared_ptr<void> GetNewBltInterface() { return nullptr; }

public:
    PMOS_INTERFACE         m_osInterface = nullptr;
    std::shared_ptr<void>  m_bltItfNew   = nullptr;
};

typedef class MhwBltInterface *PMHW_BLT_INTERFACE;

#endif  // __MHW_BLT_LEGACY_H__
