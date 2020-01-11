/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file     mos_utilities_next.cpp
//! \brief    Common OS service across different platform
//! \details  Common OS service across different platform
//!

#include <fcntl.h>
#include <math.h>
#include "mos_os.h"
#include "mos_utilities_specific.h"

int32_t              MosUtilities::m_mosMemAllocCounterNoUserFeature    = 0;
int32_t              MosUtilities::m_mosMemAllocCounterNoUserFeatureGfx = 0;
const MtControlData *MosUtilities::m_mosTraceControlData                = nullptr;
MtEnable             MosUtilities::m_mosTraceEnable                     = false;
MtFilter             MosUtilities::m_mosTraceFilter                     = {};
MtLevel              MosUtilities::m_mosTraceLevel                      = {};

uint64_t MosUtilities::MosGetCurTime()
{
    using us = std::chrono::microseconds;
    using clock = std::chrono::steady_clock;

    clock::time_point Timer = clock::now();
    uint64_t usStartTime =
            std::chrono::duration_cast<us>(Timer.time_since_epoch()).count();

    return usStartTime;
}

#if (_DEBUG || _RELEASE_INTERNAL)

uint32_t MosUtilities::m_mosAllocMemoryFailSimulateMode = 0;
uint32_t MosUtilities::m_mosAllocMemoryFailSimulateFreq = 0;
uint32_t MosUtilities::m_mosAllocMemoryFailSimulateHint = 0;

void MosUtilities::MosInitAllocMemoryFailSimulateFlag(MediaUserSettingSharedPtr userSettingPtr)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    uint32_t   value   = 0;

    //default off for simulate random fail
    m_mosAllocMemoryFailSimulateMode = MEMORY_ALLOC_FAIL_SIMULATE_MODE_DEFAULT;
    m_mosAllocMemoryFailSimulateFreq = 0;
    m_mosAllocMemoryFailSimulateHint = 0;
    if (m_mosAllocMemoryFailSimulateAllocCounter != nullptr)
    {
        *m_mosAllocMemoryFailSimulateAllocCounter = 0;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("SimulateAllocCounter is nullptr");
    }
    // Read Config : memory allocation failure simulate mode
    ReadUserSetting(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_MODE,
        MediaUserSetting::Group::Device);

    if ((value == MEMORY_ALLOC_FAIL_SIMULATE_MODE_DEFAULT) ||
        (value == MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM) ||
        (value == MEMORY_ALLOC_FAIL_SIMULATE_MODE_TRAVERSE))
    {
        m_mosAllocMemoryFailSimulateMode = value;
        MOS_OS_NORMALMESSAGE("Init MosSimulateAllocMemoryFailSimulateMode as %d \n ", m_mosAllocMemoryFailSimulateMode);
    }
    else
    {
        m_mosAllocMemoryFailSimulateMode = MEMORY_ALLOC_FAIL_SIMULATE_MODE_DEFAULT;
        MOS_OS_NORMALMESSAGE("Invalid Alloc Memory Fail Simulate Mode from config: %d \n ", value);
    }

    // Read Config : memory allocation failure simulate frequence
    value = 0;
    ReadUserSetting(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_FREQ,
        MediaUserSetting::Group::Device);

    if ((value >= MIN_MEMORY_ALLOC_FAIL_FREQ) &&
        (value <= MAX_MEMORY_ALLOC_FAIL_FREQ))
    {
        m_mosAllocMemoryFailSimulateFreq = value;
        MOS_OS_NORMALMESSAGE("Init m_MosSimulateRandomAllocMemoryFailFreq as %d \n ", m_mosAllocMemoryFailSimulateFreq);

        if (m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM)
        {
            srand((unsigned int)time(nullptr));
        }
    }
    else
    {
        m_mosAllocMemoryFailSimulateFreq = 0;
        MOS_OS_NORMALMESSAGE("Invalid Alloc Memory Fail Simulate Freq from config: %d \n ", value);
    }

    // Read Config : memory allocation failure simulate counter
    value = 0;
    ReadUserSetting(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_HINT,
        MediaUserSetting::Group::Device);
    if (value <= m_mosAllocMemoryFailSimulateFreq)
    {
        m_mosAllocMemoryFailSimulateHint = value;
        MOS_OS_NORMALMESSAGE("Init m_MosAllocMemoryFailSimulateHint as %d \n ", m_mosAllocMemoryFailSimulateHint);
    }
    else
    {
        m_mosAllocMemoryFailSimulateHint = m_mosAllocMemoryFailSimulateFreq;
        MOS_OS_NORMALMESSAGE("Set m_mosAllocMemoryFailSimulateHint as %d since INVALID CONFIG %d \n ", m_mosAllocMemoryFailSimulateHint, value);
    }
}

bool MosUtilities::MosSimulateAllocMemoryFail(
    size_t      size,
    size_t      alignment,
    const char *functionName,
    const char *filename,
    int32_t     line)
{
    bool bSimulateAllocFail = false;

    if (!MosAllocMemoryFailSimulationEnabled)
    {
        return false;
    }

    if (m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM)
    {
        int32_t Rn = rand();
        MosAtomicIncrement(m_mosAllocMemoryFailSimulateAllocCounter);
        if (Rn % m_mosAllocMemoryFailSimulateFreq == 1)
        {
            bSimulateAllocFail = true;
            MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_CRITICAL, MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, "Simulated Allocate Memory Fail (Rn=%d, SimulateAllocCounter=%d) for: functionName: %s, filename: %s, line: %d, size: %d, alignment: %d \n", Rn, (m_mosAllocMemoryFailSimulateAllocCounter ? *m_mosAllocMemoryFailSimulateAllocCounter : 0), functionName, filename, line, size, alignment);
        }
        else
        {
            bSimulateAllocFail = false;
        }
    }
    else if (m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_TRAVERSE)
    {
        if (m_mosAllocMemoryFailSimulateAllocCounter &&
            (*m_mosAllocMemoryFailSimulateAllocCounter == m_mosAllocMemoryFailSimulateHint))
        {
            MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_CRITICAL, MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, "Simulated Allocate Memory Fail (hint=%d) for: functionName: %s, filename: %s, line: %d, size: %d \n", m_mosAllocMemoryFailSimulateHint, functionName, filename, line, size, alignment);
            bSimulateAllocFail = true;
        }
        else
        {
            bSimulateAllocFail = false;
        }
        MosAtomicIncrement(m_mosAllocMemoryFailSimulateAllocCounter);
    }
    else
    {
        MOS_OS_NORMALMESSAGE("Invalid m_MosAllocMemoryFailSimulateMode: %d \n ", m_mosAllocMemoryFailSimulateMode);
        bSimulateAllocFail = false;
    }

    return bSimulateAllocFail;
}
#endif  // #if (_DEBUG || _RELEASE_INTERNAL)

#if MOS_MESSAGES_ENABLED
void *MosUtilities::MosAlignedAllocMemoryUtils(
    size_t      size,
    size_t      alignment,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void  *MosUtilities::MosAlignedAllocMemory(
    size_t  size,
    size_t  alignment)
#endif // MOS_MESSAGES_ENABLED
{
    void  *ptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateAllocMemoryFail(size, alignment, functionName, filename, line))
    {
        return nullptr;
    }
#endif

    ptr = _aligned_malloc(size, alignment);

    MOS_OS_ASSERT(ptr != nullptr);

    if(ptr != nullptr)
    {
        MosAtomicIncrement(m_mosMemAllocCounter);
        MOS_MEMNINJA_ALLOC_MESSAGE(ptr, size, functionName, filename, line);
        PRINT_ALLOCATE_MEMORY(MT_MOS_ALLOCATE_MEMORY, MT_NORMAL,
                MT_MEMORY_PTR, (int64_t)(ptr),
                MT_MEMORY_SIZE, static_cast<int64_t>(size), 
                functionName, filename, line);
    }

    return ptr;
}

#if MOS_MESSAGES_ENABLED
void MosUtilities::MosAlignedFreeMemoryUtils(
    void        *ptr,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void MosUtilities::MosAlignedFreeMemory(void *ptr)
#endif // MOS_MESSAGES_ENABLED
{
    MOS_OS_ASSERT(ptr != nullptr);

    if(ptr != nullptr)
    {
        MosAtomicDecrement(m_mosMemAllocCounter);
        MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line);
        PRINT_DESTROY_MEMORY(MT_MOS_DESTROY_MEMORY, MT_NORMAL,
                MT_MEMORY_PTR, (int64_t)(ptr),
                functionName, filename, line);        
        _aligned_free(ptr);
    }
}

#if MOS_MESSAGES_ENABLED
void *MosUtilities::MosAllocMemoryUtils(
    size_t      size,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void *MosUtilities::MosAllocMemory(size_t size)
#endif // MOS_MESSAGES_ENABLED
{
    void  *ptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateAllocMemoryFail(size, NO_ALLOC_ALIGNMENT, functionName, filename, line))
    {
        return nullptr;
    }
#endif

    ptr = malloc(size);

    MOS_OS_ASSERT(ptr != nullptr);

    if(ptr != nullptr)
    {
        MosAtomicIncrement(m_mosMemAllocCounter);
        MOS_MEMNINJA_ALLOC_MESSAGE(ptr, size, functionName, filename, line);
        PRINT_ALLOCATE_MEMORY(MT_MOS_ALLOCATE_MEMORY, MT_NORMAL,
                MT_MEMORY_PTR, (int64_t)(ptr),
                MT_MEMORY_SIZE, static_cast<int64_t>(size),
                functionName, filename, line);
    }

    return ptr;
}

#if MOS_MESSAGES_ENABLED
void *MosUtilities::MosAllocAndZeroMemoryUtils(
    size_t      size,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void *MosUtilities::MosAllocAndZeroMemory(size_t size)
#endif // MOS_MESSAGES_ENABLED
{
    void  *ptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateAllocMemoryFail(size, NO_ALLOC_ALIGNMENT, functionName, filename, line))
    {
        return nullptr;
    }
#endif

    ptr = malloc(size);

    MOS_OS_ASSERT(ptr != nullptr);

    if(ptr != nullptr)
    {
        MosZeroMemory(ptr, size);

        MosAtomicIncrement(m_mosMemAllocCounter);
        MOS_MEMNINJA_ALLOC_MESSAGE(ptr, size, functionName, filename, line);
        PRINT_ALLOCATE_MEMORY(MT_MOS_ALLOCATE_MEMORY, MT_NORMAL,
                MT_MEMORY_PTR, (int64_t)(ptr),
                MT_MEMORY_SIZE, static_cast<int64_t>(size),
                functionName, filename, line);
    }

    return ptr;
}

#if MOS_MESSAGES_ENABLED
void *MosUtilities::MosReallocMemoryUtils(
    void       *ptr,
    size_t     newSize,
    const char *functionName,
    const char *filename,
    int32_t    line)
#else
void *MosUtilities::MosReallocMemory(
    void       *ptr,
    size_t     newSize)
#endif // MOS_MESSAGES_ENABLED
{
    uintptr_t oldPtr = reinterpret_cast<uintptr_t>(nullptr);
    void *newPtr = nullptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateAllocMemoryFail(newSize, NO_ALLOC_ALIGNMENT, functionName, filename, line))
    {
        return nullptr;
    }
#endif

    oldPtr = reinterpret_cast<uintptr_t>(ptr);
    newPtr = realloc(ptr, newSize);

    MOS_OS_ASSERT(newPtr != nullptr);

    if (newPtr != reinterpret_cast<void*>(oldPtr))
    {
        if (oldPtr != reinterpret_cast<uintptr_t>(nullptr))
        {
            MosAtomicDecrement(m_mosMemAllocCounter);
            MOS_MEMNINJA_FREE_MESSAGE(oldPtr, functionName, filename, line);
            PRINT_DESTROY_MEMORY(MT_MOS_DESTROY_MEMORY, MT_NORMAL,
                MT_MEMORY_PTR, (int64_t)(oldPtr), 
                functionName, filename, line); 
        }

        if (newPtr != nullptr)
        {
            MosAtomicIncrement(m_mosMemAllocCounter);
            MOS_MEMNINJA_ALLOC_MESSAGE(newPtr, newSize, functionName, filename, line);
            PRINT_ALLOCATE_MEMORY(MT_MOS_ALLOCATE_MEMORY, MT_NORMAL,
                MT_MEMORY_PTR, (int64_t)(newPtr),
                MT_MEMORY_SIZE, static_cast<int64_t>(newSize),
                functionName, filename, line);
        }
    }

    return newPtr;
}

//!
//! \brief    Wrapper for free(). Performs error checking.
//! \details  Wrapper for free(). Performs error checking.
//!           It decreases memory allocation counter variable
//!           m_mosMemAllocCounter for checking memory leaks.
//! \param    void  *ptr
//!           [in] Pointer to the memory to be freed
//! \return   void
//!
#if MOS_MESSAGES_ENABLED
void MosUtilities::MosFreeMemoryUtils(
    void        *ptr,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void MosUtilities::MosFreeMemory(void  *ptr)
#endif // MOS_MESSAGES_ENABLED
{
    if(ptr != nullptr)
    {
        MosAtomicDecrement(m_mosMemAllocCounter);
        MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line);
        PRINT_DESTROY_MEMORY(MT_MOS_DESTROY_MEMORY, MT_NORMAL, 
            MT_MEMORY_PTR, (int64_t)(ptr), 
            functionName, filename, line); 
        free(ptr);
        ptr = nullptr;
    }
}

void MosUtilities::MosZeroMemory(void  *pDestination, size_t stLength)
{
    MOS_OS_ASSERT(pDestination != nullptr);

    if(pDestination != nullptr)
    {
        memset(pDestination, 0, stLength);
    }
}

void MosUtilities::MosFillMemory(void  *pDestination, size_t stLength, uint8_t bFill)
{
    MOS_OS_ASSERT(pDestination != nullptr);

    if(pDestination != nullptr)
    {
        memset(pDestination, bFill, stLength);
    }
}

MOS_STATUS  MosUtilities::MosReadFileToPtr(
    const char      *pFilename,
    uint32_t        *lpNumberOfBytesRead,
    void            **ppReadBuffer)
{
    HANDLE          hFile;
    void            *lpBuffer;
    uint32_t        fileSize;
    uint32_t        bytesRead;
    MOS_STATUS      eStatus;

    *ppReadBuffer = nullptr;
    *lpNumberOfBytesRead = 0;

    eStatus = MosCreateFile(&hFile, (char *)pFilename, O_RDONLY);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to open file '%s'.", pFilename);
        return eStatus;
    }

    eStatus = MosGetFileSize(hFile, &fileSize, nullptr);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to get size of file '%s'.", pFilename);
        MosCloseHandle(hFile);
        return eStatus;
    }

    lpBuffer = MOS_AllocAndZeroMemory(fileSize);
    if (lpBuffer == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
        MosCloseHandle(hFile);
        return MOS_STATUS_NO_SPACE;
    }

    if((eStatus = MosReadFile(hFile, lpBuffer, fileSize, &bytesRead, nullptr)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to read from file '%s'.", pFilename);
        MosCloseHandle(hFile);
        MOS_FreeMemory(lpBuffer);
        lpBuffer = nullptr;
        return eStatus;
    }

    MosCloseHandle(hFile);
    *lpNumberOfBytesRead = bytesRead;
    *ppReadBuffer = lpBuffer;
    return eStatus;
}

MOS_STATUS MosUtilities::MosWriteFileFromPtr(
    const char      *pFilename,
    void            *lpBuffer,
    uint32_t        writeSize)
{
    HANDLE          hFile;
    uint32_t        bytesWritten;
    MOS_STATUS      eStatus;

    MOS_OS_CHK_NULL_RETURN(pFilename);
    MOS_OS_CHK_NULL_RETURN(lpBuffer);

    if (writeSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("Attempting to write 0 bytes to a file");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    bytesWritten    = 0;

    eStatus = MosCreateFile(&hFile, (char *)pFilename, O_WRONLY|O_CREAT);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to open file '%s'.", pFilename);
        return eStatus;
    }

    if((eStatus = MosWriteFile(hFile, lpBuffer, writeSize, &bytesWritten, nullptr)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write to file '%s'.", pFilename);
        MosCloseHandle(hFile);
        return eStatus;
    }

    MosCloseHandle(hFile);

    return eStatus;
}

MOS_STATUS MosUtilities::MosAppendFileFromPtr(
    const char      *pFilename,
    void            *pData,
    uint32_t        dwSize)
{
    MOS_STATUS  eStatus;
    HANDLE      hFile;
    uint32_t    dwWritten;

    //------------------------------
    MOS_OS_ASSERT(pFilename);
    MOS_OS_ASSERT(pData);
    //------------------------------
    dwWritten   = 0;

    eStatus = MosCreateFile(&hFile, (char *)pFilename, O_WRONLY | O_CREAT | O_APPEND);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to open file '%s'.", pFilename);
        return eStatus;
    }

    eStatus = MosSetFilePointer(hFile, 0, nullptr, SEEK_END);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to set file pointer'%s'.", pFilename);
        MosCloseHandle(hFile);
        return eStatus;
    }

    // Write the file
    if((eStatus = MosWriteFile(hFile, pData, dwSize, &dwWritten, nullptr)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write to file '%s'.", pFilename);
        MosCloseHandle(hFile);
        return eStatus;
    }

    MosCloseHandle(hFile);
    return eStatus;
}

float MosUtilities::MosSinc(float x)
{
    return (MOS_ABS(x) < 1e-9f) ? 1.0F : (float)(sin(x) / x);
}

float MosUtilities::MosLanczos(float x, uint32_t dwNumEntries, float fLanczosT)
{
    uint32_t dwNumHalfEntries;

    dwNumHalfEntries = dwNumEntries >> 1;
    if (fLanczosT < dwNumHalfEntries)
    {
        fLanczosT = (float)dwNumHalfEntries;
    }

    if (MOS_ABS(x) >= dwNumHalfEntries)
    {
        return 0.0;
    }

    x *= MOS_PI;

    return MosSinc(x) * MosSinc(x / fLanczosT);
}

float MosUtilities::MosLanczosG(float x, uint32_t dwNumEntries, float fLanczosT)
{
    uint32_t dwNumHalfEntries;

    dwNumHalfEntries = (dwNumEntries >> 1) + (dwNumEntries & 1);
    if (fLanczosT < dwNumHalfEntries)
    {
        fLanczosT = (float)dwNumHalfEntries;
    }

    if (x > (dwNumEntries >> 1) || (- x) >= dwNumHalfEntries)
    {
        return 0.0;
    }

    x *= MOS_PI;

    return MosSinc(x) * MosSinc(x / fLanczosT);
}

uint32_t MosUtilities::MosGCD(uint32_t a, uint32_t b)
{
    if (b == 0)
    {
        return a;
    }
    else
    {
        return MosGCD(b, a % b);
    }
}

#ifdef _MOS_UTILITY_EXT
#include "mos_utilities_ext_next.h"
#else
#define Mos_SwizzleOffset MosUtilities::MosSwizzleOffset
#endif

__inline int32_t MosUtilities::MosSwizzleOffset(
    int32_t         OffsetX,
    int32_t         OffsetY,
    int32_t         Pitch,
    MOS_TILE_TYPE   TileFormat,
    int32_t         CsxSwizzle,
    int32_t         ExtFlags)
{
    // When dealing with a tiled surface, logical linear accesses to the
    // surface (y * pitch + x) must be translated into appropriate tile-
    // formated accesses--This is done by swizzling (rearranging/translating)
    // the given access address--though it is important to note that the
    // swizzling is actually done on the accessing OFFSET into a TILED
    // REGION--not on the absolute address itself.

    // (!) Y-MAJOR TILING, REINTERPRETATION: For our purposes here, Y-Major
    // tiling will be thought of in a different way, we will deal with
    // the 16-byte-wide columns individually--i.e., we will treat a single
    // Y-Major tile as 8 separate, thinner tiles--Doing so allows us to
    // deal with both X- and Y-Major tile formats in the same "X-Major"
    // way--just with different dimensions: either 512B x 8 rows, or
    // 16B x 32 rows, respectively.

    // A linear offset into a surface is of the form
    //     y * pitch + x   =   y:x (Shorthand, meaning: y * (x's per y) + x)
    //
    // To treat a surface as being composed of tiles (though still being
    // linear), just as a linear offset has a y:x composition--its y and x
    // components can be thought of as having Row:Line and Column:X
    // compositions, respectively, where Row specifies a row of tiles, Line
    // specifies a row of pixels within a tile, Column specifies a column
    // of tiles, and X in this context refers to a byte within a Line--i.e.,
    //     offset = y:x
    //     y = Row:Line
    //     x = Col:X
    //     offset = y:x = Row:Line:Col:X

    // Given the Row:Line:Col:X composition of a linear offset, all that
    // tile swizzling does is swap the Line and Col components--i.e.,
    //     Linear Offset:   Row:Line:Col:X
    //     Swizzled Offset: Row:Col:Line:X
    // And with our reinterpretation of the Y-Major tiling format, we can now
    // describe both the X- and Y-Major tiling formats in two simple terms:
    // (1) The bit-depth of their Lines component--LBits, and (2) the
    // swizzled bit-position of the Lines component (after it swaps with the
    // Col component)--LPos.

    int32_t Row, Line, Col, x; // Linear Offset Components
    int32_t LBits, LPos; // Size and swizzled position of the Line component.
    int32_t SwizzledOffset;
    if (TileFormat == MOS_TILE_LINEAR)
    {
        return(OffsetY * Pitch + OffsetX);
    }

    if (TileFormat == MOS_TILE_Y)
    {
        LBits = 5; // Log2(TileY.Height = 32)
        LPos = 4;  // Log2(TileY.PseudoWidth = 16)
    }
    else //if (TileFormat == MOS_TILE_X)
    {
        LBits = 3; // Log2(TileX.Height = 8)
        LPos = 9;  // Log2(TileX.Width = 512)
    }

    Row = OffsetY >> LBits;               // OffsetY / LinesPerTile
    Line = OffsetY & ((1 << LBits) - 1);   // OffsetY % LinesPerTile
    Col = OffsetX >> LPos;                // OffsetX / BytesPerLine
    x = OffsetX & ((1 << LPos) - 1);    // OffsetX % BytesPerLine

    SwizzledOffset =
        (((((Row * (Pitch >> LPos)) + Col) << LBits) + Line) << LPos) + x;
    //                V                V                 V
    //                / BytesPerLine   * LinesPerTile    * BytesPerLine

    /// Channel Select XOR Swizzling ///////////////////////////////////////////
    if (CsxSwizzle)
    {
        if (TileFormat == MOS_TILE_Y) // A6 = A6 ^ A9
        {
            SwizzledOffset ^= ((SwizzledOffset >> (9 - 6)) & 0x40);
        }
        else //if (TileFormat == VPHAL_TILE_X) // A6 = A6 ^ A9 ^ A10
        {
            SwizzledOffset ^= (((SwizzledOffset >> (9 - 6)) ^ (SwizzledOffset >> (10 - 6))) & 0x40);
        }
    }

    return(SwizzledOffset);
}

int32_t MosUtilities::MosSwizzleOffsetWrapper(
    int32_t         OffsetX,
    int32_t         OffsetY,
    int32_t         Pitch,
    MOS_TILE_TYPE   TileFormat,
    int32_t         CsxSwizzle,
    int32_t         Flags)
{
    return Mos_SwizzleOffset(OffsetX, OffsetY, Pitch, TileFormat, CsxSwizzle, Flags);
}

void MosUtilities::MosSwizzleData(
    uint8_t         *pSrc,
    uint8_t         *pDst,
    MOS_TILE_TYPE   SrcTiling,
    MOS_TILE_TYPE   DstTiling,
    int32_t         iHeight,
    int32_t         iPitch,
    int32_t         extFlags)
{

#define IS_TILED(_a)                ((_a) != MOS_TILE_LINEAR)
#define IS_TILED_TO_LINEAR(_a, _b)  (IS_TILED(_a) && !IS_TILED(_b))
#define IS_LINEAR_TO_TILED(_a, _b)  (!IS_TILED(_a) && IS_TILED(_b))

    int32_t LinearOffset;
    int32_t TileOffset;
    int32_t x;
    int32_t y;

    // Translate from one format to another
    for (y = 0, LinearOffset = 0, TileOffset = 0; y < iHeight; y++)
    {
        for (x = 0; x < iPitch; x++, LinearOffset++)
        {
            // x or y --> linear
            if (IS_TILED_TO_LINEAR(SrcTiling, DstTiling))
            {
                TileOffset = Mos_SwizzleOffset(
                    x,
                    y,
                    iPitch,
                    SrcTiling,
                    false,
                    extFlags);
                if(TileOffset < iHeight * iPitch)
                    *(pDst + LinearOffset) = *(pSrc + TileOffset);
            }
            // linear --> x or y
            else if (IS_LINEAR_TO_TILED(SrcTiling, DstTiling))
            {
                TileOffset = Mos_SwizzleOffset(
                    x,
                    y,
                    iPitch,
                    DstTiling,
                    false,
                    extFlags);
                if(TileOffset < iHeight * iPitch)
                    *(pDst + TileOffset) = *(pSrc + LinearOffset);
            }
            else
            {
                MOS_OS_ASSERT(0);
            }
        }
    }
}

std::shared_ptr<PerfUtility> PerfUtility::instance = nullptr;
std::mutex PerfUtility::perfMutex;
PerfUtility* g_perfutility = PerfUtility::getInstance();

PerfUtility *PerfUtility::getInstance()
{
    if (instance == nullptr)
    {
        instance = std::make_shared<PerfUtility>();
    }

    return instance.get();
}

PerfUtility::PerfUtility()
{
    bPerfUtilityKey = false;
    dwPerfUtilityIsEnabled = 0;
}

PerfUtility::~PerfUtility()
{
    for (const auto &data : records)
    {
        if (data.second)
        {
            delete data.second;
        }
    }
    records.clear();
}

void PerfUtility::setupFilePath(const char *perfFilePath)
{
    int32_t pid = MosUtilities::MosGetPid();
    MOS_SecureStringPrint(sSummaryFileName, MOS_MAX_PATH_LENGTH + 1, MOS_MAX_PATH_LENGTH + 1,
        "%sperf_summary_pid%d.csv", perfFilePath, pid);
    MOS_SecureStringPrint(sDetailsFileName, MOS_MAX_PATH_LENGTH + 1, MOS_MAX_PATH_LENGTH + 1,
        "%sperf_details_pid%d.txt", perfFilePath, pid);
}

void PerfUtility::setupFilePath()
{
    int32_t pid = MosUtilities::MosGetPid();
    MOS_SecureStringPrint(sSummaryFileName, MOS_MAX_PATH_LENGTH + 1, MOS_MAX_PATH_LENGTH + 1,
        "perf_summary_pid%d.csv", pid);
    MOS_SecureStringPrint(sDetailsFileName, MOS_MAX_PATH_LENGTH + 1, MOS_MAX_PATH_LENGTH + 1,
        "perf_details_pid%d.txt", pid);
}

void PerfUtility::savePerfData()
{
    printPerfSummary();

    printPerfDetails();
}

void PerfUtility::printPerfSummary()
{
    std::ofstream fout;
    fout.open(sSummaryFileName);
    if(fout.good() == false)
    {
        fout.close();
        return;
    }
    printHeader(fout);
    printBody(fout);
    fout.close();
    return;
}

void PerfUtility::printPerfDetails()
{
    std::ofstream fout;
    fout.open(sDetailsFileName);
    if(fout.good() == false)
    {
        fout.close();
        return;
    }
    for (auto data : records)
    {
        fout << getDashString((uint32_t)data.first.length());
        fout << data.first << std::endl;
        fout << getDashString((uint32_t)data.first.length());
        for (auto t : *data.second)
        {
            fout << t.time << std::endl;
        }
        fout << std::endl;
    }

    fout.close();
    return;
}

void PerfUtility::printHeader(std::ofstream& fout)
{
    fout << "Summary: " << std::endl;
    std::stringstream ss;
    ss << "CPU Latency Tag,";
    ss << "Hit Count,";
    ss << "Average (ms),";
    ss << "Minimum (ms),";
    ss << "Maximum (ms)" << std::endl;
    fout << ss.str();
}

void PerfUtility::printBody(std::ofstream& fout)
{
    for (const auto& data : records)
    {
        fout << formatPerfData(data.first, *data.second);
    }
}

std::string PerfUtility::formatPerfData(std::string tag, std::vector<Tick>& record)
{
    std::stringstream ss;
    PerfInfo info = {};
    getPerfInfo(record, &info);

    ss << tag;
    ss << ",";
    ss.precision(3);
    ss.setf(std::ios::fixed, std::ios::floatfield);

    ss << info.count;
    ss << ",";
    ss << info.avg;
    ss << ",";
    ss << info.min;
    ss << ",";
    ss << info.max << std::endl;

    return ss.str();
}

void PerfUtility::getPerfInfo(std::vector<Tick>& record, PerfInfo* info)
{
    if (record.size() <= 0)
        return;

    info->count = (uint32_t)record.size();
    double sum = 0, max = 0, min = 10000000.0;
    for (auto t : record)
    {
        sum += t.time;
        max = (max < t.time) ? t.time : max;
        min = (min > t.time) ? t.time : min;
    }
    info->avg = sum / info->count;
    info->max = max;
    info->min = min;
}

void PerfUtility::printFooter(std::ofstream& fout)
{
    fout << getDashString(80);
}

std::string PerfUtility::getDashString(uint32_t num)
{
    std::stringstream ss;
    ss.width(num);
    ss.fill('-');
    ss << std::left << "" << std::endl;
    return ss.str();
}
