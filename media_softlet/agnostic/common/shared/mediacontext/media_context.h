/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     media_context.h
//! \brief    Defines the common interface for media context
//! \details  The media context interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __MEDIA_CONTEXT_H__
#define __MEDIA_CONTEXT_H__

#include "mos_os.h"
#include "media_scalability_factory.h"

struct _MHW_VDBOX_GPUNODE_LIMIT;
typedef struct _MHW_VDBOX_GPUNODE_LIMIT MHW_VDBOX_GPUNODE_LIMIT, *PMHW_VDBOX_GPUNODE_LIMIT;

enum MediaFunction
{
    RenderGenericFunc,
    VdboxDecodeFunc,
    VdboxEncodeFunc,
    VdboxCpFunc,
    VeboxVppFunc,
    ComputeMdfFunc,
    ComputeVppFunc,
    VdboxDecodeWaFunc,
    VdboxDecrpytFunc,
    VdboxDecodeVirtualNode0Func,
    VdboxDecodeVirtualNode1Func,
    MediaFuncMax
};
// Be compatible to legacy MOS and re-define the name
#define INVALID_MEDIA_FUNCTION MediaFuncMax
#define IS_RENDER_ENGINE_FUNCTION(func) ( func == RenderGenericFunc || func == ComputeMdfFunc || func == ComputeVppFunc )

struct GpuContextAttribute
{
    MediaFunction      func                = INVALID_MEDIA_FUNCTION;
    MediaScalability * scalabilityState    = nullptr;

    MOS_GPU_CONTEXT    ctxForLegacyMos     = MOS_GPU_CONTEXT_MAX;
    GPU_CONTEXT_HANDLE gpuContext          = MOS_GPU_CONTEXT_INVALID_HANDLE;
};

class MediaContext
{
public:
    //!
    //! \brief  Media context constructor
    //!
    MediaContext(uint8_t componentType, void *hwInterface, PMOS_INTERFACE osInterface);

    //!
    //! \brief  Media context destructor
    //!
    virtual ~MediaContext();

    //!
    //! \brief  Interface to pipeline to switch media context and get corresponding scalabilityState for programming
    //! \param  [in] func
    //!         Indicate the media function of the context to switch
    //! \param  [in] requirement
    //!         Pointer to the context requirement 
    //! \param  [out] scalabilityState
    //!         Pointer to the pointer of output scalability state.
    //!         Pipeline can only use this scalabilityState until next time calling SwitchContext
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SwitchContext(MediaFunction func, ContextRequirement *requirement, MediaScalability **scalabilityState);

    //!
    //! \brief  Interface to pipeline to switch media context and get corresponding scalabilityState for programming
    //! \param  [in] func
    //!         Indicate the media function of the context to switch
    //! \param  [in] scalabilityOption
    //!         The intialzed scalability option  
    //! \param  [out] scalabilityState
    //!         Pointer to the pointer of output scalability state.
    //!         Pipeline can only use this scalabilityState until next time calling SwitchContext
    //! \param  [in] isEnc
    //!         Indicate if this is Enc stage
    //! \param  [in] isPak
    //!         Indicate if this is Pak stage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SwitchContext(MediaFunction func, MediaScalabilityOption &scalabilityOption, MediaScalability **scalabilityState,
                             bool isEnc = false, bool isPak = false);

    //!
    //! \brief  Check if in current media context render engine is used
    //! \return bool
    //!         True if render engine used, else false.
    //!
    bool IsRenderEngineUsed()
    {
        auto gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);
        // Be compatiable to legacy MOS
        return MOS_RCS_ENGINE_USED(gpuContext);
    }

    uint8_t GetNumVdbox()
    {
        m_numVdbox                      = 1;
        MEDIA_SYSTEM_INFO *gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
        if (gtSystemInfo != nullptr)
        {
            // Both VE mode and media solo mode should be able to get the VDBOX number via the same interface
            m_numVdbox = (uint8_t)(gtSystemInfo->VDBoxInfo.NumberOfVDBoxEnabled);
        }
        return m_numVdbox;
    }

    //!
    //! \brief  Set latest decoder virtual node
    //!
    void SetLatestDecoderVirtualNode();

    //!
    //! \brief  Reassgine virtual node for decoder
    //! \param  [in] frameNum
    //!         The current decoding frame number
    //! \param  [in] scalabilityOption
    //!         The intialzed scalability option
    //! \param  [out] scalabilityState
    //!         Pointer to the pointer of output scalability state.
    //!         Pipeline can only use this scalabilityState until next time calling SwitchContext
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReassignContextForDecoder(uint32_t frameNum, MediaScalabilityOption &scalabilityOption, MediaScalability **scalabilityState);

    //!
    //! \brief  Reassgine virtual node for decoder
    //! \param  [in] frameNum
    //!         The current decoding frame number
    //! \param  [in] scalabilityOption
    //!         The intialzed scalability option
    //! \param  [out] scalabilityState
    //!         Pointer to the pointer of output scalability state.
    //!         Pipeline can only use this scalabilityState until next time calling SwitchContext
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReassignContextForDecoder(uint32_t frameNum, ContextRequirement *requirement, MediaScalability **scalabilityState);

protected:
    PMOS_INTERFACE                    m_osInterface             = nullptr;           //!< OS interface
    void                             *m_hwInterface             = nullptr;           //!< HW interface
    uint8_t                           m_componentType           = scalabilityTotal;  //!< Media component
    uint32_t                          m_streamId                = m_invalidStreamId; //!< Stream id of this media context

    std::vector<GpuContextAttribute>  m_gpuContextAttributeTable;                    //!< Gpu Context Attribute Table to store the contexts to reuse

    static const uint32_t             m_invalidContextAttribute = 0xffffffdf;        //!< Index value to indicate invalid Context Attribute
    static const uint32_t             m_invalidStreamId         = 0xffffffcb;        //!< Id to indicate invalid Stream
    static const uint32_t             m_maxContextAttribute     = 4096;              //!< Max number of entries supported in gpuContextAttributeTable in one media context
    uint8_t                           m_numVdbox                = 1;
    bool                              m_scalabilitySupported    = false;
    MOS_GPU_NODE                      m_curNodeOrdinal          = MOS_GPU_NODE_MAX;  //!< Current virtual node for codec gpu context
    MediaUserSettingSharedPtr         m_userSettingPtr          = nullptr;           //!< Shared pointer to user setting instance

    //!
    //! \brief  Search the ContextAttributeTable to reuse or create gpu Context and scalabilty state meeting the requirements
    //! \param  [in] func
    //!         Indicate the media function of the context
    //! \param  [in] params
    //!         The params for context searching
    //! \param  [out] indexFound
    //!         The index of the ContextAttributeTable which is found to match the requirements
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    template<typename T>
    MOS_STATUS SearchContext(MediaFunction func, T params, uint32_t& indexFound);

    //!
    //! \brief  Creat the context for new requirement
    //! \param  [in] func
    //!         Indicate the media function of the context
    //! \param  [in] params
    //!         Pointer to the context requirement 
    //! \param  [out] indexReturn
    //!         Return index for new ContextAttributeTable
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    template<typename T>
    MOS_STATUS CreateContext(MediaFunction func, T params, uint32_t& indexReturn);

    //!
    //! \brief  Get the GPU node of the Media Function
    //! \detail Usually there is a corresponding GPU node to implement the specific media function
    //! \param  [in] func
    //!         Media function to get GPU node
    //! \param  [in] option
    //!         Media GPU create option
    //! \param  [out] node
    //!         GPU node of the media function
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FunctionToNode(MediaFunction func, const MOS_GPUCTX_CREATOPTIONS_ENHANCED &option, MOS_GPU_NODE& node);
    MOS_STATUS FunctionToNodeCodec(MediaFunction func, MOS_GPU_NODE &node);
    MOS_STATUS FindGpuNodeToUse(MediaFunction func, PMHW_VDBOX_GPUNODE_LIMIT gpuNodeLimit);

    // Be compatible to Legacy MOS
    MOS_STATUS FunctionToGpuContext(MediaFunction func, const MOS_GPUCTX_CREATOPTIONS_ENHANCED &option, const MOS_GPU_NODE &node, MOS_GPU_CONTEXT &ctx);
    MOS_STATUS FunctionToGpuContextDecode(const MOS_GPUCTX_CREATOPTIONS_ENHANCED &option, const MOS_GPU_NODE &node, MOS_GPU_CONTEXT &ctx);
    MOS_STATUS FunctionToGpuContextEncode(const MOS_GPUCTX_CREATOPTIONS_ENHANCED &option, MOS_GPU_CONTEXT &ctx);

    #if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS CheckScalabilityOverrideValidity();
    #endif

MEDIA_CLASS_DEFINE_END(MediaContext)
};
#endif  // !__MEDIA_CONTEXT_H__
