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
//! \file     vp_packet_reuse_manager.h
//! \brief    Defines the classes for vp packet reuse
//!           this file is for the base interface which is shared by all features.
//!

#ifndef __VP_PACKET_REUSE_MANAGER_H__
#define __VP_PACKET_REUSE_MANAGER_H__

#include "mos_defs.h"
#include "media_class_trace.h"
#include "sw_filter.h"
#include "vp_packet_pipe.h"

namespace vp
{

class VpCmdPacket;
class PacketPipeFactory;
class Policy;
class PacketPipe;
class SwFilterPipe;
class VpResourceManager;

class VpFeatureReuseBase
{
public:
    virtual ~VpFeatureReuseBase();
    virtual MOS_STATUS UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter);
    virtual MOS_STATUS UpdatePacket(SwFilter *filter, VpCmdPacket *packet);
    virtual MOS_STATUS CheckTeamsParams(bool reusable, bool &reused, SwFilter *filter, uint32_t index);
    virtual MOS_STATUS StoreTeamsParams(SwFilter *filter, uint32_t index);

    MOS_STATUS HandleNullSwFilter(bool reusableOfLastPipe, bool &isPacketPipeReused, SwFilter *filter, bool &ignoreUpdateFeatureParams)
    {
        if (filter == nullptr)
        {
            if (!reusableOfLastPipe || m_paramsAvailable)
            {
                isPacketPipeReused = false;
            }
            m_paramsAvailable = false;
            ignoreUpdateFeatureParams = true;
        }
        else if (reusableOfLastPipe && m_paramsAvailable)
        {
            m_paramsAvailable = true;
        }
        else
        {
            isPacketPipeReused = false;
            m_paramsAvailable  = true;
        }

        return MOS_STATUS_SUCCESS;
    }

protected:
    bool m_paramsAvailable = true;
    MEDIA_CLASS_DEFINE_END(vp__VpFeatureReuseBase)
};

class VpScalingReuse : public VpFeatureReuseBase
{
public:
    VpScalingReuse();
    virtual ~VpScalingReuse();

    MOS_STATUS UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter);

    MOS_STATUS UpdatePacket(SwFilter *filter, VpCmdPacket *packet);

    MOS_STATUS CheckTeamsParams(bool reusable, bool &reused, SwFilter *filter, uint32_t index);

    MOS_STATUS StoreTeamsParams(SwFilter *filter, uint32_t index);

protected:
    MOS_STATUS UpdateFeatureParams(FeatureParamScaling &params);

    FeatureParamScaling         m_params          = {};
    VPHAL_COLORFILL_PARAMS      m_colorFillParams = {};     //!< ColorFill - BG only
    VPHAL_ALPHA_PARAMS          m_compAlpha       = {};     //!< Alpha for composited surfaces
    std::map<uint32_t, FeatureParamScaling> m_params_Teams;

MEDIA_CLASS_DEFINE_END(vp__VpScalingReuse)
};

class VpCscReuse : public VpFeatureReuseBase
{
public:
    VpCscReuse();
    virtual ~VpCscReuse() ;

    MOS_STATUS UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter);

    MOS_STATUS UpdatePacket(SwFilter *filter, VpCmdPacket *packet);

    MOS_STATUS CheckTeamsParams(bool reusable, bool &reused, SwFilter *filter, uint32_t index);

    MOS_STATUS StoreTeamsParams(SwFilter *filter, uint32_t index);

protected:
    MOS_STATUS UpdateFeatureParams(FeatureParamCsc &params);

    FeatureParamCsc             m_params            = {};
    VPHAL_ALPHA_PARAMS          m_alphaParams       = {};     //!< Alpha for composited surfaces
    VPHAL_IEF_PARAMS            m_iefParams         = {};
    std::map<uint32_t, FeatureParamCsc> m_params_Teams;

MEDIA_CLASS_DEFINE_END(vp__VpCscReuse)
};

class VpRotMirReuse : public VpFeatureReuseBase
{
public:
    VpRotMirReuse();
    virtual ~VpRotMirReuse();
    MOS_STATUS UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter);

    MOS_STATUS UpdatePacket(SwFilter *filter, VpCmdPacket *packet);

    MOS_STATUS CheckTeamsParams(bool reusable, bool &reused, SwFilter *filter, uint32_t index);

    MOS_STATUS StoreTeamsParams(SwFilter *filter, uint32_t index);

protected:
    MOS_STATUS UpdateFeatureParams(FeatureParamRotMir &params);
    FeatureParamRotMir m_params = {};
    std::map<uint32_t, FeatureParamRotMir> m_params_Teams;

MEDIA_CLASS_DEFINE_END(vp__VpRotMirReuse)
};

class VpColorFillReuse : public VpFeatureReuseBase
{
public:
    VpColorFillReuse();
    virtual ~VpColorFillReuse();
    MOS_STATUS UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter);
    MOS_STATUS UpdatePacket(SwFilter *filter, VpCmdPacket *packet);
protected:
    MOS_STATUS UpdateFeatureParams(FeatureParamColorFill &params);

    FeatureParamColorFill m_params = {};
    VPHAL_COLORFILL_PARAMS m_colorFillParams = {};

MEDIA_CLASS_DEFINE_END(vp__VpColorFillReuse)
};

class VpAlphaReuse : public VpFeatureReuseBase
{
public:
    VpAlphaReuse();
    virtual ~VpAlphaReuse();
    MOS_STATUS UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter);
    MOS_STATUS UpdatePacket(SwFilter *filter, VpCmdPacket *packet);

protected:
    MOS_STATUS UpdateFeatureParams(FeatureParamAlpha &params);

    FeatureParamAlpha m_params = {};
    VPHAL_ALPHA_PARAMS m_compAlpha = {};

MEDIA_CLASS_DEFINE_END(vp__VpAlphaReuse)
};

class VpDenoiseReuse : public VpFeatureReuseBase
{
public:
    VpDenoiseReuse();
    virtual ~VpDenoiseReuse();
    MOS_STATUS UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter);
    MOS_STATUS UpdatePacket(SwFilter *filter, VpCmdPacket *packet);

protected:
    MOS_STATUS UpdateFeatureParams(FeatureParamDenoise &params);

    FeatureParamDenoise m_params = {};

    MEDIA_CLASS_DEFINE_END(vp__VpDenoiseReuse)
};

class VpTccReuse : public VpFeatureReuseBase
{
public:
    VpTccReuse();
    virtual ~VpTccReuse();
    MOS_STATUS UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter);
    MOS_STATUS UpdatePacket(SwFilter *filter, VpCmdPacket *packet);

protected:
    MOS_STATUS UpdateFeatureParams(FeatureParamTcc &params);

    FeatureParamTcc m_params = {};

MEDIA_CLASS_DEFINE_END(vp__VpTccReuse)
};

class VpSteReuse : public VpFeatureReuseBase
{
public:
    VpSteReuse();
    virtual ~VpSteReuse();
    MOS_STATUS UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter);
    MOS_STATUS UpdatePacket(SwFilter *filter, VpCmdPacket *packet);

protected:
    MOS_STATUS UpdateFeatureParams(FeatureParamSte &params);

    FeatureParamSte m_params = {};

MEDIA_CLASS_DEFINE_END(vp__VpSteReuse)
};

class VpProcampReuse : public VpFeatureReuseBase
{
public:
    VpProcampReuse();
    virtual ~VpProcampReuse();
    MOS_STATUS UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter);
    MOS_STATUS UpdatePacket(SwFilter *filter, VpCmdPacket *packet);

protected:
    MOS_STATUS UpdateFeatureParams(FeatureParamProcamp &params);

    FeatureParamProcamp  m_params = {};
    VPHAL_PROCAMP_PARAMS m_procampParams = {};

    MEDIA_CLASS_DEFINE_END(vp__VpProcampReuse)
};

class VpPacketReuseManager
{
public:
    VpPacketReuseManager(PacketPipeFactory &packetPipeFactory, VpUserFeatureControl &userFeatureControl);
    virtual ~VpPacketReuseManager();
    virtual MOS_STATUS RegisterFeatures();
    MOS_STATUS PreparePacketPipeReuse(SwFilterPipe *&swFilterPipes, Policy &policy, VpResourceManager &resMgr, bool &isPacketPipeReused, bool &isTeamsWL);
    // Be called for not reused case before packet pipe execution.
    MOS_STATUS UpdatePacketPipeConfig(PacketPipe *&pipe);
    PacketPipe *GetPacketPipeReused()
    {
        return m_pipeReused;
    }

protected: 
    void ReturnPacketPipeReused();

protected:
    bool m_reusable = false;    // Current parameter can be reused.
    PacketPipe *m_pipeReused = nullptr;
    std::map<FeatureType, VpFeatureReuseBase *> m_features;
    PacketPipeFactory &m_packetPipeFactory;
    bool m_disablePacketReuse = false;
    uint32_t curIndex = 0;
    uint32_t MaxTeamsPacketSize = 16;    // max 16 Teams Packet stored
    bool m_TeamsPacket = false;
    bool m_TeamsPacket_reuse = false;
    bool m_enablePacketReuseTeamsAlways = false;
    std::map<uint32_t, PacketPipe *> m_pipeReused_TeamsPacket;
MEDIA_CLASS_DEFINE_END(vp__VpPacketReuseManager)
};

}  // namespace vp

#endif
