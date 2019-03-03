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
#include "cm_def.h"

CM_AVS_STATE_MSG_EX::CM_AVS_STATE_MSG_EX():
    enableAllChannelAdaptiveFilter(false),
    enableRgbAdaptiveFilter(false),
    enable8TapAdaptiveFilter(false),
    enableUV8TapFilter(false),
    writebackFormat(true),  // Results are returned in "sampleunorm" format.
    writebackMode(true),  // AVS mode.
    stateSelection(0),  // scaler8x8 state 0 is used.

    enableIef(false),  // No image enhancement filtering.
    iefType(false),
    enableIefSmooth(false),
    r3cCoefficient(0.46875f),
    r3xCoefficient(0.1875f),
    r5cCoefficient(0.09375f),
    r5cxCoefficient(0.25f),
    r5xCoefficient(0.28125f),

    strongEdgeThreshold(8),
    strongEdgeWeight(6),
    weakEdgeThreshold(1),
    regularEdgeWeight(3),
    nonEdgeWeight(2),

    enableChromaKey(false),
    chromaKeyIndex(0),

    enableSkinTone(false),
    enableVySkinToneDetection(false),
    skinDetailFactor(false),
    skinTypesMargin(20),
    skinTypesThreshold(120),

    gainFactor(32),
    globalNoiseEstimation(0),
    mrBoost(false),
    mrSmoothThreshold(2),
    mrThreshold(5),
    steepnessBoost(false),
    steepnessThreshold(0),
    textureCoordinateMode(true),  // Clamps to edge.
    maxHue(14),
    maxSaturation(31),
    angles(52),
    diamondMargin(4),
    diamondDu(0),
    diamondDv(0),
    diamondAlpha(1.5625f),
    diamondThreshold(35),
    rectangleMargin(3),

    defaultSharpnessLevel(255),
    enableXAdaptiveFilter(true),
    enableYAdaptiveFilter(true),
    maxDerivative4Pixels(0),
    maxDerivative8Pixels(20),
    transitionArea4Pixels(3),
    transitionArea8Pixels(5)
{
  rectangleMidpoint[0] = 110;
  rectangleMidpoint[1] = 154;
  vyInverseMargin[0] = 3300.0f/65536.0f;
  vyInverseMargin[1] = 1600.0f/65536.0f;
  piecewiseLinearYPoints[0] = 46;
  piecewiseLinearYPoints[1] = 47;
  piecewiseLinearYPoints[2] = 254;
  piecewiseLinearYPoints[3] = 255;
  piecewiseLinearYSlopes[0] = 31.0f/8.0f;
  piecewiseLinearYSlopes[1] = 31.0f/8.0f;
  piecewiseLinearPointsLower[0] = 46;
  piecewiseLinearPointsLower[1] = 216;
  piecewiseLinearPointsLower[2] = 236;
  piecewiseLinearPointsLower[3] = 236;
  piecewiseLinearPointsUpper[0] = 46;
  piecewiseLinearPointsUpper[1] = 66;
  piecewiseLinearPointsUpper[2] = 150;
  piecewiseLinearPointsUpper[3] = 236;
  piecewiseLinearSlopesLower[0] = -5.0f/256.0f;
  piecewiseLinearSlopesLower[1] =  piecewiseLinearSlopesLower[2]
      =  piecewiseLinearSlopesLower[3] = 0;
  piecewiseLinearSlopesUpper[0] = 1.0f;
  piecewiseLinearSlopesUpper[1] = 113.0f/256.0f;
  piecewiseLinearSlopesUpper[2] = -179.0f/256.0f;
  piecewiseLinearSlopesUpper[3] = 0;
  piecewiseLinearBiasesLower[0] = 133;
  piecewiseLinearBiasesLower[1] = piecewiseLinearBiasesLower[2]
      = piecewiseLinearBiasesLower[3] = 130;
  piecewiseLinearBiasesUpper[0] = 143;
  piecewiseLinearBiasesUpper[1] = 163;
  piecewiseLinearBiasesUpper[2] = 200;
  piecewiseLinearBiasesUpper[3] = 140;

  for (int i = 0; i < CM_NUM_COEFF_ROWS_SKL; ++i)  {
    // Y horizontal
    table0X[i].filterCoeff_0_0 = table0X[i].filterCoeff_0_1
        = table0X[i].filterCoeff_0_2 = table0X[i].filterCoeff_0_3
        = table0X[i].filterCoeff_0_4 = table0X[i].filterCoeff_0_5
        = table0X[i].filterCoeff_0_6 = table0X[i].filterCoeff_0_7 = 0.125f;
    // Y vertical
    table0Y[i].filterCoeff_0_0 = table0Y[i].filterCoeff_0_1
        = table0Y[i].filterCoeff_0_2 = table0Y[i].filterCoeff_0_3
        = table0Y[i].filterCoeff_0_4 = table0Y[i].filterCoeff_0_5
        = table0Y[i].filterCoeff_0_6 = table0Y[i].filterCoeff_0_7 = 0.125f;
    // UV horizontal
    table1X[i].filterCoeff_0_0 = table1X[i].filterCoeff_0_1
        = table1X[i].filterCoeff_0_2 = table1X[i].filterCoeff_0_3
        = table1X[i].filterCoeff_0_4 = table1X[i].filterCoeff_0_5
        = table1X[i].filterCoeff_0_6 = table1X[i].filterCoeff_0_7 = 0.125f;
    // UV vertical
    table1Y[i].filterCoeff_0_0 = table1Y[i].filterCoeff_0_1
        = table1Y[i].filterCoeff_0_2 = table1Y[i].filterCoeff_0_3
        = table1Y[i].filterCoeff_0_4 = table1Y[i].filterCoeff_0_5
        = table1Y[i].filterCoeff_0_6 = table1Y[i].filterCoeff_0_7 = 0.125f;
  }
  return;
}//------
