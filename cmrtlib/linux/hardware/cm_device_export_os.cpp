/*
* Copyright (c) 2017, Intel Corporation
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

#include "cm_device.h"
#include "cm_timer.h"

//!
//! \brief      Creates CM Device for hardware mode in linux
//! \details    Creates a CmDevice from scratch or creates a CmDevice based on the input 
//!             vaDisplay interface. If CmDevice is created from scratch, an 
//!             internal vaDisplay interface will be created by the runtime. 
//!             Creation of more than one CmDevice for concurrent use is supported. 
//!             The CM API version supported by the library will be returned
//!             in parameter version.
//! \param      [out] device
//!             Reference to the pointer to the CmDevice to be created
//! \param      [out] version
//!             Reference to CM API version supported by the runtime library
//! \param      [in] vaDisplay
//!             Reference to a given VADisplay from vaInitialize if NOT nullptr 
//! \retval     CM_SUCCESS if device successfully created
//! \retval     CM_OUT_OF_HOST_MEMORY if out of system memory
//! \retval     CM_INVALID_LIBVA_INITIALIZE if vaInitialize failed
//! \retval     CM_FAILURE otherwise
//!
extern "C"
CM_RT_API int32_t CreateCmDevice(CmDevice* &device, uint32_t & version, VADisplay vaDisplay = nullptr)
{
    INSERT_PROFILER_RECORD();

    CmDevice_RT* p=NULL;
    int32_t result = CM_FAILURE;

    if ( vaDisplay == nullptr)
    {
        result = CmDevice_RT::Create(p, CM_DEVICE_CREATE_OPTION_DEFAULT);
    }
    else
    {
        result = CmDevice_RT::Create(vaDisplay, p, CM_DEVICE_CREATE_OPTION_DEFAULT);
    }

    device = static_cast< CmDevice* >(p);
    if( result == CM_SUCCESS )
    {
        version = CURRENT_CM_VERSION;
    }
    else
    {
        version = 0;
    }

    return result;
}

//!
//! \brief      Creates CM Device according to user specified config for hardware mode in linux
//! \details    This API is supported from Cm 4.0. The definition of DevCreateOption is described 
//!             in below table. Scratch memory space is used to provide the memory for kernel's 
//!             spill code per thread. If there is no spill code exists in kernel, it is wise to 
//!             disable scratch space in device creation.The benefit is that video memory could 
//!             be saved and the time spent on device creation could be shortened.The default 
//!             scratch space size for HSW is 128K per hardware thread.If the kernel does not 
//!             need such big memory, it is recommended that programmer to specify the 
//!             scratch space size which kernel actually need by setting the bits[1:3].\n
//!       <table>
//!             <TR>
//!                    <TH> Usage </TH>
//!                    <TH> Bits </TH>
//!                    <TH> Length </TH>
//!                    <TH> Notes </TH>
//!             </TR>
//!                <TR>
//!                    <TD> Flag to disable scratch space </TD>
//!                    <TD> [0] </TD>
//!                    <TD> 1 </TD>
//!                    <TD> 0: enable(default) 1 : disable </TD>
//!             </TR>
//!                <TR>
//!                    <TD> Max scratch space size(HSW only) </TD>
//!                    <TD> [1:3] </TD>
//!                    <TD> 3 </TD>
//!                    <TD> if[1:3] > 0 MaxScratchSpaceSize = [1:3] * 16K; 
//!                         if[1:3] = 0 MaxScratchSpaceSize=128K(default); </TD>
//!             </TR>
//!                <TR>
//!                    <TD> Max Task Number </TD>
//!                    <TD> [4:5] </TD>
//!                    <TD> 2 </TD>
//!                    <TD> MaxTaskNumber = [4:5] * 4 + 4 </TD>
//!       </TR>
//!                <TR>
//!                    <TD> Reserved on Linux </TD>
//!                    <TD> [6] </TD>
//!                    <TD> 1 </TD>
//!                    <TD> N/A </TD>
//!       </TR>
//!                <TR>
//!                    <TD> media reset enable </TD>
//!                    <TD> [7] </TD>
//!                    <TD> 1   </TD>
//!                    <TD> 1: enable; 0: disable(default) </TD>
//!                </TR>
//!                <TR>
//!                    <TD> adding extra Task Number </TD>
//!                    <TD> [8:9] </TD>
//!                    <TD> 2 </TD>
//!                    <TD> extra task number = [4:5] * (Max task + 1) </TD>
//!       </TR>
//!                <TR>
//!                    <TD> slice shut down enable </TD>
//!                    <TD> [10] </TD>
//!                    <TD> 1   </TD>
//!                    <TD> 1: enable; 0: disble </TD>
//!                </TR>
//!                <TR>
//!                    <TD> surface reuse enable </TD>
//!                    <TD> [11] </TD>
//!                    <TD> 1   </TD>
//!                    <TD> 1: enable; 0: disble </TD>
//!                </TR>
//!                <TR>
//!                    <TD> GPU context enable </TD>
//!                    <TD> [12] </TD>
//!                    <TD> 1    </TD>
//!                    <TD> 1: GPU context enable; 0: GPU context disable (default) </TD>
//!                </TR>
//!                <TR>
//!                    <TD> kernel binary size in GSH </TD>
//!                    <TD> [13:20] </TD>
//!                    <TD> 8 </TD>
//!                    <TD> GSH kernel size is [13:20] * 2MB </TD>
//!                </TR>
//!                <TR>
//!                    <TD> DSH disable  </TD>
//!                    <TD> [21] </TD>
//!                    <TD> 1 </TD>
//!                    <TD> 1: DSH disable; 0: DSH enable (default) </TD>
//!                </TR>
//!                <TR>
//!                    <TD> Disable mid-thread level preemption </TD>
//!                    <TD> [22] </TD>
//!                    <TD> 1    </TD>
//!                    <TD> 1: disable mid-thread preemption; 0: enable (default) </TD>
//!                </TR>
//!                <TR>
//!                    <TD> kernel debug enable </TD>
//!                    <TD> [23] </TD>
//!                 <TD> 1 </TD>
//!                 <TD> 1: enable kernel debug; 0: disable (default) </TD>
//!                </TR>
//!                <TR>
//!                    <TD> Reserved </TD>
//!                    <TD> [24:27] </TD>
//!                    <TD> 4 </TD>
//!                    <TD> Must to be set as Zero </TD>
//!                </TR>
//!                <TR>
//!                    <TD> Disable VEBOX interfaces in CM </TD>
//!                    <TD> [28] </TD>
//!                    <TD> 1 </TD>
//!                    <TD> 1: disable VEBOX interfaces; 0: enable (default) </TD>
//!                </TR>
//!                <TR>
//!                    <TD> Disable FastCopy interfaces in CM </TD>
//!                    <TD> [29] </TD>
//!                    <TD> 1 </TD>
//!                    <TD> 1: disable FastCopy interfaces; 0: enable (default) </TD>
//!                </TR>
//!                <TR>
//!                    <TD> Enabel Fast Path in CM </TD>
//!                    <TD> [30] </TD>
//!                    <TD> 1 </TD>
//!                    <TD> 1: enable fast path; 0: disable (default) </TD>
//!                </TR>
//!       </table>
//! 
//! \param      [out] device
//!             Reference to the pointer to the CmDevice to be created
//! \param      [out] version
//!             Reference to CM API version supported by the runtime library
//! \param      [in] vaDisplay
//!             VReference to a given VADisplay from vaInitialize if NOT nullptr 
//! \param      [in] DevCreateOption
//!             Device creation option.
//! \retval     CM_SUCCESS if device successfully created
//! \retval     CM_OUT_OF_HOST_MEMORY if out of system memory
//!    \retval  CM_INVALID_LIBVA_INITIALIZE if vaInitialize failed
//! \retval     CM_FAILURE otherwise
//!
extern "C"
CM_RT_API int32_t CreateCmDeviceEx(CmDevice* &device, uint32_t & version, VADisplay vaDisplay , uint32_t createOption = CM_DEVICE_CREATE_OPTION_DEFAULT )
{
    INSERT_PROFILER_RECORD();

    CmDevice_RT* p=NULL;
    int32_t result = CM_FAILURE;

    if ( vaDisplay == nullptr)
    {
        result = CmDevice_RT::Create(p, createOption);
    }
    else
    {
        result = CmDevice_RT::Create(vaDisplay, p, createOption);
    }

    device = static_cast< CmDevice* >(p);
    if( result == CM_SUCCESS )
    {
        version = CURRENT_CM_VERSION;
    }
    else
    {
        version = 0;
    }

    return result;
}

