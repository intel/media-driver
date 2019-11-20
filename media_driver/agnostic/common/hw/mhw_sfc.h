/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     mhw_sfc.h
//! \brief    MHW interface for constructing commands for the SFC
//! \details  Impelements the functionalities common across all platforms for MHW_SFC
//!
#ifndef __MHW_SFC_H__
#define __MHW_SFC_H__

#include "mhw_cp_interface.h"
#include "mhw_state_heap.h"
#include "mhw_utilities.h"
#include "mos_os.h"

static const int   MHW_SFC_CACHELINE_SIZE    = 64;
static const int   MHW_SFC_MIN_HEIGHT        = 128;
static const int   MHW_SFC_MIN_WIDTH         = 128;
static const int   MHW_SFC_MAX_HEIGHT        = 4096;
static const int   MHW_SFC_MAX_WIDTH         = 4096;
static const int   MHW_SFC_VE_HEIGHT_ALIGN   = 4;
static const int   MHW_SFC_VE_WIDTH_ALIGN    = 16;
static const float MHW_SFC_MIN_SCALINGFACTOR = (1.0F / 8.0F);
static const float MHW_SFC_MAX_SCALINGFACTOR = 8.0F;

typedef class MhwSfcInterface MHW_SFC_INTERFACE, *PMHW_SFC_INTERFACE;

typedef enum _SFC_AVS_INPUT_SITING_COEF
{
    SFC_AVS_INPUT_SITING_COEF_0_OVER_8 = 0x0,
    SFC_AVS_INPUT_SITING_COEF_1_OVER_8 = 0x1,
    SFC_AVS_INPUT_SITING_COEF_2_OVER_8 = 0x2,
    SFC_AVS_INPUT_SITING_COEF_3_OVER_8 = 0x3,
    SFC_AVS_INPUT_SITING_COEF_4_OVER_8 = 0x4,
    SFC_AVS_INPUT_SITING_COEF_5_OVER_8 = 0x5,
    SFC_AVS_INPUT_SITING_COEF_6_OVER_8 = 0x6,
    SFC_AVS_INPUT_SITING_COEF_7_OVER_8 = 0x7,
    SFC_AVS_INPUT_SITING_COEF_8_OVER_8 = 0x8
} SFC_AVS_INPUT_SITING_COEF, *PSFC_AVS_INPUT_SITING_COEF;

//!
//! \brief  Structure to hold AVS Luma Filter Coeff, same as SFC_AVS_LUMA_FILTER_COEFF_G9
//!
typedef struct _SFC_AVS_LUMA_FILTER_COEFF
{
    // DWORD 0
    union
    {
        struct
        {
            uint32_t    Table0XFilterCoefficient0   : MHW_BITFIELD_RANGE(0,7);
            uint32_t    Table0YFilterCoefficient0   : MHW_BITFIELD_RANGE(8,15);
            uint32_t    Table0XFilterCoefficient1   : MHW_BITFIELD_RANGE(16,23);
            uint32_t    Table0YFilterCoefficient1   : MHW_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t    Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            uint32_t    Table0XFilterCoefficient2   : MHW_BITFIELD_RANGE(0,7);
            uint32_t    Table0YFilterCoefficient2   : MHW_BITFIELD_RANGE(8,15);
            uint32_t    Table0XFilterCoefficient3   : MHW_BITFIELD_RANGE(16,23);
            uint32_t    Table0YFilterCoefficient3   : MHW_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t    Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            uint32_t    Table0XFilterCoefficient4   : MHW_BITFIELD_RANGE(0,7);
            uint32_t    Table0YFilterCoefficient4   : MHW_BITFIELD_RANGE(8,15);
            uint32_t    Table0XFilterCoefficient5   : MHW_BITFIELD_RANGE(16,23);
            uint32_t    Table0YFilterCoefficient5   : MHW_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t    Value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
            uint32_t    Table0XFilterCoefficient6   : MHW_BITFIELD_RANGE(0,7);
            uint32_t    Table0YFilterCoefficient6   : MHW_BITFIELD_RANGE(8,15);
            uint32_t    Table0XFilterCoefficient7   : MHW_BITFIELD_RANGE(16,23);
            uint32_t    Table0YFilterCoefficient7   : MHW_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t    Value;
        };
    } DW3;
} SFC_AVS_LUMA_FILTER_COEFF, *PSFC_AVS_LUMA_FILTER_COEFF;

//!
//! \brief  Structure to hold AVS Chroma Filter Coeff, same as SFC_AVS_CHROMA_FILTER_COEFF_G9
//!
typedef struct _SFC_AVS_CHROMA_FILTER_COEFF
{
    // DWORD 0
    union
    {
        struct
        {
            uint32_t    Table1XFilterCoefficient2   : MHW_BITFIELD_RANGE(0,7);
            uint32_t    Table1YFilterCoefficient2   : MHW_BITFIELD_RANGE(8,15);
            uint32_t    Table1XFilterCoefficient3   : MHW_BITFIELD_RANGE(16,23);
            uint32_t    Table1YFilterCoefficient3   : MHW_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t    Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            uint32_t    Table1XFilterCoefficient4   : MHW_BITFIELD_RANGE(0,7);
            uint32_t    Table1YFilterCoefficient4   : MHW_BITFIELD_RANGE(8,15);
            uint32_t    Table1XFilterCoefficient5   : MHW_BITFIELD_RANGE(16,23);
            uint32_t    Table1YFilterCoefficient5   : MHW_BITFIELD_RANGE(24,31);
        };
        struct
        {
            uint32_t    Value;
        };
    } DW1;
} SFC_AVS_CHROMA_FILTER_COEFF, *PSFC_AVS_CHROMA_FILTER_COEFF;

//!
//! \brief  Structure to hold AVS Luma Coeff tables
//!
typedef struct _MHW_SFC_AVS_LUMA_TABLE
{
    uint8_t                      sfcPipeMode;                           //!< SFC Pipe Mode
    SFC_AVS_LUMA_FILTER_COEFF    LumaTable[NUM_HW_POLYPHASE_TABLES];
} MHW_SFC_AVS_LUMA_TABLE, *PMHW_SFC_AVS_LUMA_TABLE;

//!
//! \brief  Structure to hold AVS Chroma Coeff tables
//!
typedef struct _MHW_SFC_AVS_CHROMA_TABLE
{
    uint8_t                      sfcPipeMode;                           //!< SFC Pipe Mode
    SFC_AVS_CHROMA_FILTER_COEFF  ChromaTable[NUM_HW_POLYPHASE_TABLES];
} MHW_SFC_AVS_CHROMA_TABLE, *PMHW_SFC_AVS_CHROMA_TABLE;

//!
//! \brief  Structure to hold AVS State
//!
typedef struct _MHW_SFC_AVS_STATE
{
    uint8_t                         sfcPipeMode;                        //!< SFC Pipe Mode
    uint32_t                        dwInputHorizontalSiting;
    uint32_t                        dwInputVerticalSitting;
} MHW_SFC_AVS_STATE, *PMHW_SFC_AVS_STATE;

//!
//! \brief  SFC State Command parameters
//!
typedef struct _MHW_SFC_STATE_PARAMS
{
    uint8_t                         sfcPipeMode;                                //!< SFC Pipe Mode: VD-to-SFC or VE-to-SFC
    uint32_t                        dwVDVEInputOrderingMode;
    uint32_t                        dwInputChromaSubSampling;                   // Chroma subsampling at SFC input
    float                           fAlphaPixel;                                // Alpha pixel
    float                           fChromaSubSamplingXSiteOffset;              // Chroma siting X offset
    float                           fChromaSubSamplingYSiteOffset;              // Chroma siting Y offset
    uint32_t                        dwChromaDownSamplingMode;                   // Chroma Downsampling Mode
    uint32_t                        dwChromaDownSamplingVerticalCoef;           // Chomra Downsampling Vertical Coef
    uint32_t                        dwChromaDownSamplingHorizontalCoef;         // Chomra Downsampling Horizontal Coef

    uint32_t                        dwOutputFrameHeight;                        // Output Frame Height
    uint32_t                        dwOutputFrameWidth;                         // Output Frame Width
    MOS_FORMAT                      OutputFrameFormat;                          // Output Frame Format
    uint32_t                        dwInputFrameHeight;                         // Input Frame Height
    uint32_t                        dwInputFrameWidth;                          // Input Frame Width

    // Scaling parameters
    uint32_t                        dwAVSFilterMode;                            // Bilinear, 5x5 or 8x8
    uint32_t                        dwSourceRegionHeight;                       // Source/Crop region height
    uint32_t                        dwSourceRegionWidth;                        // Source/Crop region width
    uint32_t                        dwSourceRegionVerticalOffset;               // Source/Crop region vertical offset
    uint32_t                        dwSourceRegionHorizontalOffset;             // Source/Crop region horizontal offset
    uint32_t                        dwScaledRegionHeight;                       // Scaled region height
    uint32_t                        dwScaledRegionWidth;                        // Scaled region width
    uint32_t                        dwScaledRegionVerticalOffset;               // Scaled region vertical offset
    uint32_t                        dwScaledRegionHorizontalOffset;             // Scaled region horizontal offset
    float                           fAVSXScalingRatio;                          // X Scaling Ratio
    float                           fAVSYScalingRatio;                          // Y Scaling Ratio
    bool                            bBypassXAdaptiveFilter;                     // If true, X direction will use Default Sharpness level to blend
                                                                                // b/w smooth and sharp filters rather than the calculated value
    bool                            bBypassYAdaptiveFilter;                     // If true, Y direction will use Default Sharpness level to blend
                                                                                // b/w smooth and sharp filters rather than the calculated value
    bool                            bRGBAdaptive;                               // If true, Enable the RGB Adaptive filter
    // IEF params
    bool                            bIEFEnable;                                 // IEF Filter enable
    bool                            bSkinToneTunedIEFEnable;                    // Skin Tone Tuned IEF enable
    bool                            bAVSChromaUpsamplingEnable;                 // Up sample chroma prior to IEF filter
    bool                            b8tapChromafiltering;                       // This bit enables 8 tap filtering for Chroma Channels

    // Rotation Params
    MHW_ROTATION                    RotationMode;                               // Rotation mode -- 0, 90, 180 or 270
    uint32_t                        dwMirrorType;                               // Mirror Type -- vert/horiz
    bool                            bMirrorEnable;                              // Mirror mode -- enable/disable

    // ColorFill params
    bool                            bColorFillEnable;                           // ColorFill enable
    float                           fColorFillYRPixel;                          // ColorFill Y/R pixel
    float                           fColorFillUGPixel;                          // ColorFill U/G pixel
    float                           fColorFillVBPixel;                          // ColorFill V/B pixel
    float                           fColorFillAPixel;                           // ColorFill A pixel

    // CSC Params
    bool                            bCSCEnable;                                 // YUV->RGB/YUV->YUV CSC enable
    bool                            bRGBASwapEnable;                            // R, B Channel Swap enable
    bool                            bInputColorSpace;                           //0: YUV color space, 1:RGB color space

    // Memory compression Enable Flag
    bool                            bMMCEnable;                                 // Flag used to decide whether sfc output should be compressed
    MOS_RESOURCE_MMC_MODE           MMCMode;                                    // Memory compression mode

    // Resources used by SFC
    PMOS_RESOURCE                   pOsResOutputSurface;                        // Output Frame written by SFC
    PMOS_RESOURCE                   pOsResAVSLineBuffer;                        // AVS Line buffer used by SFC
    PMOS_RESOURCE                   pOsResIEFLineBuffer;                        // IEF Line buffer used by SFC

    uint32_t                        dwOutputSurfaceOffset;                      // Output Frame offset (page based offset)
    uint16_t                        wOutputSurfaceUXOffset;                     // Output Frame offset (page internal U offset for X axis)
    uint16_t                        wOutputSurfaceUYOffset;                     // Output Frame offset (page internal U offset for Y axis)
    uint16_t                        wOutputSurfaceVXOffset;                     // Output Frame offset (page internal V offset for X axis)
    uint16_t                        wOutputSurfaceVYOffset;                     // Output Frame offset (page internal V offset for Y axis)

} MHW_SFC_STATE_PARAMS, *PMHW_SFC_STATE_PARAMS;

//!
//! \brief  SFC Output Surface Command parameters
//!
typedef struct _MHW_SFC_OUT_SURFACE_PARAMS
{
    uint32_t                    ChromaSiting;       //!<  Chroma siting
    MOS_FORMAT                  Format;             //!<  Surface format
    uint32_t                    dwWidth;            //!<  Surface width
    uint32_t                    dwHeight;           //!<  Surface height
    uint32_t                    dwPitch;            //!<  Surface pitch
    MOS_TILE_TYPE               TileType;           //!<  Tile Type
    MOS_TILE_MODE_GMM           TileModeGMM;        //!<  Tile Type from GMM Definition
    bool                        bGMMTileEnabled;    //!<  GMM defined tile mode flag
    uint32_t                    dwStreamID;         //!<  Surface StreamID
    uint32_t                    dwSurfaceXOffset;   //!<  Surface X offset
    uint32_t                    dwSurfaceYOffset;   //!<  Surface Y offset
    uint32_t                    dwUYoffset;         //!<  Surface Uoffset in Vertical
    PMOS_RESOURCE               pOsResource;        //!<  Surface resource
    bool                        bCompressible;      //!<  Surface can be compressed
    uint32_t                    dwCompressionFormat;//!< Surface Compression format
} MHW_SFC_OUT_SURFACE_PARAMS, *PMHW_SFC_OUT_SURFACE_PARAMS;

//!
//! \brief  SFC Lock Command parameters
//!
typedef struct _MHW_SFC_LOCK_PARAMS
{
    uint8_t  sfcPipeMode;                                                        //!< SFC Pipe Mode
    uint32_t dwGaClientId;                                                       // Ga Client Id
    bool     bOutputToMemory;                                                    // Write Vebox or Vdbox o/p to memory
} MHW_SFC_LOCK_PARAMS, *PMHW_SFC_LOCK_PARAMS;

//!
//! \brief  SFC IEF State parameters
//!
typedef struct _MHW_SFC_IEF_STATE_PARAMS
{
    uint8_t                             sfcPipeMode;                            //!< SFC Pipe Mode

    // IEF params
    bool                                bSkinDetailFactor;                      // Skin Detail Factor
    bool                                bVYSTDEnable;                           // Enable STD in VY subspace
    bool                                bIEFEnable;                             // Enable IEF
    uint8_t                             StrongEdgeWeight;
    uint8_t                             RegularWeight;
    uint8_t                             StrongEdgeThreshold;
    uint32_t                            dwGainFactor;
    uint32_t                            dwR5xCoefficient;
    uint32_t                            dwR5cxCoefficient;
    uint32_t                            dwR5cCoefficient;
    uint32_t                            dwR3xCoefficient;
    uint32_t                            dwR3cCoefficient;

    // CSC params
    bool                                bCSCEnable;                             // Enable CSC transform
    float                               *pfCscCoeff;                             // [3x3] CSC Coeff matrix
    float                               *pfCscInOffset;                          // [3x1] CSC Input Offset matrix
    float                               *pfCscOutOffset;                         // [3x1] CSC Output Offset matrix
} MHW_SFC_IEF_STATE_PARAMS, *PMHW_SFC_IEF_STATE_PARAMS;

class MhwSfcInterface
{
public:
    virtual ~MhwSfcInterface()
    {
    }

    //!
    //! \brief    Adds a resource to the command buffer or indirect state (SSH)
    //! \details  Internal MHW function to add either a graphics address of a resource or
    //!           add the resource to the patch list for the requested buffer or state
    //! \param    [in] pOsInterface
    //!           OS interface
    //! \param    [in] pCmdBuffer
    //!           If adding a resource to the command buffer, the buffer to which the resource
    //!           is added
    //! \param    [in] pParams
    //!           Parameters necessary to add the graphics address
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS(*pfnAddResourceToCmd) (
        PMOS_INTERFACE                 pOsInterface,
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_RESOURCE_PARAMS           pParams);

    //!
    //! \brief      Add SFC Lock
    //! \param      [in] pCmdBuffer
    //!             Pointer to Command buffer
    //! \param      [in] pSfcLockParams
    //!              Pointer to SFC_LOCK params
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS AddSfcLock (
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_LOCK_PARAMS           pSfcLockParams) = 0;

    //!
    //! \brief    Add SFC State
    //! \param    [in] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \param    [in] pSfcStateParams
    //!           Pointer to SFC_STATE params
    //! \param    [in] pOutSurface
    //!           Pointer to Output surface params
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS AddSfcState(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_STATE_PARAMS          pSfcStateParams,
        PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface) = 0;

    //!
    //! \brief      Add SFC AVS State command
    //! \param      [in] pCmdBuffer
    //!             Pointer to Command Buffer
    //! \param      [in] pSfcAvsState
    //!             Pointer to SFC AVS State
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS AddSfcAvsState(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_AVS_STATE             pSfcAvsState) = 0;

    //!
    //! \brief      Add SFC Frame Start command
    //! \param      [in] pCmdBuffer
    //!             Pointer to Command Buffer
    //! \param      [in] sfcPipeMode
    //!             SFC pipe mode
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS AddSfcFrameStart (
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        uint8_t                        sfcPipeMode) = 0;

    //!
    //! \brief      Add SFC IEF State command
    //! \param      [in] pCmdBuffer
    //!             Pointer to Command Buffer
    //! \param      [in] pSfcIefStateParams
    //!             Pointer to IEF State params
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS AddSfcIefState(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_IEF_STATE_PARAMS      pSfcIefStateParams) = 0;

    //!
    //! \brief      Add SFC AVS Chroma Table command
    //! \param      [in] pCmdBuffer
    //!             Pointer to Command Buffer
    //! \param      [in] pChromaTable
    //!             Pointer to Chroma Coefficient table
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS AddSfcAvsChromaTable (
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_AVS_CHROMA_TABLE      pChromaTable) = 0;

    //!
    //! \brief      Add SFC AVS Chroma Table command
    //! \param      [in] pCmdBuffer
    //!             Pointer to Command Buffer
    //! \param      [in] pChromaTable
    //!             Pointer to Chroma Coefficient table
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS AddSfcAvsLumaTable (
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_AVS_LUMA_TABLE        pLumaTable) = 0;

    //!
    //! \brief      Set Sfc Sampler8x8 Table
    //! \details    Set Sfc AVS Luma and Chroma table
    //! \param      [out] pLumaTable
    //!             Pointer to AVS luma table
    //! \param      [out] pChromaTable
    //!             Pointer to AVS chroma table
    //! \param      [in] pAvsParams
    //!             Pointer to AVS params
    //! \param      [in] SrcFormat
    //!             Input Source Format
    //! \param      [in] fScaleX
    //!             Scaling ratio in width
    //! \param      [in] fScaleY
    //!             Scaling ratio in height
    //! \param      [in] dwChromaSiting
    //!             Chroma Siting info
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS SetSfcSamplerTable (
        PMHW_SFC_AVS_LUMA_TABLE         pLumaTable,
        PMHW_SFC_AVS_CHROMA_TABLE       pChromaTable,
        PMHW_AVS_PARAMS                 pAvsParams,
        MOS_FORMAT                      SrcFormat,
        float                           fScaleX,
        float                           fScaleY,
        uint32_t                        dwChromaSiting,
        bool                            bUse8x8Filter);

protected:

    MhwSfcInterface(PMOS_INTERFACE pOsInterface);

    //!
    //! \brief      Sets AVS Luma Coefficient table
    //! \param      [in] SrcFormat
    //!             Source format
    //! \param      [in] pCoeffTable
    //!             Pointer to coefficient table
    //! \param      [in] piYCoefsX
    //!             Pointer to Y coefficients X
    //! \param      [in] piYCoefsY
    //!             Pointer to Y coefficients Y
    //! \param      [in] bUse8x8Filter
    //!             Is 8x8 Filter used
    //! \return     void
    //!
    void SetSfcAVSLumaTable(
        MOS_FORMAT                      SrcFormat,
        PSFC_AVS_LUMA_FILTER_COEFF      pCoeffTable,
        int32_t                         *piYCoefsX,
        int32_t                         *piYCoefsY,
        bool                            bUse8x8Filter);
    //!
    //! \brief      Sets AVS Chroma Coefficient table
    //! \param      [in] pUVCoeffTable
    //!             Pointer to UV coefficient table
    //! \param      [in] piUVCoefsX
    //!             Pointer to UV coefficients X
    //! \param      [in] piUVCoefsY
    //!             Pointer to UV coefficients Y
    //! \return     void
    //!
    void SetSfcAVSChromaTable(
        PSFC_AVS_CHROMA_FILTER_COEFF        pUVCoeffTable,
        int32_t                             *piUVCoefsX,
        int32_t                             *piUVCoefsY);

public:
    enum SFC_PIPE_MODE
    {
        SFC_PIPE_MODE_VDBOX = 0,
        SFC_PIPE_MODE_VEBOX = 1
    };

public:
    PMOS_INTERFACE                             m_osInterface       = nullptr;

    uint16_t                                   m_veWidthAlignment  = MHW_SFC_VE_WIDTH_ALIGN;
    uint16_t                                   m_veHeightAlignment = MHW_SFC_VE_HEIGHT_ALIGN;
    uint32_t                                   m_maxWidth          = MHW_SFC_MAX_WIDTH;
    uint32_t                                   m_maxHeight         = MHW_SFC_MAX_HEIGHT;
    uint32_t                                   m_minWidth          = MHW_SFC_MIN_WIDTH;
    uint32_t                                   m_minHeight         = MHW_SFC_MIN_HEIGHT;

    float                                      m_maxScalingRatio   = MHW_SFC_MAX_SCALINGFACTOR;
    float                                      m_minScalingRatio   = MHW_SFC_MIN_SCALINGFACTOR;

    MHW_MEMORY_OBJECT_CONTROL_PARAMS           m_outputSurfCtrl;          // Output Frame caching control bits
    MHW_MEMORY_OBJECT_CONTROL_PARAMS           m_avsLineBufferCtrl;       // AVS Line Buffer caching control bits
    MHW_MEMORY_OBJECT_CONTROL_PARAMS           m_iefLineBufferCtrl;       // IEF Line Buffer caching control bits
};

#endif // __MHW_SFC_H__
