/*
* Copyright (c) 2021, Intel Corporation
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
//! \file      codec_hw_next.h
//! \brief     This modules implements HW interface layer to be used on all platforms on all operating systems/DDIs, across CODECHAL components. 
//!

#ifndef __CODEC_HW_NEXT_H__
#define __CODEC_HW_NEXT_H__

#include "codechal.h"
#include "media_interfaces_mhw_next.h"

// Remove legacy header files
#include "mhw_vdbox_mfx_interface.h"
#include "mhw_vdbox_hcp_interface.h"
#include "mhw_vdbox_vdenc_interface.h"
#include "mhw_vdbox_huc_interface.h"
#include "mhw_mi_itf.h"
//------------------------------------------------------------------------------
// Macros specific to MOS_CODEC_SUBCOMP_HW sub-comp
//------------------------------------------------------------------------------
#define CODEC_HW_ASSERT(_expr)                                                          \
    MOS_ASSERT(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _expr)

#define CODEC_HW_ASSERTMESSAGE(_message, ...)                                           \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define CODEC_HW_NORMALMESSAGE(_message, ...)                                           \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define CODEC_HW_VERBOSEMESSAGE(_message, ...)                                          \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define CODEC_HW_FUNCTION_ENTER                                                         \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW)

#define CODEC_HW_CHK_STATUS(_stmt)                                                      \
    MOS_CHK_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _stmt)

#define CODEC_HW_CHK_STATUS_RETURN(_stmt)                                               \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _stmt)

#define CODEC_HW_CHK_STATUS_MESSAGE(_stmt, _message, ...)                               \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _stmt, _message, ##__VA_ARGS__)

#define CODEC_HW_CHK_NULL(_ptr)                                                         \
    MOS_CHK_NULL(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _ptr)

#define CODEC_HW_CHK_NULL_RETURN(_ptr)                                                  \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _ptr)

#define CODEC_HW_CHK_NULL_NO_STATUS(_ptr)                                               \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _ptr)

#define CODEC_HW_CHK_COND_RETURN(_expr, _message, ...)                                  \
    MOS_CHK_COND_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW,_expr,_message, ##__VA_ARGS__)

//!  Codec hw next interface
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal
*/
class CodechalHwInterfaceNext
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceNext(
        PMOS_INTERFACE     osInterface,
        CODECHAL_FUNCTION  codecFunction,
        MhwInterfacesNext  *mhwInterfacesNext,
        bool               disableScalability = false);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceNext() {}

    //!
    //! \brief    Get avp interface
    //! \details  Get avp interface in codechal hw interface next
    //!
    //! \return    pointer to new AVP interface
    //!
    inline std::shared_ptr<mhw::vdbox::avp::Itf> GetAvpInterfaceNext()
    {
        return m_avpItf;
    }

    //!
    //! \brief    Get vdenc interface
    //! \details  Get vdenc interface in codechal hw interface next
    //!
    //! \return    pointer to new VDENC interface
    //!
    inline std::shared_ptr<mhw::vdbox::vdenc::Itf> GetVdencInterfaceNext()
    {
        return m_vdencItf;
    }
    
    //! \brief    Get huc interface
    //! \details  Get huc interface in codechal hw interface next
    //!
    //! \return    pointer to new HUC interface
    //!
    inline std::shared_ptr<mhw::vdbox::huc::Itf> GetHucInterfaceNext()
    {
        return m_hucItf;
    }

    //!
    //! \brief    Get mi interface
    //! \details  Get mi interface in codechal hw interface next
    //!
    //! \return    pointer to new MI interface
    //!
    inline std::shared_ptr<mhw::mi::Itf> GetMiInterfaceNext()
    {
        return m_miItf;
    }

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
    virtual MOS_STATUS GetAvpPrimitiveCommandSize(
            uint32_t                        mode,
            uint32_t                        *commandsSize,
            uint32_t                        *patchListSize);

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
        MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]);

    //!
    //! \brief    Calculates the maximum size for Huc picture level commands
    //! \details  Client facing function to calculate the maximum size for Huc picture level commands
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
    virtual MOS_STATUS GetHucStateCommandSize(
            uint32_t mode,
            uint32_t* commandsSize,
            uint32_t* patchListSize,
            PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

protected:
    std::shared_ptr<mhw::vdbox::avp::Itf>    m_avpItf = nullptr;      //!< Pointer to Mhw avp interface
    std::shared_ptr<mhw::vdbox::vdenc::Itf>  m_vdencItf = nullptr;      //!< Pointer to Mhw vdenc interface
    std::shared_ptr<mhw::vdbox::huc::Itf>    m_hucItf = nullptr;      //!< Pointer to Mhw huc interface
    std::shared_ptr<mhw::mi::Itf>            m_miItf = nullptr;       //!< Pointer to Mhw mi interface

    // Next: remove legacy Interfaces
    MhwCpInterface                  *m_cpInterface = nullptr;         //!< Pointer to Mhw cp interface
    MhwVdboxMfxInterface            *m_mfxInterface = nullptr;        //!< Pointer to Mhw mfx interface
    MhwVdboxHcpInterface            *m_hcpInterface = nullptr;        //!< Pointer to Mhw hcp interface
    MhwVdboxVdencInterface          *m_vdencInterface = nullptr;      //!< Pointer to Mhw vdenc interface

MEDIA_CLASS_DEFINE_END(CodechalHwInterfaceNext)
};

#endif  // __CODEC_HW_NEXT_H__
