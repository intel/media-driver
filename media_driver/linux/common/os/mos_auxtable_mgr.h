/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file    mos_auxtable_mgr.h
//! \brief   Container class for GMM aux table manager wrapper
//!

#ifndef MOS_AUXTABLE_MGR_H
#define MOS_AUXTABLE_MGR_H

#include "xf86drm.h"
#include "drm.h"
#include "i915_drm.h"
#include "mos_bufmgr.h"
#include "mos_os.h"

//!
//! \class  AuxTableMgr
//! \brief  Aux Table Manager
//!
class AuxTableMgr
{
public:
    //!
    //! \brief  Constructor
    //!
    AuxTableMgr(MOS_BUFMGR *bufMgr);

    //!
    //! \brief  Destructor
    //!
    virtual ~AuxTableMgr();

    //!
    //! \brief    Create AuxTableMgr object
    //! \details  Map GMM resource into aux table if needed and also mark it in bo.
    //! \param    [in] bufMgr
    //!           MOS_BUFMGR pointer
    //! \param    [in] sku
    //!           Sku table pointer
    //! \param    [in] wa
    //!           WA table pointer
    //! \return   Object pointer to AuxTableMgr
    //!           Return object pointer if success or return nullptr if failed
    //!
    static AuxTableMgr * CreateAuxTableMgr(MOS_BUFMGR *bufMgr, MEDIA_FEATURE_TABLE *sku);

    //!
    //! \brief    Map resource to aux table
    //! \details  Map GMM resource into aux table if needed and also mark it in bo.
    //! \param    [in] gmmResInfo
    //!           GMM_RESOURCE_INFO pointer
    //! \param    [in] bo
    //!           MOS_LINUX_BO pointer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS  MapResource(GMM_RESOURCE_INFO *gmmResInfo, MOS_LINUX_BO *bo);

    //!
    //! \brief    Unmap resource from aux table
    //! \details  Unmap resource from aux table if it was mapped and unmark it in bo.
    //! \param    [in] gmmResInfo
    //!           GMM_RESOURCE_INFO pointer
    //! \param    [in] bo
    //!           MOS_LINUX_BO pointer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS  UnmapResource(GMM_RESOURCE_INFO *gmmResInfo, MOS_LINUX_BO *bo);

    //!
    //! \brief    Insert aux table BOs into specific execute buffer
    //! \details  Retrieve all BOs allocated for aux table and attach them to execute buffer 
    //!           so that they are pinned before h/w access it.
    //! \param    [out] cmd_bo
    //!           execute buffer bo to be attached
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS  EmitAuxTableBOList(MOS_LINUX_BO *cmd_bo);

    //!
    //! \brief    Get aux table base address
    //! \details  Get current root aux table base address
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    uint64_t    GetAuxTableBase();

private:
    GMM_CLIENT_CONTEXT *m_gmmClientContext = nullptr;     //!<  GMM Client Context for GMM Page table manager
    void *m_gmmPageTableMgr = nullptr;                    //!<  The GMM Page Table Manager
};

#endif //MOS_AUXTABLE_MGR_H
