/*===================== begin_copyright_notice ==================================
# Copyright (c) 2020-2021, Intel Corporation
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/
//!
//! \file      codechal_hw_next_xe_hpm.h
//! \brief     This modules implements HW interface layer to be used on dg2 platforms on all operating systems/DDIs, across CODECHAL components.
//!
#ifndef __CODECHAL_HW_NEXT_XE_HPM_H__
#define __CODECHAL_HW_NEXT_XE_HPM_H__

#include "codechal_hw.h"
#include "codechal_debug.h"

//!  Codechal hw interface Gen12
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for Gen12 platforms
*/
class CodechalHwInterfaceNextXe_Hpm : public CodechalHwInterfaceNext
{
public:

    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceNextXe_Hpm(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfacesNext *mhwInterfacesNext,
        bool              disableScalability);

    //!
    //! \brief    Copy constructor
    //!
    CodechalHwInterfaceNextXe_Hpm(const CodechalHwInterfaceNextXe_Hpm &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalHwInterfaceNextXe_Hpm &operator=(const CodechalHwInterfaceNextXe_Hpm &) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceNextXe_Hpm();

    bool UsesRenderEngine(CODECHAL_FUNCTION codecFunction, uint32_t standard);

    bool Uses2PlanesInputSurfaceFilmGrain()
    {
        return true;
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
            PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) override;

#if USE_CODECHAL_DEBUG_TOOL
    //! \brief    Create media copy
    //! \details  Create media copy instance.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //! \return   MediaCopyBaseState*
    //!           Pointer to MediaCopyBaseState
    //!
    virtual MediaCopyBaseState *CreateMediaCopy(PMOS_INTERFACE mosInterface) override;
#endif

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

    virtual MOS_STATUS Initialize(
        CodechalSetting *settings) override;
private:
    //!
    //! \brief    Called by constructor
    //!
    void PrepareCmdSize(CODECHAL_FUNCTION codecFunction);

    std::shared_ptr<MhwMiInterface> m_miInterface     = nullptr;
    MhwRenderInterface             *m_renderInterface = nullptr;

MEDIA_CLASS_DEFINE_END(CodechalHwInterfaceNextXe_Hpm)
};

#endif // __CODECHAL_HW_NEXT_XE_HPM_H__