/*
* Copyright (c) 2009-2023, Intel Corporation
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
//! \file     mos_util_user_feature_keys.h
//! \brief    Definition of user feature keys
//! \details  Definition of user feature keys
//!           including Codec, VP, and MOS user feature keys
//!

#ifndef __MOS_UTIL_USER_FEATURE_KEYS_H__
#define __MOS_UTIL_USER_FEATURE_KEYS_H__

#include "mos_defs.h"
#include "mos_util_user_feature_keys_specific.h"

#define __MEDIA_USER_FEATURE_VALUE_DECODE_LOCK_DISABLE                  "Disable Decode Lock"

//!
//! \brief      User feature key for enabling MemNinja Release feature that uses counter value to determine mem leak
//! \details    1. MemNinja Counter - Driver reports the sum of internal counters MosMemAllocCounter and 
//!             MosMemAllocCounterGfx when test completes to this User feature key. Test application checks this value.
//!             If MemNinjaCounter != 0, test app can flag test as fail.
//!
#define __MEDIA_USER_FEATURE_VALUE_MEMNINJA_COUNTER                     "MemNinja Counter"

//!
//! \brief      User feature key to override the number of Slices/Sub-slices/EUs to suhutdown
//! \details    Same setting will apply to all command buffer submissions
//!             Byte0 is for num Slices. Byte1 is for num Sub-slices. Bytes 2 and 3 are for num EUs
//!             31________________________________16_15______________8_7____________0
//!             |                                   |                 |             |
//!             |              Num EUs              |  Num Sub-Slices | Num Slices  |
//!             |___________________________________|_________________|_____________|
//!
#define __MEDIA_USER_FEATURE_VALUE_SSEU_SETTING_OVERRIDE                    "SSEU Setting Override"

//!
//! \brief Keys for Media Processing
//!
#define __MEDIA_USER_FEATURE_VALUE_APOGEIOS_ENABLE                      "ApogeiosEnable"
#define __MEDIA_USER_FEATURE_VALUE_VDI_MODE                             "VDI Mode"
#define __MEDIA_USER_FEATURE_VALUE_MEDIA_WALKER_MODE                    "Media Walker Mode"
#define __MEDIA_USER_FEATURE_VALUE_RA_MODE_ENABLE                       "RA Mode Enable"
#define __MEDIA_USER_FEATURE_VALUE_PROTECT_MODE_ENABLE                  "Protect Mode Enable"
#define __MEDIA_USER_FEATURE_VALUE_ENABLE_HCP_SCALABILITY_DECODE        "Enable HCP Scalability Decode"
#define __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE        "Enable Vebox Scalability"

#if (_DEBUG || _RELEASE_INTERNAL)

//!
//! \brief Dump surface for MediaCopy in/out
//!
#define __COMMON_DBG_SURF_DUMPER_RESOURCE_LOCK                          "McpySurfaceDumperResourceLockError"
#define __COMMON_DBG_SURF_DUMP_OUTFILE_KEY_NAME                         "McpyOutfileLocation"
#define __COMMON_DBG_DUMP_OUTPUT_DIRECTORY                              "Common Debug Dump Output Directory"
#define __COMMON_DBG_SURF_DUMP_LOCATION_KEY_NAME_IN                     "dumpLocation before MCPY"
#define __COMMON_DBG_SURF_DUMP_LOCATION_KEY_NAME_OUT                    "dumpLocation after MCPY"

#define __MEDIA_USER_FEATURE_VALUE_MEDIA_RESET_ENABLE                   "Media Reset"
#define __MEDIA_USER_FEATURE_VALUE_FORCE_RESET_THRESHOLD                "Force media reset threshold"
#define __MEDIA_USER_FEATURE_VALUE_FORCE_MEDIA_COMPRESSED_WRITE         "Force Media Compressed Write"

//!
//! \brief Keys for media
//!
#define __MEDIA_USER_FEATURE_VALUE_MEDIA_PREEMPTION_ENABLE              "Media Preemption Enable"
#define __MEDIA_USER_FEATURE_VALUE_MEDIA_RESET_TH                       "Media Reset TH"
#define __MEDIA_USER_FEATURE_VALUE_MEDIA_DEBUG_CFG_GENERATION           "Media Debug Cfg Generation"
#define __MEDIA_USER_FEATURE_MCPY_MODE                                  "MediaCopy Mode"
#define __MEDIA_USER_FEATURE_VALUE_VEBOX_SPLIT_RATIO                    "Vebox Split Ratio"
#define __MEDIA_USER_FEATURE_SET_MCPY_FORCE_MODE                        "MCPY Force Mode"
#define __MEDIA_USER_FEATURE_ENABLE_VECOPY_SMALL_RESOLUTION             "Enable VE copy small resolution"  // resolution smaller than 64x32

//!
//! \brief Keys for mmc
//!
#define __MEDIA_USER_FEATURE_ENABLE_RENDER_ENGINE_MMC                   "Enable Media RenderEngine MMC"

//!
//! \brief ISA ASM Debug Enable and Debug Surface BTI
//!
#define __MEDIA_USER_FEATURE_VALUE_ISA_ASM_DEBUG_ENABLE                 "IsaAsm Debug Enable"
#define __MEDIA_USER_FEATURE_VALUE_ISA_ASM_DEBUG_SURF_BTI               "IsaAsm Debug Surf BT Index"

//!
//! \brief MediaSolo user feature keys
//!
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_ENABLE                             "MediaSolo Enable"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_PLATFORM                           "MediaSolo Platform"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_STEPPING                           "MediaSolo Stepping"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_ENABLE_MEM_TRACE                   "MediaSolo Enable Mem Trace"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_ENABLE_LOCAL_MEM                   "MediaSolo Enable Local Mem"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_AUBLOAD_DIRECTORY                  "MediaSolo AubLoad Directory"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_OUTPUT_DIRECTORY                   "MediaSolo Output Directory"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_GT_SKU                             "MediaSolo GT SKU"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_DEBUG_OUTPUT_ENABLE                "MediaSolo Debug Output Enable"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_AUBCAPTURE_ENABLE                  "MediaSolo AubCapture Enable"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_PATHLIST_ENABLE                    "MediaSolo Patch List Enable"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_RESET_GFXADDR_PER_CONTEXT_DISABLE  "MediaSolo Reset GfxAddr Per Context Disable"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_MMIO_GT_SETTING                    "MediaSolo PAVPC MMIO GT Setting"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_PERF_TEST_ENABLE                   "MediaSolo Perf Test Enable"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_AUBLOAD_COMMAND_LINE_OPTIONS       "MediaSolo AubLoad Command Line Options"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_VE_SCHEDULE_FIXED_MODE_ENABLE      "MediaSolo VE Schedule Fixed Mode Enable"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_AUBCAPTURE_BMP_DUMP_ENABLE         "MediaSolo AubCapture BMP Dump Enable"
#define __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_AUBCAPTURE_RECON_DUMP_ENABLE       "MediaSolo AubCapture Recon Dump Enable"

//!
//! \brief enable Huc based DRM for CHV
//!
#define __MEDIA_USER_FEATURE_VALUE_HUC_DRM_ENABLE                         "HuC DRM Enable"                 //!< 0: Disable, Others: enable

#define __MEDIA_USER_FEATURE_VALUE_MEMORY_NINJA_BEGIN_COUNTER             "MemNinjaBeginCounter"
#define __MEDIA_USER_FEATURE_VALUE_MEMORY_NINJA_END_COUNTER               "MemNinjaEndCounter"

#define __MEDIA_USER_FEATURE_VALUE_SIM_ENABLE                             "Simulation Enable"
#define __MEDIA_USER_FEATURE_VALUE_SIM_IN_USE                             "Simulation In Use"

#define __MEDIA_USER_FEATURE_VALUE_ENABLE_VE_DEBUG_OVERRIDE               "Enable VE Debug Override"
#define __MEDIA_USER_FEATURE_VALUE_FORCE_VDBOX                            "Force VDBOX"
#define __MEDIA_USER_FEATURE_VALUE_FORCE_VEBOX                            "Force VEBOX"
#define __MEDIA_USER_FEATURE_VALUE_FORCE_YFYS                             "Force to allocate YfYs"

#define __MEDIA_USER_FEATURE_VALUE_NULL_HW_ACCELERATION_ENABLE            "NullHWAccelerationEnable"

#define __MEDIA_USER_FEATURE_VALUE_ENABLE_LINUX_FRAME_SPLIT               "Enable Linux Frame Split"
#define __MEDIA_USER_FEATURE_VALUE_ENABLE_GUC_SUBMISSION                  "Enable Guc Submission"
#define __MEDIA_USER_FEATURE_VALUE_SOFT_RESET_ENABLE                      "Soft Reset"

#define __MEDIA_USER_FEATURE_VALUE_ENABLE_DECODE_VE_CTXSCHEDULING         "Enable Decode VE CtxBasedScheduling"
#define __MEDIA_USER_FEATURE_VALUE_ENABLE_DECODE_VIRTUAL_ENGINE           "Enable Decode VE"
#define __MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VIRTUAL_ENGINE           "Enable Encode VE"

#define __MEDIA_USER_FEATURE_VALUE_ENABLE_MEDIA_CCS                       "Enable Media CCS"

#endif // (_DEBUG || _RELEASE_INTERNAL)

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
#define __MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_BUFFER_ENABLE             "Dump Command Buffer Enable"
#endif

#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
#define __MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_INFO_ENABLE               "Dump Command Info Enable"
#define __MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_INFO_PATH                 "Dump Command Info Path"
#endif
//!
//! \brief User feature keys to define debug message levels and assertions.
//!

#if MOS_MESSAGES_ENABLED

//!
//! \brief Message level and assert flag for each component and its sub-components.
//!         For each component, prints and asserts can be enabled/disabled by a single key (3 bits for level, 1 bit for assert).
//!         The second key determines behavior of sub-components (3 bits for level, 1 bit for assert).
//!         A message will be printed iff
//!            1. "Message Print Enabled" is on,
//!            2. The component level is right and
//!            3. The sub-component level is right.
//!         An assert will trigger iff asserts are enabled both for component and sub-component.

#define __MOS_USER_FEATURE_KEY_MESSAGE_HLT_ENABLED                  "Message HLT Enabled"
#define __MOS_USER_FEATURE_KEY_MESSAGE_HLT_OUTPUT_DIRECTORY         "Message HLT Output Directory"
#define __MOS_USER_FEATURE_KEY_MESSAGE_PRINT_ENABLED                "Message Print Enabled"
#define __MOS_USER_FEATURE_KEY_FLUSH_LOG_FILE_BEFORE_SUBMISSION     "Flush Log File Before Submission"
#define __MOS_USER_FEATURE_KEY_ENABLE_MEMORY_FOOT_PRINT             "Enable Memory Foot Print"

//!
//! \brief Message level and assert flag for each component is set through the user feature keys
//!        "Mos Message Tags", "Codec Message Tags", "VP Message Tags",
//!        "CP Message Tags", "DDI Message Tags" and "CM Message Tags"
//!        3 bits for level, 1 bit for assert on/off per sub-component.
//!        Each component has to create a separate key for its sub-comps.
//!
//!        31____________________________________________________________________________3__________0
//!         |                                                                 |          |Asrt|level|
//!         |________________________________|__________|__________|__________|__________|__________|
//!

//!
//! \brief User feature keys for component MOS:
//!
#define __MOS_USER_FEATURE_KEY_MESSAGE_OS_TAG                       "Mos Message Tags"
#define __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_OS                  "Mos Tags By Sub Component"
//!
//! \brief 63____________________________________________________________________________3__________0
//!         |                                                                            |   Self   |
//!         |                    Reserved (HLT can be added as a sub-comp of MOS)        |Asrt|level|
//!         |____________________________________________________________________________|__________|
//!
#define __MOS_USER_FEATURE_KEY_SUB_COMPONENT_OS_TAG                 "Mos Sub Components Tags"

//!
//! \brief User feature keys for component MHW:
//!
#define __MOS_USER_FEATURE_KEY_MESSAGE_MHW_TAG                       "Mhw Message Tags"
#define __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MHW                  "Mhw Tags By Sub Component"
//!
//! \brief 63____________________________________________________________________________3__________0
//!         |                                                                            |   Self   |
//!         |                    Reserved (HLT can be added as a sub-comp of MOS)        |Asrt|level|
//!         |____________________________________________________________________________|__________|
//!
#define __MOS_USER_FEATURE_KEY_SUB_COMPONENT_MHW_TAG                 "Mhw Sub Components Tags"

//!
//! \brief User feature keys for component CODEC:
//!
#define __MOS_USER_FEATURE_KEY_MESSAGE_CODEC_TAG                    "Codec Message Tags"
#define __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CODEC               "Codec Tags By Sub Component"
//!
//! \brief Message level and assert flag for each sub-comp of CodecHal set through this user feature key
//!        3 bits for level, 1 bit for assert on/off per sub-component.
//!        Each component has to create a separate key for its sub-comps.
//!
//!        63___________________24_23______20_19______16_15______12_11_______8_7________4_3_________0
//!         |                     |  Debug   |  Public  |    HW    |  Encode  |  Decode  |    DDI   |
//!         |      Reserved       |Asrt|level|Asrt|level|Asrt|level|Asrt|level|Asrt|level|Asrt|level|
//!         |________________________________|__________|__________|__________|__________|__________|
//!
#define __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CODEC_TAG              "Codec Sub Components Tags"

//!
//! \brief User feature keys for component VP:
//!
#define __MOS_USER_FEATURE_KEY_MESSAGE_VP_TAG                       "VP Message Tags"
#define __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_VP                  "VP Tags By Sub Component"
//!
//! \brief Message level and assert flag for each sub-comp of VP set through this user feature key
//!        3 bits for level, 1 bit for assert on/off per sub-component.
//!        Each component has to create a separate key for its sub-comps.
//!
//!        63___________________24_23______20_19______16_15______12_11_______8_7________4_3_________0
//!         |                     | Reserved |  Render  |  Debug   |  Public  |    HW    |    DDI   |
//!         |      Reserved       |Asrt|level|Asrt|level|Asrt|level|Asrt|level|Asrt|level|Asrt|level|
//!         |________________________________|__________|__________|__________|__________|__________|
//!
#define __MOS_USER_FEATURE_KEY_SUB_COMPONENT_VP_TAG                 "VP Sub Components Tags"

//!
//! \brief User feature keys for component CP:
//!
#define __MOS_USER_FEATURE_KEY_MESSAGE_CP_TAG                       "CP Message Tags"
//! This can be 0/1, 1 is on, 0 is off
#define __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CP                  "CP Tags By Sub Component"
//!
//! \brief Message level and assert flag for each sub-comp of CP set through this user feature key
//!        3 bits for level, 1 bit for assert on/off per sub-component.
//!        Each component has to create a separate key for its sub-comps.
//!
//!     63_____48_47___44_43____40_39_______36_35_________32_31___28_27____24_23___20_19___16_15__12_11__8_7_________4_3_________0
//!     |        |  LIB  |  DLL   |AUTHCHANNEL|SECURESESSION|CMD_BFR|UMD_CTXT| CODEC |GPU_HAL|PCH_HAL| OS |  DEVICE   |CP_DDI    |
//!     |Reserved|  A|L  |   A|L  |    A|L    |     A|L     |  A|L  |  A|L   |  A|L  |  A|L  |  A|L  |A|L |Asrt|level |Asrt|level|
//!     |________|_______|________|___________|_____________|_______|________|_______|_______|_______|____|___________|__________|
//!
#define __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CP_TAG                 "CP Sub Components Tags"

//!
//! \brief User feature keys for component DDI:
//!
#define __MOS_USER_FEATURE_KEY_MESSAGE_DDI_TAG                      "DDI Message Tags"
#define __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_DDI                 "DDI Tags By Sub Component"
//!
//! \brief 63____________________________________________________________________________3__________0
//!         |                                                                            |   Self   |
//!         |                    Reserved (can create DDI subcomponents)                 |Asrt|level|
//!         |____________________________________________________________________________|__________|
//!
#define __MOS_USER_FEATURE_KEY_SUB_COMPONENT_DDI_TAG                "DDI Sub Components Tags"

//!
//! \brief User feature keys for component CM:
//!
#define __MOS_USER_FEATURE_KEY_MESSAGE_CM_TAG                       "CM Message Tags"
#define __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CM                  "CM Tags By Sub Component"
//!
//! \brief 63________________________________________________________________8_7________4_3_________0
//!         |                                                                 |   Self   |    DDI   |
//!         |                    Reserved (can create CM subcomponents)       |Asrt|level|Asrt|level|
//!         |____________________________________________________________________________|__________|
//!
#define __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CM_TAG                 "CM Sub Components Tags"

//!
//! \brief User feature keys for component SCALABILITY:
//!
#define __MOS_USER_FEATURE_KEY_MESSAGE_SCALABILITY_TAG              "SCALABILITY Message Tags"
#define __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_SCALABILITY         "SCALABILITY Tags By Sub Component"
//!
//! \brief 63____________________________________________________________________________3__________0
//!         |                                                                            |   Self   |
//!         |                    Reserved (can create SCALABILITY subcomponents)         |Asrt|level|
//!         |____________________________________________________________________________|__________|
//!
#define __MOS_USER_FEATURE_KEY_SUB_COMPONENT_SCALABILITY_TAG        "SCALABILITY Sub Components Tags"

//!
//! \brief User feature keys for component MMC:
//!
#define __MOS_USER_FEATURE_KEY_MESSAGE_MMC_TAG              "MMC Message Tags"
#define __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MMC         "MMC Tags By Sub Component"

//!
//! \brief 63____________________________________________________________________________3__________0
//!         |                                                                            |   Self   |
//!         |                    Reserved (can create MMC subcomponents)                 |Asrt|level|
//!         |____________________________________________________________________________|__________|
//!
#define __MOS_USER_FEATURE_KEY_SUB_COMPONENT_MMC_TAG        "MMC Sub Components Tags"

//!
//! \brief User feature keys for component BLT:
//!
#define __MOS_USER_FEATURE_KEY_MESSAGE_MCPY_TAG             "MCPY Message Tags"
#define __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MCPY        "MCPY Tags By Sub Component"
//!
//! \brief 63____________________________________________________________________________3__________0
//!         |                                                                            |   Self   |
//!         |                    Reserved (can create MCPY subcomponents)                |Asrt|level|
//!         |____________________________________________________________________________|__________|
//!
#define __MOS_USER_FEATURE_KEY_SUB_COMPONENT_MCPY_TAG       "MCPY Sub Components Tags"

#define __MOS_USER_FEATURE_KEY_DISABLE_ASSERT               "DisableAssert"

#endif // MOS_MESSAGES_ENABLED

//User feature key for MDF
#define __MEDIA_USER_FEATURE_VALUE_MDF_ETW_ENABLE                           "MDF ETW Enable"
#define __MEDIA_USER_FEATURE_VALUE_MDF_LOG_LEVEL                            "MDF LOG Level"
#define __MEDIA_USER_FEATURE_VALUE_MDF_UMD_ULT_ENABLE                       "MDF UMD ULT Enable"
#define __MEDIA_USER_FEATURE_VALUE_MDF_CMD_DUMP_ENABLE                      "MDF Command Buffer Dump Enable"
#define __MEDIA_USER_FEATURE_VALUE_MDF_CURBE_DUMP_ENABLE                    "MDF Curbe Dump Enable"
#define __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_DUMP_ENABLE                  "MDF Surface Dump Enable"
#define __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_STATE_DUMP_ENABLE            "MDF Surface State Dump Enable"
#define __MEDIA_USER_FEATURE_VALUE_MDF_EMU_MODE_ENABLE                      "MDF EMU Enable"
#define __MEDIA_USER_FEATURE_VALUE_MDF_CMD_DUMP_COUNTER                     "MDF CMD DUMP COUNTER"
#define __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_STATE_DUMP_COUNTER           "MDF SURFACE STATE DUMP COUNTER"
#define __MEDIA_USER_FEATURE_VALUE_MDF_INTERFACE_DESCRIPTOR_DATA_DUMP       "MDF Interface Descriptor Dump Enable"
#define __MEDIA_USER_FEATURE_VALUE_MDF_INTERFACE_DESCRIPTOR_DATA_COUNTER    "MDF Interface Descriptor Dump Counter"
#define __MEDIA_USER_FEATURE_VALUE_MDF_DUMPPATH_USER                        "MDF Dump Path Specified by User"
#define __MEDIA_USER_FEATURE_VALUE_MDF_FORCE_EXECUTION_PATH                 "MDF Execution Path Forced by User"
#define __MEDIA_USER_FEATURE_VALUE_MDF_MAX_THREAD_NUM                       "CmMaxThreads"
#define __MEDIA_USER_FEATURE_VALUE_MDF_FORCE_COHERENT_STATELESSBTI          "ForceCoherentStatelessBTI"

//User feature key for VP
#define __MEDIA_USER_FEATURE_VALUE_VP_3P_DUMP_UFKEY_LOCATION                "Software\\Intel\\VPPDPI"

#define __MOS_USER_FEATURE_KEY_XML_AUTOGEN              "XML AutoGen Enable"
#define __MOS_USER_FEATURE_KEY_XML_FILEPATH             "XML File Path"
#define __MOS_USER_FEATURE_KEY_XML_DUMP_GROUPS          "XML Dump Group"

#if (_DEBUG || _RELEASE_INTERNAL)
//User feature key for enable simulating random memory allocation failure
#define __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_MODE    "Alloc Memory Fail Simulate Mode"
#define __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_FREQ    "Alloc Memory Fail Simulate Freq"
#define __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_HINT    "Alloc Memory Fail Simulate Hint"
#define __MEDIA_USER_FEATURE_VALUE_OS_API_FAIL_SIMULATE_TYPE          "OS API Fail Simulate Type"
#define __MEDIA_USER_FEATURE_VALUE_OS_API_FAIL_SIMULATE_MODE          "OS API Fail Simulate Mode"
#define __MEDIA_USER_FEATURE_VALUE_OS_API_FAIL_SIMULATE_FREQ          "OS API Fail Simulate Freq"
#define __MEDIA_USER_FEATURE_VALUE_OS_API_FAIL_SIMULATE_HINT          "OS API Fail Simulate Hint"
#define __MEDIA_USER_FEATURE_VALUE_MEDIA_TILE_ENCODING_1_DEFAULT      "Media tile encoding as 1 by default"
#define __MEDIA_USER_FEATURE_VALUE_TILE_ENCODING_1_INTERNAL_USED      "Media Internal tile encoding as 1 used"
#define __MEDIA_USER_FEATURE_VALUE_TILE_ENCODING_3_INTERNAL_USED      "Media Internal tile encoding as 3 used"
//User feature key for vp surface dump
#define __VPHAL_DBG_SURF_DUMP_OUTFILE_KEY_NAME                        "outfileLocation"
#define __VPHAL_DBG_SURF_DUMP_LOCATION_KEY_NAME                       "dumpLocations"
#define __VPHAL_DBG_SURF_DUMP_MANUAL_TRIGGER_KEY_NAME                 "VphalSurfaceDumpManualTrigger"
#define __VPHAL_DBG_SURF_DUMP_START_FRAME_KEY_NAME                    "startFrame"
#define __VPHAL_DBG_SURF_DUMP_END_FRAME_KEY_NAME                      "endFrame"
#define __VPHAL_DBG_SURF_DUMPER_ENABLE_PLANE_DUMP                     "enablePlaneDump"
#define __VPHAL_DBG_SURF_DUMP_ENABLE_AUX_DUMP                         "enableAuxDump"
#define __VPHAL_DBG_SURF_DUMPER_RESOURCE_LOCK                         "SurfaceDumperResourceLockError"
#define __VPHAL_DBG_STATE_DUMP_ENABLE                                 "enableStateDump"
//User feature key for Codec debug
#define __MEDIA_USER_FEATURE_VALUE_CODECHAL_DEBUG_OUTPUT_DIRECTORY    "CodecHal Debug Output Directory"
#define __MEDIA_USER_FEATURE_VALUE_IS_CODEC_ROW_STORE_CACHE_ENABLED   "Codec Row Store Cache Enabled"

// !
// ! \brief User feature key for Split-Screen Demo Mode
// !
#define __MEDIA_USER_FEATURE_VALUE_SPLIT_SCREEN_DEMO_POSITION        "Split-Screen Demo Position"
#define __MEDIA_USER_FEATURE_VALUE_SPLIT_SCREEN_DEMO_PARAMETERS      "Split-Screen Demo Parameters"
#endif //(_DEBUG || _RELEASE_INTERNAL)

//User feature key for UMD_OCA
#define __MEDIA_USER_FEATURE_VALUE_ENABLE_UMD_OCA                               "Enable UMD_OCA"
//Error Report for UMD_OCA
#define __MEDIA_USER_FEATURE_VALUE_OCA_STATUS                                   "OCA Status"
#define __MEDIA_USER_FEATURE_VALUE_OCA_ERROR_HINT                               "OCA Error Hint"
#define __MEDIA_USER_FEATURE_VALUE_IS_INDIRECT_STATE_HEAP_INVALID               "Is Indirect State Heap Invalid"
#define __MEDIA_USER_FEATURE_VALUE_COUNT_FOR_OCA_BUFFER_LEAKED                  "Count For Oca Buffer Leaked"
#define __MEDIA_USER_FEATURE_VALUE_COUNT_FOR_OCA_1ST_LEVEL_BB_END_MISSED        "Count For Oca 1st Level BB End Missed"
#define __MEDIA_USER_FEATURE_VALUE_COUNT_FOR_ADDITIONAL_OCA_BUFFER_ALLOCATED    "Count For Additional Oca Buffer Allocated"

#define __VPHAL_ENABLE_VEBOX_MMC_DECOMPRESS                                     "Enable Vebox Decompress"

//User feature key for MMC
#define __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_ENABLE                             "Enable Codec MMC"
#define __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_IN_USE                             "Codec MMC In Use"

#define __VPHAL_ENABLE_MMC                                                      "Enable VP MMC"
#define __MEDIA_USER_FEATURE_VALUE_VP_MMC_IN_USE                                "VP MMC In Use"

#define __MEDIA_USER_FEATURE_VALUE_NULLHW_ENABLE                                "NULL HW Enable"
#if (_DEBUG || _RELEASE_INTERNAL)
#define __MEDIA_USER_FEATURE_VALUE_NULLHW_PROXY_REPEAT_COUNT                    "NULL HW Proxy Repeat Count"
#endif
#define __MEDIA_USER_FEATURE_VALUE_MOCKADAPTOR_PLATFORM                         "MockAdaptor Platform"
#define __MEDIA_USER_FEATURE_VALUE_MOCKADAPTOR_STEPPING                         "MockAdaptor Stepping"
#define __MEDIA_USER_FEATURE_VALUE_MOCKADAPTOR_DEVICE                           "MockAdaptor Device ID"

//User feature key for enable Perf Utility Tool
#define __MEDIA_USER_FEATURE_VALUE_PERF_UTILITY_TOOL_ENABLE          "Perf Utility Tool Enable"
#define __MEDIA_USER_FEATURE_VALUE_PERF_OUTPUT_DIRECTORY             "Perf Output Directory"

//User feature key for media perf profile
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_ENABLE              "Perf Profiler Enable"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_ENABLE_MUL_PROC     "Perf Profiler Multi Process Support"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_ENABLE_MER_HEADER   "Perf Profiler Merge by Header Support"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_OUTPUT_FILE_NAME    "Perf Profiler Output File Name"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_BUFFER_SIZE_KEY     "Perf Profiler Buffer Size"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_MUL_PROC_SINGLE_BIN "Perf Profiler Multi Process Single Binary"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_PARALLEL_EXEC       "Perf Profiler Parallel Execution Support"

#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_KEY_1      "Perf Profiler Register 1"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_KEY_2      "Perf Profiler Register 2"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_KEY_3      "Perf Profiler Register 3"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_KEY_4      "Perf Profiler Register 4"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_KEY_5      "Perf Profiler Register 5"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_KEY_6      "Perf Profiler Register 6"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_KEY_7      "Perf Profiler Register 7"
#define __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_KEY_8      "Perf Profiler Register 8"

//User feature key for Gmm pooled resource enabling
#define __MEDIA_USER_FEATURE_VALUE_MEDIA_TEXTURE_POOLING_ENABLE      "Enable Media Texture Pooling"

//Perf
#define __MEDIA_USER_FEATURE_VALUE_LINUX_PERFORMANCETAG_ENABLE       "Linux PerformanceTag Enable"

// Tile resource info report
#define __MEDIA_USER_FEATURE_VALUE_TILE_INFO                         "Tile Info"
#define __MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_RCS               "RCS Instance"
#define __MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_BLT               "BLT Instance"
#define __MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_VCS               "VCS Instance"
#define __MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_VECS              "VECS Instance"
#define __MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_CCS               "CCS Instance"

// IP alignment support
#define __MEDIA_USER_FEATURE_VALUE_IP_ALIGNMENT_DECODE_ENABLE        "EnableSyncSubmissionDecode"
#define __MEDIA_USER_FEATURE_VALUE_IP_ALIGNMENT_ENCODE_ENABLE        "EnableSyncSubmissionEncode"
#define __MEDIA_USER_FEATURE_VALUE_IP_ALIGNMENT_VPP_ENABLE           "EnableSyncSubmissionVPP"
#define __MEDIA_USER_FEATURE_VALUE_IP_ALIGNMENT_DECODE_TIMEOUT       "SyncSubmissionTimeOutDecode"
#define __MEDIA_USER_FEATURE_VALUE_IP_ALIGNMENT_ENCODE_TIMEOUT       "SyncSubmissionTimeOutEncode"
#define __MEDIA_USER_FEATURE_VALUE_IP_ALIGNMENT_VPP_TIMEOUT          "SyncSubmissionTimeOutVPP"

// Native Fence Mode
#define __MEDIA_USER_FEATURE_VALUE_MEDIA_NATIVE_FENCE_MODE           "Native Fence Mode"

#endif  // __MOS_UTIL_USER_FEATURE_KEYS_H__
