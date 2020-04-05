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

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <loc_filter.h>


struct _avg_filter_t {
    double avg;
    int k;
};

struct _low_pass_filter_t {
    double lpf_x;
    double alpha;
    bool started;
};



avg_filter_t* loc_filter_avgf_create()
{
    avg_filter_t *filter = NULL;

    filter = (avg_filter_t *)malloc(sizeof(avg_filter_t));
    if (filter) {
        memset(filter, 0, sizeof(avg_filter_t));
    }

    return filter;
}

void loc_filter_avgf_destroy(avg_filter_t **filter_ref)
{
    avg_filter_t *filter = *filter_ref;

    if (!filter)
        return;

    free(filter);
    filter = NULL;
}

void loc_filter_avgf_reset(avg_filter_t **filter_ref)
{
    avg_filter_t *filter = *filter_ref;

    if (!filter)
        return;

    memset(filter, 0, sizeof(avg_filter_t));
}

double loc_filter_avgf_update(avg_filter_t **filter_ref, double x)
{
    avg_filter_t *filter = *filter_ref;

    double alpha;

    if (!filter)
        return 0;

    (filter->k)++;

    alpha = (filter->k - 1) / filter->k;
    filter->avg = (alpha * filter->avg) + ((1 - alpha) * x);

    return filter->avg;
}





low_pass_filter_t* loc_filter_lpf_create(double alpha)
{
    low_pass_filter_t *filter = NULL;

    filter = (low_pass_filter_t *)malloc(sizeof(low_pass_filter_t));
    if (filter) {
        memset(filter, 0, sizeof(low_pass_filter_t));
        filter->alpha = alpha;
    }

    return filter;
}

void loc_filter_lpf_destroy(low_pass_filter_t **filter_ref)
{
    low_pass_filter_t *filter = *filter_ref;

    if (!filter)
        return;

    free(filter);
    filter = NULL;
}

void loc_filter_lpf_reset(low_pass_filter_t **filter_ref, double alpha)
{
    low_pass_filter_t *filter = *filter_ref;

    if (!filter)
        return;

    memset(filter, 0, sizeof(low_pass_filter_t));
    filter->alpha = alpha;
}

double loc_filter_lpf_update(low_pass_filter_t **filter_ref, double x)
{
    low_pass_filter_t *filter = *filter_ref;

    if (!filter)
        return 0;

    if (!filter->started) {
        filter->lpf_x = x;
        filter->started = true;
    }

    filter->lpf_x = (filter->alpha * filter->lpf_x) + ((1 - filter->alpha) * x);

    return filter->lpf_x;
}

