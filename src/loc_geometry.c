// Copyright (c) 2020-2021 LG Electronics, Inc.
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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <loc_geometry.h>

#define MATH_PI                     3.1415926535897932384626433832795
#define WGS84_EQUITORIAL_RADIUS     6378137
#define WGS84_POLAR_RADIUS          6356752.3142

const double utm_scale_factor = 0.9996;


struct _RTCEPCalculator {
    GEOCoordinates ref_geo_pos;
    UTMCoordinates ref_utm_pos;
    double lamda_x;
    double lamda_y;
    int max_count;
    int count;
};


double arc_length_of_meridian(double latitudeInRadians);
double footpoint_latitude(double northingInMeters);
UTMCoordinates wgs84_to_tm(GEOCoordinates radCoordinates, double centralMeridian);
GEOCoordinates tm_to_wgs84(UTMCoordinates tm, double centralMeridian);
UTMCoordinates get_avg_utm(GEOCoordinates *measures, int size);





double loc_geometry_radians_to_degrees(double radians)
{
    return radians * 180 / MATH_PI;
}

double loc_geometry_degrees_to_radians(double degrees)
{
    return degrees / 180 * MATH_PI;
}

double loc_geometry_calc_distance(double lat1, double lon1, double lat2, double lon2)
{
    double lambdaP, iter_limit = 100.0;
    double sin_sigma, sin_alpha, cos_sigma, sigma,  sq_cos_alpha, cos_2sigma, C;
    double sq_u, cal1, cal2, delta_sigma, cal_dist;
    double sin_lambda, cos_lambda;

    const double a = 6378137.0;
    const double b = 6356752.314245;
    const double f = 1 / 298.257223563;
    double delta_lon = ((lon2 - lon1) * MATH_PI / 180);
    double u_1 = atan((1 - f) * tan((lat1) * MATH_PI / 180));
    double u_2 = atan((1 - f) * tan((lat2) * MATH_PI / 180));

    double lambda = delta_lon;
    double sin_u1 = sin(u_1);
    double cos_u1 = cos(u_1);
    double sin_u2 = sin(u_2);
    double cos_u2 = cos(u_2);

    do {
        sin_lambda = sin(lambda);
        cos_lambda = cos(lambda);
        sin_sigma = sqrt((cos_u2 * sin_lambda) * (cos_u2 * sin_lambda) +
                         (cos_u1 * sin_u2 - sin_u1 * cos_u2 * cos_lambda) *
                         (cos_u1 * sin_u2 - sin_u1 * cos_u2 * cos_lambda));

        // co-incident points
        if (sin_sigma == 0)
            return 0;

        cos_sigma = sin_u1 * sin_u2 + cos_u1 * cos_u2 * cos_lambda;
        sigma = atan2(sin_sigma, cos_sigma);
        sin_alpha = cos_u1 * cos_u2 * sin_lambda / sin_sigma;
        sq_cos_alpha = 1.0 - sin_alpha * sin_alpha;
        cos_2sigma = cos_sigma - 2.0 * sin_u1 * sin_u2 / sq_cos_alpha;

        if (isnan(cos_2sigma))
            cos_2sigma = 0;

        C = f / 16.0 * sq_cos_alpha * (4.0 + f * (4.0 - 3.0 * sq_cos_alpha));
        lambdaP = lambda;
        lambda = delta_lon + (1.0 - C) * f * sin_alpha *
                 (sigma + C * sin_sigma * (cos_2sigma + C * cos_sigma * (-1.0 + 2.0 * cos_2sigma * cos_2sigma)));
    } while (abs(lambda - lambdaP) > 1e-12 && --iter_limit > 0);

    if (iter_limit == 0)
        return 0;

    sq_u = sq_cos_alpha * (a * a - b * b) / (b * b);
    cal1 = 1.0 + sq_u / 16384.0 * (4096.0 + sq_u * (-768.0 + sq_u * (320.0 - 175.0 * sq_u)));
    cal2 = sq_u / 1024.0 * (256.0 + sq_u * (-128.0 + sq_u * (74.0 - 47.0 * sq_u)));
    delta_sigma = cal2 * sin_sigma * (cos_2sigma + cal2 / 4.0 * (cos_sigma * (-1.0 + 2.0 * cos_2sigma * cos_2sigma) -
                cal2 / 6.0 * cos_2sigma * (-3.0 + 4.0 * sin_sigma * sin_sigma) * (-3.0 + 4.0 * cos_2sigma * cos_2sigma)));

    cal_dist = b * cal1 * (sigma - delta_sigma);

    return cal_dist;
}

UTMCoordinates loc_geometry_convert_wgs84_to_utm(GEOCoordinates wgs84)
{
    UTMCoordinates utmCoordinates;
    UTM_HEMISPHERE hemisphere;
    double cmeridian, x, y;
    unsigned int zone;

    zone = floor((wgs84.longitude + 180.0) / 6) + 1;

    if (wgs84.latitude < 0) {
        hemisphere = UTM_HEMISPHERE_SOUTHERN;
    } else {
        hemisphere = UTM_HEMISPHERE_NORTHERN;
    }

    cmeridian = loc_geometry_degrees_to_radians(-183.0 + (zone * 6.0));

    wgs84.latitude = loc_geometry_degrees_to_radians(wgs84.latitude);
    wgs84.longitude = loc_geometry_degrees_to_radians(wgs84.longitude);

    utmCoordinates = wgs84_to_tm(wgs84, cmeridian);

    x = utmCoordinates.easting * utm_scale_factor + 500000.0;
    y = utmCoordinates.northing * utm_scale_factor;
    if (y < 0.0) {
        y = y + 10000000.0;
    }

    utmCoordinates.easting = x;
    utmCoordinates.northing = y;
    utmCoordinates.grid_zone = zone;
    utmCoordinates.hemisphere = hemisphere;

    return utmCoordinates;
}

GEOCoordinates loc_geometry_convert_utm_to_wgs84(UTMCoordinates utm)
{
    GEOCoordinates geoCoordinates;
    UTM_HEMISPHERE hemisphere;
    double cmeridian, x, y;
    unsigned int zone;

    x = utm.easting;
    y = utm.northing;
    zone = utm.grid_zone;
    hemisphere = utm.hemisphere;

    x -= 500000.0;
    x /= utm_scale_factor;

    if (hemisphere == UTM_HEMISPHERE_SOUTHERN) {
        y -= 10000000.0;
    }

    y /= utm_scale_factor;

    utm.easting = x;
    utm.northing = y;

    cmeridian = loc_geometry_degrees_to_radians(-183.0 + (zone * 6.0));

    geoCoordinates = tm_to_wgs84(utm, cmeridian);

    geoCoordinates.latitude = loc_geometry_radians_to_degrees(geoCoordinates.latitude);
    geoCoordinates.longitude = loc_geometry_radians_to_degrees(geoCoordinates.longitude);

    return geoCoordinates;
}

double loc_geometry_calculate_cep(GEOCoordinates *measures, int size, GEOCoordinates *ref_position)
{
    UTMCoordinates utmRef, utmMeasured;
    double result;
    double sigmaX, sigmaY;
    int i;

    if (!measures || size <= 0)
        return 0;

    result = sigmaX = sigmaY = 0;

    if (ref_position) {
        utmRef = loc_geometry_convert_wgs84_to_utm(*ref_position);
    } else {
        utmRef = get_avg_utm(measures, size);
    }

    for (i=0; i<size; i++) {
        utmMeasured = loc_geometry_convert_wgs84_to_utm(measures[i]);

        sigmaX += pow((utmMeasured.easting - utmRef.easting), 2.0);
        sigmaY += pow((utmMeasured.northing - utmRef.northing), 2.0);
    }

    sigmaX = sqrt(sigmaX / (double)size);
    sigmaY = sqrt(sigmaY / (double)size);

    result = (0.56 * sigmaX) + (0.62 * sigmaY);

    return result;
}

double loc_geometry_calculate_drms(GEOCoordinates *measures, int size, GEOCoordinates *ref_position)
{
    UTMCoordinates utmRef, utmMeasured;
    double result;
    double sigmaX, sigmaY;
    int i;

    if (!measures || size <= 0)
        return 0;

    result = sigmaX = sigmaY = 0;

    if (ref_position) {
        utmRef = loc_geometry_convert_wgs84_to_utm(*ref_position);
    } else {
        utmRef = get_avg_utm(measures, size);
    }

    for (i=0; i<size; i++) {
        utmMeasured = loc_geometry_convert_wgs84_to_utm(measures[i]);

        sigmaX += pow((utmMeasured.easting - utmRef.easting), 2.0);
        sigmaY += pow((utmMeasured.northing - utmRef.northing), 2.0);
    }

    sigmaX = sigmaX / (double)size;
    sigmaY = sigmaY / (double)size;

    result = sqrt(sigmaX + sigmaY);

    return result;
}

double loc_geometry_calculate_2drms(GEOCoordinates *measures, int size, GEOCoordinates *ref_position)
{
    double result;

    result = 2.0 * loc_geometry_calculate_drms(measures, size, ref_position);

    return result;
}

double loc_geometry_calculate_r95(GEOCoordinates *measures, int size, GEOCoordinates *ref_position)
{
    double result;

    result = 2.08 * loc_geometry_calculate_cep(measures, size, ref_position);

    return result;
}

RTCEPCalculator* loc_geometry_rtcep_create(GEOCoordinates *ref_position)
{
    RTCEPCalculator *rtcep = NULL;

    if (!ref_position)
        return NULL;

    rtcep = (RTCEPCalculator *)malloc(sizeof(RTCEPCalculator));
    if (rtcep) {
        memset(rtcep, 0, sizeof(RTCEPCalculator));
        memcpy(&(rtcep->ref_geo_pos), ref_position, sizeof(GEOCoordinates));
        rtcep->ref_utm_pos = loc_geometry_convert_wgs84_to_utm(*ref_position);
    }

    return rtcep;
}

void loc_geometry_rtcep_destroy(RTCEPCalculator **rtcep_ref)
{
    RTCEPCalculator *rtcep = *rtcep_ref;

    if (!rtcep)
        return;

    free(rtcep);
    rtcep = NULL;
}

void loc_geometry_rtcep_reset(RTCEPCalculator **rtcep_ref)
{
    RTCEPCalculator *rtcep = *rtcep_ref;

    if (!rtcep)
        return;

    rtcep->lamda_x = 0;
    rtcep->lamda_y = 0;
    rtcep->count = 0;
}

void loc_geometry_rtcep_set_max_count(RTCEPCalculator **rtcep_ref, int max_count)
{
    RTCEPCalculator *rtcep = *rtcep_ref;

    if (!rtcep)
        return;

    rtcep->max_count = max_count;
}

void loc_geometry_rtcep_update(RTCEPCalculator **rtcep_ref, GEOCoordinates *measured_position)
{
    RTCEPCalculator *rtcep = *rtcep_ref;
    UTMCoordinates utmMeasured;

    if (!rtcep)
        return;

    if (!measured_position)
        return;

    if (rtcep->max_count > 0 && rtcep->count >= rtcep->max_count)
        return;

    utmMeasured = loc_geometry_convert_wgs84_to_utm(*measured_position);

    rtcep->lamda_x += pow((utmMeasured.easting - rtcep->ref_utm_pos.easting), 2.0);
    rtcep->lamda_y += pow((utmMeasured.northing - rtcep->ref_utm_pos.northing), 2.0);

    rtcep->count++;
}

GEOCoordinates* loc_geometry_rtcep_get_ref_position(RTCEPCalculator **rtcep_ref)
{
    RTCEPCalculator *rtcep = *rtcep_ref;

    if (!rtcep)
        return NULL;

    return &(rtcep->ref_geo_pos);
}

int loc_geometry_rtcep_get_max_count(RTCEPCalculator **rtcep_ref)
{
    RTCEPCalculator *rtcep = *rtcep_ref;

    if (!rtcep)
        return 0;

    return rtcep->max_count;
}

int loc_geometry_rtcep_get_current_count(RTCEPCalculator **rtcep_ref)
{
    RTCEPCalculator *rtcep = *rtcep_ref;

    if (!rtcep)
        return 0;

    return rtcep->count;
}

double loc_geometry_rtcep_get_cep(RTCEPCalculator **rtcep_ref)
{
    double sigma_x, sigma_y, result;
    RTCEPCalculator *rtcep = *rtcep_ref;

    if (!rtcep)
        return 0;

    sigma_x = sqrt(rtcep->lamda_x / (double)rtcep->count);
    sigma_y = sqrt(rtcep->lamda_y / (double)rtcep->count);

    result = (0.56 * sigma_x) + (0.62 * sigma_y);

    return result;
}

double loc_geometry_rtcep_get_drms(RTCEPCalculator **rtcep_ref)
{
    double sigma_x, sigma_y, result;
    RTCEPCalculator *rtcep = *rtcep_ref;

    if (!rtcep)
        return 0;

    sigma_x = rtcep->lamda_x / (double)rtcep->count;
    sigma_y = rtcep->lamda_y / (double)rtcep->count;

    result = sqrt(sigma_x + sigma_y);

    return result;
}

double loc_geometry_rtcep_get_2drms(RTCEPCalculator **rtcep_ref)
{
    double result;

    result = 2.0 * loc_geometry_rtcep_get_drms(rtcep_ref);

    return result;
}

double loc_geometry_rtcep_get_r95(RTCEPCalculator **rtcep_ref)
{
    double result;

    result = 2.08 * loc_geometry_rtcep_get_cep(rtcep_ref);

    return result;
}

double arc_length_of_meridian(double latitudeInRadians)
{
    double alpha;
    double beta;
    double gamma;
    double delta;
    double epsilon;
    double n;
    double result;

    double equitorialRadus = WGS84_EQUITORIAL_RADIUS;
    double polarRadius = WGS84_POLAR_RADIUS;

    n = (equitorialRadus - polarRadius) / (equitorialRadus + polarRadius);

    alpha = ((equitorialRadus + polarRadius) / 2.0) * (1.0 + (pow(n, 2.0) / 4.0) + (pow(n, 4.0) / 64.0));

    beta = (-3.0 * n / 2.0) + (9.0 * pow(n, 3.0) / 16.0) + (-3.0 * pow(n, 5.0) / 32.0);

    gamma = (15.0 * pow(n, 2.0) / 16.0) + (-15.0 * pow(n, 4.0) / 32.0);

    delta = (-35.0 * pow(n, 3.0) / 48.0) + (105.0 * pow(n, 5.0) / 256.0);

    epsilon = (315.0 * pow(n, 4.0) / 512.0);

    result = alpha * (latitudeInRadians +
                      (beta * sin(2.0 * latitudeInRadians)) +
                      (gamma * sin(4.0 * latitudeInRadians)) +
                      (delta * sin(6.0 * latitudeInRadians)) +
                      (epsilon * sin(8.0 * latitudeInRadians)));

    return result;
}

double footpoint_latitude(double northingInMeters)
{
    double y;
    double alpha;
    double beta;
    double gamma;
    double delta;
    double epsilon;
    double n;
    double footprintLatitudeInRadians;

    double equitorialRadus = WGS84_EQUITORIAL_RADIUS;
    double polarRadius = WGS84_POLAR_RADIUS;

    n = (equitorialRadus - polarRadius) / (equitorialRadus + polarRadius);

    alpha = ((equitorialRadus + polarRadius) / 2.0) * (1 + (pow(n, 2.0) / 4) + (pow(n, 4.0) / 64));

    y = northingInMeters / alpha;

    beta = (3.0 * n / 2.0) + (-27.0 * pow(n, 3.0) / 32.0) + (269.0 * pow(n, 5.0) / 512.0);

    gamma = (21.0 * pow(n, 2.0) / 16.0) + (-55.0 * pow(n, 4.0) / 32.0);

    delta = (151.0 * pow(n, 3.0) / 96.0) + (-417.0 * pow(n, 5.0) / 128.0);

    epsilon = (1097.0 * pow(n, 4.0) / 512.0);

    footprintLatitudeInRadians = y + (beta * sin(2.0 * y)) + (gamma * sin(4.0 * y)) +
                                 (delta * sin(6.0 * y)) + (epsilon * sin(8.0 * y));

    return footprintLatitudeInRadians;
}

UTMCoordinates wgs84_to_tm(GEOCoordinates radCoordinates, double centralMeridian)
{
    UTMCoordinates tmCoordinatesi = {0};

    double N, nu2, ep2, t, t2, l;
    double l3coef, l4coef, l5coef, l6coef, l7coef, l8coef;

    double phi = radCoordinates.latitude;
    double lambda = radCoordinates.longitude;

    double equitorialRadus = WGS84_EQUITORIAL_RADIUS;
    double polarRadius = WGS84_POLAR_RADIUS;

    ep2 = (pow(equitorialRadus, 2.0) - pow(polarRadius, 2.0)) / pow(polarRadius, 2.0);

    nu2 = ep2 * pow(cos(phi), 2.0);

    N = pow(equitorialRadus, 2.0) / (polarRadius * sqrt(1 + nu2));

    t = tan(phi);
    t2 = t * t;

    l = lambda - centralMeridian;

    l3coef = 1.0 - t2 + nu2;
    l4coef = 5.0 - t2 + 9 * nu2 + 4.0 * (nu2 * nu2);
    l5coef = 5.0 - 18.0 * t2 + (t2 * t2) + 14.0 * nu2 - 58.0 * t2 * nu2;
    l6coef = 61.0 - 58.0 * t2 + (t2 * t2) + 270.0 * nu2 - 330.0 * t2 * nu2;
    l7coef = 61.0 - 479.0 * t2 + 179.0 * (t2 * t2) - (t2 * t2 * t2);
    l8coef = 1385.0 - 3111.0 * t2 + 543.0 * (t2 * t2) - (t2 * t2 * t2);

    tmCoordinates.easting = N * cos(phi) * l +
                            (N / 6.0 * pow(cos(phi), 3.0) * l3coef * pow(l, 3.0)) +
                            (N / 120.0 * pow(cos(phi), 5.0) * l5coef * pow(l, 5.0)) +
                            (N / 5040.0 * pow(cos(phi), 7.0) * l7coef * pow(l, 7.0));

    tmCoordinates.northing = arc_length_of_meridian(phi) +
                             (t / 2.0 * N * pow(cos(phi), 2.0) * pow(l, 2.0)) +
                             (t / 24.0 * N * pow(cos(phi), 4.0) * l4coef * pow(l, 4.0)) +
                             (t / 720.0 * N * pow(cos(phi), 6.0) * l6coef * pow(l, 6.0)) +
                             (t / 40320.0 * N * pow(cos(phi), 8.0) * l8coef * pow(l, 8.0));

    return tmCoordinates;
}

GEOCoordinates tm_to_wgs84(UTMCoordinates tm, double centralMeridian)
{
    GEOCoordinates coordinates;
    double phif, Nf, Nfpow, nuf2, ep2, tf, tf2, tf4, cf;
    double x1frac, x2frac, x3frac, x4frac, x5frac, x6frac, x7frac, x8frac;
    double x2poly, x3poly, x4poly, x5poly, x6poly, x7poly, x8poly;

    double x = tm.easting;
    double y = tm.northing;

    double equitorialRadus = WGS84_EQUITORIAL_RADIUS;
    double polarRadius = WGS84_POLAR_RADIUS;

    phif = footpoint_latitude(y);

    ep2 = (pow(equitorialRadus, 2.0) - pow(polarRadius, 2.0)) / pow(polarRadius, 2.0);

    cf = cos(phif);

    nuf2 = ep2 * pow(cf, 2.0);

    Nf = pow(equitorialRadus, 2.0) / (polarRadius * sqrt(1 + nuf2));
    Nfpow = Nf;

    tf = tan(phif);
    tf2 = tf * tf;
    tf4 = tf2 * tf2;

    x1frac = 1.0 / (Nfpow * cf);

    Nfpow *= Nf;
    x2frac = tf / (2.0 * Nfpow);

    Nfpow *= Nf;
    x3frac = 1.0 / (6.0 * Nfpow * cf);

    Nfpow *= Nf;
    x4frac = tf / (24.0 * Nfpow);

    Nfpow *= Nf;
    x5frac = 1.0 / (120.0 * Nfpow * cf);

    Nfpow *= Nf;
    x6frac = tf / (720.0 * Nfpow);

    Nfpow *= Nf;
    x7frac = 1.0 / (5040.0 * Nfpow * cf);

    Nfpow *= Nf;
    x8frac = tf / (40320.0 * Nfpow);

    x2poly = -1.0 - nuf2;
    x3poly = -1.0 - 2 * tf2 - nuf2;
    x4poly = 5.0 + 3.0 * tf2 + 6.0 * nuf2 - 6.0 * tf2 * nuf2 - 3.0 * (nuf2 *nuf2) - 9.0 * tf2 * (nuf2 * nuf2);
    x5poly = 5.0 + 28.0 * tf2 + 24.0 * tf4 + 6.0 * nuf2 + 8.0 * tf2 * nuf2;
    x6poly = -61.0 - 90.0 * tf2 - 45.0 * tf4 - 107.0 * nuf2 + 162.0 * tf2 * nuf2;
    x7poly = -61.0 - 662.0 * tf2 - 1320.0 * tf4 - 720.0 * (tf4 * tf2);
    x8poly = 1385.0 + 3633.0 * tf2 + 4095.0 * tf4 + 1575 * (tf4 * tf2);

    coordinates.latitude = phif + x2frac * x2poly * (x * x) + x4frac * x4poly * pow(x, 4.0) +
                           x6frac * x6poly * pow(x, 6.0) + x8frac * x8poly * pow(x, 8.0);

    coordinates.longitude = centralMeridian + x1frac * x + x3frac * x3poly * pow(x, 3.0) +
                            x5frac * x5poly * pow(x, 5.0) + x7frac * x7poly * pow(x, 7.0);

    return coordinates;
}

UTMCoordinates get_avg_utm(GEOCoordinates *measures, int size)
{
    GEOCoordinates avg;
    int i;

    avg.latitude = avg.longitude = 0;
    for (i=0; i<size; i++) {
        avg.latitude += measures[i].latitude;
        avg.longitude += measures[i].longitude;
    }

    avg.latitude /= (double)size;
    avg.longitude /= (double)size;

    return loc_geometry_convert_wgs84_to_utm(avg);
}
