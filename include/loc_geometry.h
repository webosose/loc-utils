// Copyright (c) 2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef _LOC_GEOMETRY_H_
#define _LOC_GEOMETRY_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UTM_HEMISPHERE_NORTHERN,
    UTM_HEMISPHERE_SOUTHERN
} UTM_HEMISPHERE;

typedef struct {
    double latitude;
    double longitude;
} GEOCoordinates;

typedef struct {
    double northing;
    double easting;
    unsigned int grid_zone;
    UTM_HEMISPHERE hemisphere;
} UTMCoordinates;


typedef struct _RTCEPCalculator     RTCEPCalculator;



/*
 * Geometric Math
 */

// Radians to Degrees
double loc_geometry_radians_to_degrees(double radians);

// Degrees to Radians
double loc_geometry_degrees_to_radians(double degrees);

// Calculate Distance between coordinates 1 and coordinates 2
double loc_geometry_calc_distance(double lat1, double lon1, double lat2, double lon2);





/*
 * Conversion: Geographic Coordinates vs UTM (Universal Transverse Mercator) Coordinates
 */

// WGS84 to UTM in Meters
UTMCoordinates loc_geometry_convert_wgs84_to_utm(GEOCoordinates wgs84);

// UTM to WGS84 in Degrees
GEOCoordinates loc_geometry_convert_utm_to_wgs84(UTMCoordinates utm);





/*
 * Circular Error Probable Calculation
 * measures:     Array of measured geographic (WGS84) coordinates in degrees
 * size:         Size of array
 * ref_position: Known position (if this is set to NULL, the Mean of measured positions will be used)
 */

// CEP (Circular Error Probability)
// Probability: 50%
double loc_geometry_calculate_cep(GEOCoordinates *measures, int size, GEOCoordinates *ref_position);

// DRMS (Distance Root Mean Square)
// Probability: 63% ~ 68%
double loc_geometry_calculate_drms(GEOCoordinates *measures, int size, GEOCoordinates *ref_position);

// 2DRMS (Twice the Distance Root Mean Square)
// Probability: 95% ~ 98%
double loc_geometry_calculate_2drms(GEOCoordinates *measures, int size, GEOCoordinates *ref_position);

// R95 (Radius 95%) = CEP 95%
// Probability: 95%
double loc_geometry_calculate_r95(GEOCoordinates *measures, int size, GEOCoordinates *ref_position);





/*
 * Real Time CEP Calculator
 * To support the CEP calculation for long time with the known reference position,
 * real time calculator for CEP is needed for considering of memory and overflow.
 */

// Create Real Time CEP Calculator with a given reference position
RTCEPCalculator* loc_geometry_rtcep_create(GEOCoordinates *ref_position);

// Destroy CEP Calculator
void loc_geometry_rtcep_destroy(RTCEPCalculator **rtcep_ref);

// Reset all the saved components
// To reset maximum number of measurement, use loc_geometry_rtcep_set_max_count.
void loc_geometry_rtcep_reset(RTCEPCalculator **rtcep_ref);

// Set maximum number of measurement
// Passing 0 means infinite counting. (not checking limit of count)
// If count reaches max_count, all the internal calculation will be stopped.
void loc_geometry_rtcep_set_max_count(RTCEPCalculator **rtcep_ref, int max_count);

// Update CEP calculation with a given measured position
void loc_geometry_rtcep_update(RTCEPCalculator **rtcep_ref, GEOCoordinates *measured_position);

// Get Reference Position
GEOCoordinates* loc_geometry_rtcep_get_ref_position(RTCEPCalculator **rtcep_ref);

// Get Maximum Count
int loc_geometry_rtcep_get_max_count(RTCEPCalculator **rtcep_ref);

// Get Current Count
int loc_geometry_rtcep_get_current_count(RTCEPCalculator **rtcep_ref);

// Get the current CEP(50%) value
double loc_geometry_rtcep_get_cep(RTCEPCalculator **rtcep_ref);

// Get the current DRMS(63% ~ 68%) value
double loc_geometry_rtcep_get_drms(RTCEPCalculator **rtcep_ref);

// Get the current 2DRMS(95% ~ 98%) value
double loc_geometry_rtcep_get_2drms(RTCEPCalculator **rtcep_ref);

// Get the current R95(95%) value
double loc_geometry_rtcep_get_r95(RTCEPCalculator **rtcep_ref);



#ifdef __cplusplus
}
#endif

#endif // _LOC_GEOMETRY_H_
