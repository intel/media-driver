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
//! \file      cm_array.cpp  
//! \brief     Contains  Class CmDynamicArray definitions  
//!

#include "cm_array.h"
#include "cm_mem.h"
namespace CMRT_UMD
{
/*****************************************************************************\

Function:
    CmDynamicArray Constructor

Description:
    Initializes the array

Input:
    const uint32_t initSize - initial size of the array, in elements

Output:
    none

\*****************************************************************************/
CmDynamicArray::CmDynamicArray( const uint32_t initSize )
{
    m_arrayBuffer = nullptr;

    m_usedSize = 0;
    m_actualSize = 0;

    CreateArray( initSize );

}
/*****************************************************************************\

Function:
    CmDynamicArray Constructor

Description:
    Initializes the array

Input:
    none

Output:
    none

\*****************************************************************************/
CmDynamicArray::CmDynamicArray()
{
    m_arrayBuffer = nullptr;
    m_usedSize = 0;
    m_actualSize = 0;
}
/*****************************************************************************\

Function:
    CmDynamicArray Destructor

Description:
    Frees all internal dynamic memory

Input:
    none

Output:
    none

\*****************************************************************************/

CmDynamicArray::~CmDynamicArray( void )
{
    Delete();
}

/*****************************************************************************\

Function:
    CmDynamicArray::GetElement

Description:
    Returns the element at the index in the array

Input:
    const uint32_t index - index of element to reference

Output:
    void* - value of element in array

\*****************************************************************************/

void* CmDynamicArray::GetElement( const uint32_t index ) 
{
    void* element;

    if( m_arrayBuffer && IsValidIndex( index ) )
    {
        element = m_arrayBuffer[ index ];
    }
    else
    {
        CM_NORMALMESSAGE("Warning: Failed to get the element at the index in the array.");
        CmSafeMemSet( &element, 0, sizeof(void*) );
    }
    return element;
}

/*****************************************************************************\

Function:
    CmDynamicArray::SetElement

Description:
    Sets the element at the index in the array to the given element

Input:
    const uint32_t index - index of element to reference
    const void* element - value of element to set

Output:
    bool - SUCCESS or FAIL

\*****************************************************************************/

bool CmDynamicArray::SetElement( const uint32_t index, const void* element )
{
    bool success = false;

    // If the index is larger than the size of the array then grow the array
    if( !IsValidIndex( index ) )
    {
        CreateArray( index + 1 );
    }

    if( m_arrayBuffer && IsValidIndex( index ) )
    {
        m_arrayBuffer[ index ] = (void*)element;
        success = true;
    }

    CM_ASSERT( success );
    return success;
}

/*****************************************************************************\

Function:
    CmDynamicArray::GetSize

Description:
    Returns the current number of elements in the array

Input:
    void

Output:
    uint32_t - size of the array in elements

\*****************************************************************************/

uint32_t CmDynamicArray::GetSize( void ) 
{
    const uint32_t size = m_usedSize;
    return size;
}



/*****************************************************************************\

Function:
    CmDynamicArray::Delete

Description:
    Deletes the internal data

Input:
    void

Output:
    void

\*****************************************************************************/

void CmDynamicArray::Delete( void )
{
    DeleteArray();
    m_usedSize = 0;
}


/*****************************************************************************\

Function:
    CmDynamicArray::operator=

Description:
    Equal operator to copy an array

Input:
    const CmDynamicArray& array - array to copy

Output:
    *this

\*****************************************************************************/

CmDynamicArray& CmDynamicArray::operator= ( const CmDynamicArray &array )
{

    if( array.m_arrayBuffer )
    {
        if( m_usedSize < array.m_usedSize )
        {
            CreateArray( array.m_usedSize );
        }

        if( m_arrayBuffer && ( m_usedSize >= array.m_usedSize ) )
        {
            for( uint32_t i = 0; i < array.m_usedSize; i++ )
            {
                m_arrayBuffer[i] = array.m_arrayBuffer[i];
            }
        }
    }


    return *this;
}

/*****************************************************************************\

Function:
    CmDynamicArray::CreateArray

Description:
    Creates the internal array structure of the specified size

Input:
    const uint32_t size - number of elements

Output:
    void

\*****************************************************************************/

void CmDynamicArray::CreateArray( const uint32_t size )
{
    if( size )
    {
        if( size > GetMaxSize() )
        {
            uint32_t actualSize = GetMaxSize() * 2;

            if( size > actualSize )
            {
                // The minimum allocation size is 32 elements, and
                // the allocations size is in multiples of 32 elements
                actualSize = (uint32_t)Round( Max( size, 32 ), 32 );
            }

            CM_ASSERT( actualSize >= size );
            CM_ASSERT( actualSize > m_actualSize );

            const uint32_t allocSize = actualSize * sizeof(void*);

            void** arrayBuffer = MOS_NewArray(void*, allocSize);

            if( arrayBuffer )
            {
                CmSafeMemSet( arrayBuffer, 0, allocSize );

                if( m_arrayBuffer )
                {
                    for( uint32_t i = 0; i < m_usedSize; i++ )
                    {
                        arrayBuffer[i] = m_arrayBuffer[i];
                    }

                    DeleteArray();
                }

                m_arrayBuffer = arrayBuffer;
                m_actualSize = actualSize;
                m_usedSize = size;
            }
            else
            {
                CM_ASSERTMESSAGE("Failed to create the internal array structure of the specified size.");
                return;
            }
        }
        else
        {
            // Update the array length
            m_usedSize = size;
        }
    }
}

/*****************************************************************************\

Function:
    CmDynamicArray::DeleteArray

Description:
    Deletes the internal array structure

Input:
    void

Output:
    void

\*****************************************************************************/
void CmDynamicArray::DeleteArray( void )
{
    if( m_arrayBuffer )
    {
        MOS_DeleteArray(m_arrayBuffer);
        m_arrayBuffer = nullptr;
    }

    m_actualSize = 0;
}

/*****************************************************************************\

Function:
    CmDynamicArray::GetMaxSize

Description:
    Returns the maximum number of elements in the array

Input:
    void

Output:
    uint32_t length

\*****************************************************************************/
uint32_t CmDynamicArray::GetMaxSize( void )
{
    return m_actualSize;
}

/*****************************************************************************\

Function:
    CmDynamicArray::IsValidIndex

Description:
    Determines if the index is in the array

Input:
    const uint32_t index

Output:
    bool

\*****************************************************************************/

bool CmDynamicArray::IsValidIndex( const uint32_t index ) 
{
    return ( index < GetSize() );
}

/*****************************************************************************\

Function:
    CmDynamicArray::GetFirstFreeIndex()

Description:
    Returns the index of the first free slot in the array.

Input:
    void

Output:
    Returns the index of the first free slot in the array.
    If all the slots are occupied, it will return the max size of Array.
\*****************************************************************************/
uint32_t CmDynamicArray::GetFirstFreeIndex()
{
    uint32_t index = 0;
    for(  index = 0; index < GetMaxSize(); index++ )
    {
        if( m_arrayBuffer[ index ] == nullptr)
        { // Find the first free slot in array
            return index;
        }
    }
    return index; 
}

/*****************************************************************************\

Function:
    CmDynamicArray::SetElementIntoFreeSlot(const void* element)

Description:
    Set the element into the first available slot in the array
    If all the slots are occupied, it will expend the array first. 
   

Input:
    void

Output:
    

\*****************************************************************************/

bool  CmDynamicArray::SetElementIntoFreeSlot(const void* element)
{
    uint32_t index = GetFirstFreeIndex();

    return SetElement(index, element);
}
}