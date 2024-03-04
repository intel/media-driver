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
//! \file     vp_surface_cache_settings.h
//! \brief    vp surfaces cache settings
//! \details  Detaied vp cache settings
//!
#ifndef __VP_SURFACE_CACHE_SETTINGS_H__
#define __VP_SURFACE_CACHE_SETTINGS_H__
/****************************************************************************\
  Cache Setting Table for Common Surfaces

MOS_CACHE_OBJECT(MOS_COMPONENT _componentId, uint32_t _surfaceType, bool _isHeap, bool _isOutput, ENGINE_TYPE _engineType), MOS_CACHE_ELEMENT(MOS_HW_RESOURCE_DEF _mocsUsageType, MOS_HW_RESOURCE_DEF _patIndex)
      patIndex ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------o 
      mocsUsageType -----------------------------------------------------------------------------------------------------------------------------------------------o                                               |
      engineType --------------------------------------------------------------------------------------------------------o                                         |                                               |
      isOutput -----------------------------------------------------------------------------------------------o          |                                         |                                               |
      isHeap ------------------------------------------------------------------------------------------o      |          |                                         |                                               |
      surfaceType --------------------------------------o                                              |      |          |                                         |                                               |
      componentId        |                              |                                              |      |          |                                         |                                               |
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
{MOS_CACHE_OBJECT(COMPONENT_VPCommon, (uint32_t)SUFACE_TYPE_ASSIGNED(vp::SurfaceTypeRenderOutput), false, true, RENDER_ENGINE).value, MOS_CACHE_ELEMENT(MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER, MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER)},
{MOS_CACHE_OBJECT(COMPONENT_VPCommon, (uint32_t)SUFACE_TYPE_ASSIGNED(vp::SurfaceTypeRenderOutput), false, false, RENDER_ENGINE).value, MOS_CACHE_ELEMENT(MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_RENDER, MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER)},
{MOS_CACHE_OBJECT(COMPONENT_VPCommon, (uint32_t)ISH_HEAP, true, false, RENDER_ENGINE).value, MOS_CACHE_ELEMENT(MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_RENDER, MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_RENDER)},
{MOS_CACHE_OBJECT(COMPONENT_VPCommon, (uint32_t)GSH_HEAP, true, false, RENDER_ENGINE).value, MOS_CACHE_ELEMENT(MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_RENDER, MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_RENDER)},
{MOS_CACHE_OBJECT(COMPONENT_VPCommon, (uint32_t)GSH_HEAP, true, false, RENDER_ENGINE).value, MOS_CACHE_ELEMENT(MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_RENDER, MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_RENDER)},
#endif  // __VP_SURFACE_CACHE_SETTINGS_H__