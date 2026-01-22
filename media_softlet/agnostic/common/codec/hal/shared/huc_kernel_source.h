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
//! \file     huc_kernel_source.h
//! \brief    Defines the common interface for Huc kernel source management.
//! \details  The Huc kernel source is further sub-divided by platforms.
//!
#ifndef __HUC_KERNEL_SOURCE_H__
#define __HUC_KERNEL_SOURCE_H__

#include <map>
#include "media_class_trace.h"
#include "media_pipeline.h"

#if defined(MEDIA_BIN_SUPPORT)
#define HUC_KERNEL_BIN_ELEMENT(bin) \
{                                   \
    (uint32_t * const *)&(bin),     \
    (uint32_t *)&(bin##_size)       \
}
#else
#define HUC_KERNEL_BIN_ELEMENT(bin)         \
{                                           \
    (uint32_t * const *)&(bin##_local),     \
    (uint32_t *)&(bin##_size_local)         \
}
#define HUC_KERNEL_BIN_LOCAL(bin)                 \
    const uint32_t * const bin##_local = &bin[0]; \
    const uint32_t bin##_size_local = bin##_size;
#endif

//! \brief Interface of the Huc binary management.
class HucKernelSource
{
public:
    struct HucManifest
    {
        const uint8_t *m_data = nullptr;
        uint32_t       m_size = 0;
    };

    struct HucBinary
    {
        const uint8_t *m_data = nullptr;
        uint32_t       m_size = 0;
    };

    enum KernelId
    {
        hevcS2lKernelId           = 1,
        drmKernelId               = 2,
        copyKernelId              = 3,
        vdencBrcInitKernelId      = 4,
        vdencBrcUpdateKernelId    = 5,
        vp9ProbUpdateKernelId     = 6,
        vp9EncKernelId            = 7,
        hevcBrcInitKernelId       = 8,
        hevcBrcUpdateKernelId     = 9,
        hevcBrcLowdelayKernelId   = 10,
        vp9VdencBrcInitKernelId   = 11,
        vp9VdencBrcUpdateKernelId = 12,
        vp9VdencProbKernelId      = 13,
        cmdInitializerKernelId    = 14,
        pakIntegrationKernelId    = 15,
        hevcLaAnalysisKernelId    = 16,
        backAnnonationKernelId    = 17,
        av1BrcInitKernelId        = 18,
        av1BrcUpdateKernelId      = 19,
        vvcS2lKernelId            = 20,
        avcPxpBrcInitKernelId     = 21,
        avcPxpBrcUpdateKernelId   = 22,
        av1SlbbUpdateKernelId     = 23,  // AV1 SLBB update kernel
        avcSlbbUpdateKernelId     = 24,  // AVC SLBB update kernel
        hevcSlbbUpdateKernelId    = 25   // HEVC SLBB update kernel
    };

    //!
    //! \brief    Copy constructor
    //!
    HucKernelSource(const HucKernelSource &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    HucKernelSource &operator=(const HucKernelSource &) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~HucKernelSource() {}

    //!
    //! \brief    Return manifest for Huc kernels
    //! \param  [out] manifest
    //!         Manifest for Huc kernels in order
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetManifest(HucManifest &manifest) = 0;

    //!
    //! \brief    Return hash index for specified kernel
    //! \param  [in] kernelId
    //!         Kernel ID for hash index query
    //! \param  [out] hashIndex
    //!         Kernel hash index returned if kernel ID is valid
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetKernelHashIdx(const uint32_t kernelId, uint32_t &hashIndex)
    {
        auto &hashIdxTable = GetHashIdxTable();
        auto iter = hashIdxTable.find(kernelId);
        if (iter == hashIdxTable.end())
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (iter->second < GetMinHashIndex() || iter->second > GetMaxHashIndex())
        {
            MOS_OS_CRITICALMESSAGE("hask index %d out of range [%d, %d].",
                iter->second, GetMinHashIndex(), GetMaxHashIndex());
            return MOS_STATUS_INVALID_PARAMETER;
        }

        hashIndex = iter->second;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Return binary for specified kernel
    //! \param  [in] kernelId
    //!         Kernel ID for hash index query
    //! \param  [out] hucBinary
    //!         Kernel binary returned if kernel ID is valid
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetKernelBin(const uint32_t kernelId, HucBinary &hucBinary)
    {
        auto &binTable = GetBinTable();
        auto iter = binTable.find(kernelId);
        if (iter == binTable.end())
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        hucBinary.m_data = (iter->second.m_data == nullptr) ? nullptr : reinterpret_cast<const uint8_t *>(*(iter->second.m_data));
        hucBinary.m_size = (iter->second.m_size == nullptr) ? 0       : *(iter->second.m_size);
        return MOS_STATUS_SUCCESS;
    }

    virtual bool IsPpgttMode(MEDIA_FEATURE_TABLE *skuTable, MediaUserSettingSharedPtr userSettingPtr)
    {
        if (skuTable == nullptr)
        {
            return false;
        }

        if(!MEDIA_IS_SKU(skuTable, FtrPPGTTBasedHuCLoad))
        {
            return false;
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        const std::string enableKey = "PPGTT Huc Enable";

        MOS_STATUS status = DeclareUserSettingKeyForDebug(
            userSettingPtr,
            enableKey,
            MediaUserSetting::Group::Device,
            true, // enable by default
            false);

        if (status == MOS_STATUS_SUCCESS || status == MOS_STATUS_FILE_EXISTS)
        {
            // check if disabled by regkey
            bool enablePpgtt = true;
            ReadUserSettingForDebug(
                userSettingPtr,
                enablePpgtt,
                enableKey,
                MediaUserSetting::Group::Device);

            if (!enablePpgtt)
            {
                return false;
            }
        }
#endif

        return true;
    }

    virtual MOS_STATUS ReportMode(MEDIA_FEATURE_TABLE *skuTable, MediaUserSettingSharedPtr userSettingPtr)
    {
        MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, skuTable);

        const std::string inUseKey = "PPGTT Huc In Use";

        MOS_STATUS status = DeclareUserSettingKey(
            userSettingPtr,
            inUseKey,
            MediaUserSetting::Group::Device,
            (int32_t)0,
            true);
        if (status == MOS_STATUS_SUCCESS || status == MOS_STATUS_FILE_EXISTS)
        {
            bool isPpgttEnable = IsPpgttMode(skuTable, userSettingPtr);

            ReportUserSetting(
                userSettingPtr,
                inUseKey,
                isPpgttEnable ? 1 : 0,
                MediaUserSetting::Group::Device);
        }

        return MOS_STATUS_SUCCESS;
    }

protected:
    //!
    //! \brief    Constructor
    //!
    HucKernelSource() {}

    struct HucBinaryInternal
    {
        uint32_t * const *m_data = nullptr;
        uint32_t         *m_size = nullptr;
    };

    // Binary map table (kernel id, binay data point)
    using BinaryTable  = std::map<const uint32_t, const HucBinaryInternal>;

    // Hash index table (kernel id, hash index)
    using HashIdxTable = std::map<const uint32_t, const uint32_t>;

    constexpr static HucBinaryInternal m_invalidKernelBin = { nullptr, nullptr };

    constexpr static uint32_t m_invalidHashIndex = 0xff; //!< Invalid hash index

private:
    virtual const BinaryTable  &GetBinTable() = 0;
    virtual const HashIdxTable &GetHashIdxTable() = 0;

    virtual const uint32_t GetMinHashIndex() { return m_minHashIndex; }
    virtual const uint32_t GetMaxHashIndex() { return m_maxHashIndex; }

    constexpr static uint32_t m_minHashIndex = 1;  //!< min hash index
    constexpr static uint32_t m_maxHashIndex = 31; //!< max hash index

MEDIA_CLASS_DEFINE_END(HucKernelSource)
};

#endif  // __HUC_KERNEL_SOURCE_H__
