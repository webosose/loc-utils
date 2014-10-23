/*
 * Copyright (c) 2012-2014 LG Electronics Inc. All Rights Reserved.
 */
#include <stdlib.h>
#include <math.h>
#include <loc_geometry.h>

#define MATH_PI 3.1415926535897932384626433832795


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

