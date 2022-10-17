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
//! \file      codechal_hw_xe_xpm_plus.h
//! \brief     This modules implements HW interface layer to be used on XeHP platforms on all operating systems/DDIs, across CODECHAL components.
//!
#ifndef __CODECHAL_HW_XE_XPM_PLUS_H__
#define __CODECHAL_HW_XE_XPM_PLUS_H__

#include "codechal_hw.h"
#include "codechal_hw_g12_X.h"
#include "media_blt_copy_xe_xpm_base.h"


//!  Codechal hw interface Gen12
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for Gen12 platforms
*/
class CodechalHwInterfaceXe_Xpm_Plus : public CodechalHwInterfaceG12
{
protected:
    BltStateXe_Xpm *m_bltState = nullptr;       //!< Pointer to blt state
public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceXe_Xpm_Plus(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfaces     *mhwInterfaces,
        bool              disableScalability = false);

    //!
    //! \brief    Copy constructor
    //!
    CodechalHwInterfaceXe_Xpm_Plus(const CodechalHwInterfaceXe_Xpm_Plus&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalHwInterfaceXe_Xpm_Plus& operator=(const CodechalHwInterfaceXe_Xpm_Plus&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceXe_Xpm_Plus()
    {
        if (m_bltState)
        {
            MOS_Delete(m_bltState);
            m_bltState = nullptr;
        }
    }

    virtual bool UsesRenderEngine(CODECHAL_FUNCTION codecFunction, uint32_t standard) override
    {
        // Render Engine is not not supported
        return false;
    }

    //! \brief    Get blt state
    //! \details  Get blt interface in codechal hw interface
    //!
    //! \return   [out] BltState*
    //!           Interface got.
    //!
    inline BltStateXe_Xpm *GetBltState()
    {
        return m_bltState;
    }

    //!
    //! \brief    Calculates the maximum size for Vdenc picture 2nd level commands
    //! \details  Client facing function to calculate the maximum size for Vdenc picture 2nd level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetVdencPictureSecondLevelCommandsSize(
        uint32_t  mode,
        uint32_t *commandsSize) override;

    //!
    //! \brief    Get film grain kernel info
    //! \details  Get kernel base and size
    //!
    //! \param    [out] kernelBase
    //!           base addr of film grain kernels
    //!
    //! \param    [out] kernelSize
    //!           size of film grain kernels
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetFilmGrainKernelInfo(
        uint8_t*    &kernelBase,
        uint32_t    &kernelSize) override;

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
            MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) override;

    //! \brief    Create media copy
    //! \details  Create media copy instance.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //! \return   MediaCopyBaseState*
    //!           Pointer to MediaCopyBaseState
    //!
    virtual MediaCopyBaseState* CreateMediaCopy(PMOS_INTERFACE mosInterface) override;

private:
    //!
    //! \brief    Called by constructor
    //!
    void PrepareCmdSize(CODECHAL_FUNCTION codecFunction);
};

#endif // __CODECHAL_HW_XE_XPM_PLUS_H__
