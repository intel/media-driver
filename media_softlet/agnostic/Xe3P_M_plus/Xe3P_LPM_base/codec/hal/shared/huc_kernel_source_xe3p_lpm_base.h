/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     huc_kernel_source_xe3p_lpm_base.h
//! \brief    Header file of the Xe3P_LPM_Base Huc kernel source management.
//!
#ifndef __HUC_KERNEL_SOURCE_XE3P_LPM_BASE_H__
#define __HUC_KERNEL_SOURCE_XE3P_LPM_BASE_H__

#include "huc_kernel_source.h"

class HucKernelSourceXe3P_Lpm_Base : public HucKernelSource
{
public:

    //!
    //! \brief    Copy constructor
    //!
    HucKernelSourceXe3P_Lpm_Base(const HucKernelSourceXe3P_Lpm_Base &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    HucKernelSourceXe3P_Lpm_Base &operator=(const HucKernelSourceXe3P_Lpm_Base &) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~HucKernelSourceXe3P_Lpm_Base() {}

    //!
    //! \brief    Return manifest for Huc kernels
    //! \param  [out] manifest
    //!         Manifest for Huc kernels in order
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetManifest(HucManifest &manifest) override;

    static HucKernelSourceXe3P_Lpm_Base &GetInstance();

private:
    //!
    //! \brief    Constructor
    //!
    HucKernelSourceXe3P_Lpm_Base(): HucKernelSource() {}

    virtual const BinaryTable  &GetBinTable() override       { return m_binTable; }
    virtual const HashIdxTable &GetHashIdxTable() override { return m_hashIdxTable; }

private:
    // Binary map table (kernel id, binay data point)
    static const BinaryTable m_binTable;

    // Hash index table (kernel id, hash index)
    static const HashIdxTable m_hashIdxTable;

MEDIA_CLASS_DEFINE_END(HucKernelSourceXe3P_Lpm_Base)
};

#endif  // __HUC_KERNEL_SOURCE_XE3P_LPM_BASE_H__

