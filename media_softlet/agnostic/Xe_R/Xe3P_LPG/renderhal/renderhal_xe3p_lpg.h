/*===================== begin_copyright_notice ==================================

# Copyright (c) 2023, Intel Corporation

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
//! \file       renderhal_xe3p_lpg.h
//! \brief      header file of xe3p_lpg hardware functions
//! \details    xe3p_lpg hardware functions declare
//!
#ifndef __RENDERHAL_XE3P_LPG_H__
#define __RENDERHAL_XE3P_LPG_H__

#include "renderhal_xe3p_lpg_base.h"
#include "mhw_render_hwcmd_xe3p_lpg.h"
#include "mhw_state_heap_xe3p_lpg.h"

class XRenderHal_Interface_Xe3P_Lpg : public XRenderHal_Interface_Xe3P_Lpg_Base
{
public:
    XRenderHal_Interface_Xe3P_Lpg() {}

    virtual ~XRenderHal_Interface_Xe3P_Lpg() {}

    //! \brief      Get the size of Render Surface State Command
    //! \return     size_t
    //!             the size of render surface state command
    size_t GetSurfaceStateCmdSize() override;

    //! \brief      Get the address of the ith Palette Data
    //! \param      [in] i
    //!             Index of the palette data
    //! \return     void *
    //!             address of the ith palette data table
    void *GetPaletteDataAddress(int i) override
    {
        return &m_paletteData[i];
    }

    //! \brief      Get the size of Binding Table State Command
    //! \return     size_t
    //!             the size of binding table state command
    size_t GetBTStateCmdSize() override
    { 
        return mhw_state_heap_xe3p_lpg::BINDING_TABLE_STATE_CMD::byteSize; 
    }

protected:
    mhw::render::xe3p_lpg::Cmd::PALETTE_ENTRY_CMD
        m_paletteData[RENDERHAL_PALETTE_MAX][RENDERHAL_PALETTE_ENTRIES_MAX];

MEDIA_CLASS_DEFINE_END(XRenderHal_Interface_Xe3P_Lpg)
};

#endif // __RENDERHAL_XE3P_LPG_H__
