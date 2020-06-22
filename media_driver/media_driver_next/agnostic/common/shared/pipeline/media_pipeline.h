/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_pipeline.h
//! \brief    Defines the common interface for media pipeline
//! \details  The media pipeline interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __MEDIA_PIPELINE_H__
#define __MEDIA_PIPELINE_H__
#include <map>
#include "mos_defs.h"
#include "mos_os.h"
#include "media_task.h"
#include "media_context.h"
#include "codechal_debug.h"
#include "media_status_report.h"
#include "media_feature_manager.h"
#include "codechal_utilities.h"

class MediaPacket;
class MediaPipeline
{
public:
    //!
    //! \brief  Media pipeline constructor
    //! \param  [in] osInterface
    //!         Pointer to MOS_INTERFACE
    //!
    MediaPipeline(PMOS_INTERFACE osInterface);

    //!
    //! \brief  Media pipeline destructor
    //!
    virtual ~MediaPipeline();

    //!
    //! \brief  Initialize the media pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *settings) = 0;

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare(void *params) = 0;

    //!
    //! \brief  Finish the execution for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute() = 0;

    //!
    //! \brief  Get media pipeline execution status
    //! \param  [out] status
    //!         The point to encode status
    //! \param  [in] numStatus
    //!         The requested number of status reports
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetStatusReport(void *status, uint16_t numStatus) = 0;

    //!
    //! \brief  Destory the media pipeline and release internal resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() = 0;

    //!
    //! \brief  Delete the packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DeletePackets();

    //!
    //! \brief  Delete the tasks
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DeleteTasks();

    MediaStatusReport* GetStatusReportInstance() { return m_statusReport; }

    MediaContext *GetMediaContext() { return m_mediaContext; }
    virtual MediaFeatureManager *GetFeatureManager() { return m_featureManager; };
    MediaScalability* &GetMediaScalability() { return m_scalability; }
    //!
    //! \brief  Get if frame tracking is enabled from scalability
    //! \return bool
    //!         true if enabled, else false
    //!
    bool IsFrameTrackingEnabled();

protected:
    //!
    //! \brief  User Feature Key Report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureReport();

    //!
    //! \brief  Initialize the platform infos
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitPlatform();

    //!
    //! \brief  Register packets into packet pool
    //! \param  [in] packetId
    //!         Packet Id
    //! \param  [in] packet
    //!         Pointer to created packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RegisterPacket(uint32_t packetId, MediaPacket* packet);

    //!
    //! \brief  Retrieve the task with given Id
    //! \param  [in] taskId
    //!         Task Id
    //! \return MediaTask*
    //!         Pointer to media task if success, else nullptr
    //!
    MediaTask* GetTask(MediaTask::TaskType type);

    //!
    //! \brief  Activate packet and add it to active packet list
    //! \param  [in] packetId
    //!         Packet Id
    //! \param  [in] immediateSubmit
    //!         Indicate if this packet to activate is needed to submit immediately after been added to task
    //! \param  [in] pass
    //!         pass belongs to the Packet
    //! \param  [in] pipe
    //!         pipe belongs to the Packet
    //! \param  [in] pipe numbers
    //!         pipe numbers the Packet needs to use
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ActivatePacket(uint32_t packetId, bool immediateSubmit, uint8_t pass, uint8_t pipe, uint8_t pipeNum = 1, uint8_t subPass = 0, uint8_t rowNum = 0);

    //!
    //! \brief  Finish the active packets execution
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteActivePackets();

    //!
    //! \brief  Create task and add into packet pool
    //! \param  [in] type
    //!         Task type
    //! \return MediaTask*
    //!         pointer to MediaTask
    //!
    MediaTask* CreateTask(MediaTask::TaskType type);

    //!
    //! \brief  create media feature manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateFeatureManager();

protected:
    PMOS_INTERFACE                   m_osInterface = nullptr;      //!< OS interface
    CodechalDebugInterface           *m_debugInterface = nullptr;  //!< Interface used for debug dumps
    PLATFORM                         m_platform = {};              //!< The platorm info
    MEDIA_FEATURE_TABLE              *m_skuTable     = nullptr;    //!< SKU table
    MEDIA_WA_TABLE                   *m_waTable      = nullptr;    //!< WA table
    MEDIA_SYSTEM_INFO                *m_gtSystemInfo = nullptr;    //!< GT system infomation
 
    MediaScalability    *m_scalability = nullptr;
    MediaContext        *m_mediaContext = nullptr;
    MediaStatusReport   *m_statusReport = nullptr;
    MediaFeatureManager *m_featureManager = nullptr;

    std::map<uint32_t, MediaPacket *>               m_packetList;        //!< Packets list
    std::vector<PacketProperty>               m_activePacketList;  //!< Active packets property list
    std::map<MediaTask::TaskType, MediaTask *>      m_taskList;          //!< Task list

};
#endif // !__MEDIA_PIPELINE_H__
