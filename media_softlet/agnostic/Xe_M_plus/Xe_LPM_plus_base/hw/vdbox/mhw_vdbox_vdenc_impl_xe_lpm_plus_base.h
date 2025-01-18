/*===================== begin_copyright_notice ==================================

# Copyright (c) 2022, Intel Corporation

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
//! \file     mhw_vdbox_vdenc_impl_xe_lpm_plus_base.h
//! \brief    MHW VDBOX VDENC interface common base for Xe_LPM_plus+ platforms
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_IMPL_XE_LPM_PLUS_BASE_H__
#define __MHW_VDBOX_VDENC_IMPL_XE_LPM_PLUS_BASE_H__

#include "mhw_vdbox_vdenc_impl.h"
#include "mos_interface.h"

namespace mhw
{
namespace vdbox
{
namespace vdenc
{
namespace xe_lpm_plus_base
{
template <typename cmd_t>
class BaseImplGeneric : public vdenc::Impl<cmd_t>
{
protected:
    using base_t = vdenc::Impl<cmd_t>;

    BaseImplGeneric(PMOS_INTERFACE osItf) : base_t(osItf){};

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(VDENC_PIPE_MODE_SELECT);
        
    #define DO_FIELDS()                                                                  \
        DO_FIELD(DW4, VDENC_PIPE_MODE_SELECT_DW4_BIT22, params.VdencPipeModeSelectPar2); \
        DO_FIELD(DW4, VDENC_PIPE_MODE_SELECT_DW4_BIT24, params.VdencPipeModeSelectPar3); \
        DO_FIELD(DW4, VDENC_PIPE_MODE_SELECT_DW4_BIT26, params.VdencPipeModeSelectPar4); \
        DO_FIELD(DW4, VDENC_PIPE_MODE_SELECT_DW4_BIT28, params.VdencPipeModeSelectPar5); \
        DO_FIELD(DW4, VDENC_PIPE_MODE_SELECT_DW4_BIT16, params.VdencPipeModeSelectPar6); \
        DO_FIELD(DW4, VDENC_PIPE_MODE_SELECT_DW4_BIT18, params.VdencPipeModeSelectPar7); \
        DO_FIELD(DW4, VDENC_PIPE_MODE_SELECT_DW4_BIT12, 0);

    #include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_PIPE_BUF_ADDR_STATE);

        {                                                                                                 
            MHW_RESOURCE_PARAMS resourceParams = {};                                                      
            resourceParams.HwCommandType       = MOS_VDENC_PIPE_BUF_ADDR;
            if (!Mos_ResourceIsNull(params.vdencPipeBufAddrStatePar0))                                    
            {                                                                                             
                resourceParams.presResource    = params.vdencPipeBufAddrStatePar0;                        
                resourceParams.dwOffset        = 0;                                                       
                resourceParams.pdwCmd          = (uint32_t *)&(cmd.VDENC_PIPE_BUF_ADDR_STATE_DW71_73.LowerAddress);  
                resourceParams.dwLocationInCmd = 71;                                                      
                resourceParams.bIsWritable     = false;                                                   
                                                                                                          
                cmd.VDENC_PIPE_BUF_ADDR_STATE_DW71_73.PictureFields.DW0.MemoryCompressionEnable  = 0;                 
                cmd.VDENC_PIPE_BUF_ADDR_STATE_DW71_73.PictureFields.DW0.CompressionType          = 0;
                InitMocsParams(resourceParams, &cmd.VDENC_PIPE_BUF_ADDR_STATE_DW71_73.PictureFields.DW0.Value, 1, 6);
                                                                                                          
                MHW_MI_CHK_STATUS(this->AddResourceToCmd(                                                 
                    this->m_osItf,                                                                        
                    this->m_currentCmdBuf,                                                                
                    &resourceParams));                                                                    
            }                                                                                             
            if (!Mos_ResourceIsNull(params.vdencPipeBufAddrStatePar1))                                    
            {                                                                                             
                resourceParams.presResource    = params.vdencPipeBufAddrStatePar1;                        
                resourceParams.dwOffset        = 0;                                                       
                resourceParams.pdwCmd          = (uint32_t *)&(cmd.VDENC_PIPE_BUF_ADDR_STATE_DW74_76.LowerAddress);  
                resourceParams.dwLocationInCmd = 74;                                                      
                resourceParams.bIsWritable     = true;                                                    
                                                                                                          
                cmd.VDENC_PIPE_BUF_ADDR_STATE_DW74_76.PictureFields.DW0.MemoryCompressionEnable  = 0;                 
                cmd.VDENC_PIPE_BUF_ADDR_STATE_DW74_76.PictureFields.DW0.CompressionType          = 0;
                InitMocsParams(resourceParams, &cmd.VDENC_PIPE_BUF_ADDR_STATE_DW74_76.PictureFields.DW0.Value, 1, 6);
                                                                                                          
                MHW_MI_CHK_STATUS(this->AddResourceToCmd(                                                 
                    this->m_osItf,                                                                        
                    this->m_currentCmdBuf,                                                                
                    &resourceParams));                                                                    
            }                                                                                             
        }

        return MOS_STATUS_SUCCESS;
    }
    _MHW_SETCMD_OVERRIDE_DECL(VDENC_AVC_IMG_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_AVC_IMG_STATE);

#define DO_FIELDS_EXT() \
        __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_AVC_IMG_STATE_IMPL_XE_LPM_BASE_EXT)

#include "mhw_hwcmd_process_cmdfields.h"
    }

MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__xe_lpm_plus_base__BaseImplGeneric)
};
}  // namespace xe_lpm_plus_base
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VDENC_IMPL_XE_LPM_PLUS_BASE_H__
