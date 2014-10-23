/*
 * Copyright (c) 2012-2013 LG Electronics Inc. All Rights Reserved.
 */

#ifndef _LOC_FILTER_H_
#define _LOC_FILTER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _avg_filter_t        avg_filter_t;
typedef struct _low_pass_filter_t   low_pass_filter_t;



/*
 * Average Filter
 */
avg_filter_t* loc_filter_avgf_create();

void loc_filter_avgf_destroy(avg_filter_t **filter_ref);

void loc_filter_avgf_reset(avg_filter_t **filter_ref);

double loc_filter_avgf_update(avg_filter_t **filter_ref, double x);



/*
 * Low Pass Filter
 */
low_pass_filter_t* loc_filter_lpf_create(double alpha);

void loc_filter_lpf_destroy(low_pass_filter_t **filter_ref);

void loc_filter_lpf_reset(low_pass_filter_t **filter_ref, double alpha);

double loc_filter_lpf_update(low_pass_filter_t **filter_ref, double x);



#ifdef __cplusplus
}
#endif

#endif
