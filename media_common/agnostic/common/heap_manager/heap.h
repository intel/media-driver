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
//!
//! \file     heap.h
//! \brief    A heap is a buffer in graphics memory in which the driver may store information
//! \details  Heaps are created for the client via the heap manager, through which the client
//!           is expected to claim blocks of memory for data storage. The client has no direct
//!           access to the heap.
//!

#ifndef __HEAP_H__
#define __HEAP_H__

#include "mos_os.h"
#include "mos_util_debug.h"
#include "mos_interface.h"

//! Evaluates the status and returns if failure
#define HEAP_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _stmt)
//! Checks the pointer and returns if nullptr
#define HEAP_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _ptr)
//! Asserts and prints \a _message at critical message level
#define HEAP_ASSERTMESSAGE(_message, ...)                                                 \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_HW, MOS_SUBCOMP_SELF, _message, ##__VA_ARGS__)
//! Prints \a _message at normal message level
#define HEAP_NORMALMESSAGE(_message, ...)                                                 \
    MOS_NORMALMESSAGE(MOS_COMPONENT_HW, MOS_SUBCOMP_SELF, _message, ##__VA_ARGS__)
//! Prints \a _message at verbose message level
#define HEAP_VERBOSEMESSAGE(_message, ...)                                                \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_HW, MOS_SUBCOMP_SELF, _message, ##__VA_ARGS__)
//! Prints the function name at function enter message level
#define HEAP_FUNCTION_ENTER                                                               \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_HW, MOS_SUBCOMP_SELF)
//! Prints the function name at verbose function enter message level
#define HEAP_FUNCTION_ENTER_VERBOSE                                                       \
    MOS_FUNCTION_ENTER_VERBOSE(MOS_COMPONENT_HW, MOS_SUBCOMP_SELF)

//! \brief Generic heap for storing client written data in graphics memory
class Heap
{
    friend class MemoryBlockInternal;
    friend class MemoryBlockManager;

public:
    //!
    //! \brief  Default constructor
    //!         Identifier for the heap
    //!  
    Heap();

    //!
    //! \brief    Copy constructor
    //!
    Heap(const Heap&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    Heap& operator=(const Heap&) = delete;

    //!
    //! \brief  Constructor
    //! \param  [in] id
    //!         Identifier for the heap
    //!  
    Heap(uint32_t id);
    virtual ~Heap();

    //!  
    //! \brief  Dumps the contents of the heap
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!  
    MOS_STATUS Dump();

    //!
    //! \brief  Checks heap for validity
    //! \return bool
    //!         true if the heap is valid and false if not
    //!
    bool IsValid() { return (m_size > 0); }

    //!
    //! \brief  Gets the graphics resource for the heap
    //! \param  [out] resource
    //!         Container for the output graphics resource
    //! \return MOS_RESOURCE*
    //!         Valid pointer if success, nullptr for any fail
    //!
    MOS_RESOURCE* GetResource()
    {
        HEAP_FUNCTION_ENTER_VERBOSE;
        if (m_size && !Mos_ResourceIsNull(m_resource))
        {
            return m_resource;
        }
        HEAP_ASSERTMESSAGE("Resource is not valid");
        return nullptr;
    }

    //!
    //! \brief  Gets the size of the heap
    //! \return The size of the heap \see m_size
    //!
    uint32_t GetSize() { return m_size; }

    //!
    //! \brief  Gets the used size of the heap
    //! \return The amount of space used in the heaps \see m_usedSize
    //!
    uint32_t GetUsedSize() { return m_usedSpace; }

    //!
    //! \brief  Gets the ID for the heap
    //! \return The Heap ID \see m_id
    //!
    uint32_t GetId() { return m_id; }

    //!
    //! \brief  Whether or not the heap has been marked for freeing.
    //! \return If the heap is being freed \see m_freeInProgress
    //!  
    bool IsFreeInProgress() { return m_freeInProgress; }

    //! \brief Invalid heap ID.
    static const uint8_t m_invalidId = 0;

    //!
    //! \brief  Mark the heap as hardware write only heap or not
    void SetHeapHwWriteOnly(bool isHwWriteOnly) { m_hwWriteOnly = isHwWriteOnly; }

protected:
    //!
    //! \brief  Stores the provided OS interface in the heap
    //! \param  [in] osInterface
    //!         Must be valid
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!  
    MOS_STATUS RegisterOsInterface(PMOS_INTERFACE osInterface);

    //!
    //! \brief  Allocates a graphics resource of size \a heapSize
    //! \param  [in] heapSize
    //!         The size of the state heap, must be non-zero
    //! \param  [in] keepLocked
    //!         Locks the heap when allocated and does not unlock it until it is destroyed
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!  
    MOS_STATUS Allocate(uint32_t heapSize, bool keepLocked);

    //!
    //! \brief  Locks the heap asynchonously
    //! \return uint8_t*
    //!         If successful, pointer to data in the heap resource; if failed, nullptr \see m_resource
    //!  
    uint8_t* Lock();

    //!
    //! \brief  Unlocks the heap
    //!  
    void Unlock()
    {
        HEAP_FUNCTION_ENTER_VERBOSE;
        if (!m_keepLocked)
        {
            if (m_osInterface == nullptr)
            {
                HEAP_ASSERTMESSAGE("Invalid m_osInterface(nullptr)");
                return;
            }

            m_osInterface->pfnUnlockResource(m_osInterface, m_resource);
        }
    }

    //!
    //! \brief  Prepares the heap for being freed which will occur when all blocks are
    //!         no longer in use.
    //!  
    void PrepareForFree()
    {
        HEAP_FUNCTION_ENTER;
        m_freeInProgress = true;
    }

    //!
    //! \brief  Adjusts the free space in the heap. 
    //! \param  [in] addedSpace
    //!         Space to be added and subtracted from the free and used space
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //! 
    MOS_STATUS AdjustFreeSpace(uint32_t addedSpace);

    //!
    //! \brief  Adjusts the used space in the heap. 
    //! \param  [in] addedSpace
    //!         Space to be added and subtracted from the free and used space
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //! 
    MOS_STATUS AdjustUsedSpace(uint32_t addedSpace);

private:
    //! \brief Indicates that the heap is in the process of being freed
    bool m_freeInProgress = false;
    bool m_keepLocked = false;          //!< Indictaes that heap is kept locked until freed.
    PMOS_RESOURCE m_resource = nullptr; //!< Graphics resource for the heap.
    uint32_t m_size = 0;                //!< The size of the heap.
    //! \brief   Pointer to the locked heap data.
    //! \details This pointer is only valid if the heap is locked always
    uint8_t *m_lockedHeap = nullptr;
    uint32_t m_freeSpace = 0;           //!< The amount of space available in the heap
    uint32_t m_usedSpace = 0;           //!< The amount of used space within the heap
    //! \brief OS interface used for managing graphics resources
    PMOS_INTERFACE m_osInterface = nullptr;
    //! \brief unique identifier for the heap within the heap manager
    uint32_t m_id = m_invalidId;
    bool m_hwWriteOnly = false;            //!< Indictaes that heap is used by hardware write only.
};
#endif // __HEAP_H__
