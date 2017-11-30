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
    enable_all_channel_adaptive_filter(false),
    enable_rgb_adaptive_filter(false),
    enable_8_tap_adaptive_filter(false),
    enable_uv_8_tap_filter(false),
    writeback_format(true),  // Results are returned in "sampleunorm" format.
    writeback_mode(true),  // AVS mode.
    state_selection(0),  // scaler8x8 state 0 is used.

    enable_ief(false),  // No image enhancement filtering.
    ief_type(false),
    enable_ief_smooth(false),
    r3c_coefficient(0.46875f),
    r3x_coefficient(0.1875f),
    r5c_coefficient(0.09375f),
    r5cx_coefficient(0.25f),
    r5x_coefficient(0.28125f),

    strong_edge_threshold(8),
    strong_edge_weight(6),
    weak_edge_threshold(1),
    regular_edge_weight(3),
    non_edge_weight(2),

    enable_chroma_key(false),
    chroma_key_index(0),

    enable_skin_tone(false),
    enable_vy_skin_tone_detection(false),
    skin_detail_factor(false),
    skin_types_margin(20),
    skin_types_threshold(120),

    gain_factor(32),
    global_noise_estimation(0),
    mr_boost(false),
    mr_smooth_threshold(2),
    mr_threshold(5),
    steepness_boost(false),
    steepness_threshold(0),
    texture_coordinate_mode(true),  // Clamps to edge.
    max_hue(14),
    max_saturation(31),
    angles(52),
    diamond_margin(4),
    diamond_du(0),
    diamond_dv(0),
    diamond_alpha(1.5625f),
    diamond_threshold(35),
    rectangle_margin(3),

    default_sharpness_level(255),
    enable_x_adaptive_filter(true),
    enable_y_adaptive_filter(true),
    max_derivative_4_pixels(0),
    max_derivative_8_pixels(20),
    transition_area_4_pixels(3),
    transition_area_8_pixels(5)
{
  rectangle_midpoint[0] = 110;
  rectangle_midpoint[1] = 154;
  vy_inverse_margin[0] = 3300.0f/65536.0f;
  vy_inverse_margin[1] = 1600.0f/65536.0f;
  piecewise_linear_y_points[0] = 46;
  piecewise_linear_y_points[1] = 47;
  piecewise_linear_y_points[2] = 254;
  piecewise_linear_y_points[3] = 255;
  piecewise_linear_y_slopes[0] = 31.0f/8.0f;
  piecewise_linear_y_slopes[1] = 31.0f/8.0f;
  piecewise_linear_points_lower[0] = 46;
  piecewise_linear_points_lower[1] = 216;
  piecewise_linear_points_lower[2] = 236;
  piecewise_linear_points_lower[3] = 236;
  piecewise_linear_points_upper[0] = 46;
  piecewise_linear_points_upper[1] = 66;
  piecewise_linear_points_upper[2] = 150;
  piecewise_linear_points_upper[3] = 236;
  piecewise_linear_slopes_lower[0] = -5.0f/256.0f;
  piecewise_linear_slopes_lower[1] =  piecewise_linear_slopes_lower[2]
      =  piecewise_linear_slopes_lower[3] = 0;
  piecewise_linear_slopes_upper[0] = 1.0f;
  piecewise_linear_slopes_upper[1] = 113.0f/256.0f;
  piecewise_linear_slopes_upper[2] = -179.0f/256.0f;
  piecewise_linear_slopes_upper[3] = 0;
  piecewise_linear_biases_lower[0] = 133;
  piecewise_linear_biases_lower[1] = piecewise_linear_biases_lower[2]
      = piecewise_linear_biases_lower[3] = 130;
  piecewise_linear_biases_upper[0] = 143;
  piecewise_linear_biases_upper[1] = 163;
  piecewise_linear_biases_upper[2] = 200;
  piecewise_linear_biases_upper[3] = 140;

  for (int i = 0; i < CM_NUM_COEFF_ROWS_SKL; ++i)  {
    // Y horizontal
    table0_x[i].FilterCoeff_0_0 = table0_x[i].FilterCoeff_0_1
        = table0_x[i].FilterCoeff_0_2 = table0_x[i].FilterCoeff_0_3
        = table0_x[i].FilterCoeff_0_4 = table0_x[i].FilterCoeff_0_5
        = table0_x[i].FilterCoeff_0_6 = table0_x[i].FilterCoeff_0_7 = 0.125f;
    // Y vertical
    table0_y[i].FilterCoeff_0_0 = table0_y[i].FilterCoeff_0_1
        = table0_y[i].FilterCoeff_0_2 = table0_y[i].FilterCoeff_0_3
        = table0_y[i].FilterCoeff_0_4 = table0_y[i].FilterCoeff_0_5
        = table0_y[i].FilterCoeff_0_6 = table0_y[i].FilterCoeff_0_7 = 0.125f;
    // UV horizontal
    table1_x[i].FilterCoeff_0_0 = table1_x[i].FilterCoeff_0_1
        = table1_x[i].FilterCoeff_0_2 = table1_x[i].FilterCoeff_0_3
        = table1_x[i].FilterCoeff_0_4 = table1_x[i].FilterCoeff_0_5
        = table1_x[i].FilterCoeff_0_6 = table1_x[i].FilterCoeff_0_7 = 0.125f;
    // UV vertical
    table1_y[i].FilterCoeff_0_0 = table1_y[i].FilterCoeff_0_1
        = table1_y[i].FilterCoeff_0_2 = table1_y[i].FilterCoeff_0_3
        = table1_y[i].FilterCoeff_0_4 = table1_y[i].FilterCoeff_0_5
        = table1_y[i].FilterCoeff_0_6 = table1_y[i].FilterCoeff_0_7 = 0.125f;
  }
  return;
}//------
