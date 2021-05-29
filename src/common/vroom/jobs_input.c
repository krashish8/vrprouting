/*PGR-GNU*****************************************************************
File: jobs_input.c

Copyright (c) 2021 pgRouting developers
Mail: project@pgrouting.org

Function's developer:
Copyright (c) 2021 Ashish Kumar
Mail: ashishkr23438@gmail.com

------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 ********************************************************************PGR-GNU*/

#include "c_common/vroom/jobs_input.h"

#include "c_types/column_info_t.h"

#include "c_common/debug_macro.h"
#include "c_common/get_check_data.h"
#include "c_common/time_msg.h"

// TODO(ashish): At the end, check and remove all unnecessary includes

static
void fetch_jobs(
        HeapTuple *tuple,
        TupleDesc *tupdesc,
        Column_info_t info[8],
        vrp_vroom_jobs_t *job) {
    // TODO(ashish): Change BigInt to Int, wherever required.
    // TODO(ashish): Check for null in optional columns
    job->id = pgr_SPI_getBigInt(tuple, tupdesc, info[0]);
    job->location_index = pgr_SPI_getBigInt(tuple, tupdesc, info[1]);

    job->service = column_found(info[2].colNumber) ?
        pgr_SPI_getBigInt(tuple, tupdesc, info[2]) : 0;

    /*
     * The deliveries
     */
    job->delivery_size = 0;
    job->delivery = column_found(info[3].colNumber) ?
        pgr_SPI_getBigIntArr(tuple, tupdesc, info[3], &job->delivery_size) : NULL;

    /*
     * The pickups
     */
    job->pickup_size = 0;
    job->pickup = column_found(info[4].colNumber) ?
        pgr_SPI_getBigIntArr(tuple, tupdesc, info[4], &job->pickup_size) : NULL;

    job->skills_size = 0;
    job->skills = column_found(info[5].colNumber) ?
        pgr_SPI_getBigIntArr(tuple, tupdesc, info[5], &job->skills_size) : NULL;

    job->priority = column_found(info[6].colNumber) ?
        pgr_SPI_getBigInt(tuple, tupdesc, info[6]) : 0;
    job->time_windows_sql = column_found(info[7].colNumber) ?
        pgr_SPI_getText(tuple, tupdesc, info[7]) : "DEFAULT";
}


static
void
vrp_get_vroom_jobs_general(
        char *jobs_sql,
        vrp_vroom_jobs_t **jobs,
        size_t *total_jobs) {
    clock_t start_t = clock();

    const int tuple_limit = 1000000;

    PGR_DBG("vrp_get_vroom_jobs data");
    PGR_DBG("%s", jobs_sql);

    Column_info_t info[8];

    int i;
    for (i = 0; i < 8; ++i) {
        info[i].colNumber = -1;
        info[i].type = 0;
        info[i].strict = false;
        info[i].eType = ANY_INTEGER;
    }

    info[0].name = "id";
    info[1].name = "location_index";
    info[2].name = "service";
    info[3].name = "delivery";
    info[4].name = "pickup";
    info[5].name = "skills";
    info[6].name = "priority";
    info[7].name = "time_windows_sql";

    // TODO(ashish): Check for ANY_INTEGER, INTEGER, etc types in info[x].name. Better change INTEGER to ANY_INTEGER

    // TODO(ashish): info[2].eType = INTEGER;
    info[3].eType = ANY_INTEGER_ARRAY;
    info[4].eType = ANY_INTEGER_ARRAY;
    info[5].eType = ANY_INTEGER_ARRAY;  // TODO(ashish): info[5].eType = INTEGER_ARRAY;
    // TODO(ashish): info[6].eType = INTEGER;
    info[7].eType = TEXT;

    /* Only id and location_index are mandatory */
    info[0].strict = true;
    info[1].strict = true;

    size_t total_tuples;

    void *SPIplan;
    SPIplan = pgr_SPI_prepare(jobs_sql);
    Portal SPIportal;
    SPIportal = pgr_SPI_cursor_open(SPIplan);

    bool moredata = true;
    (*total_jobs) = total_tuples = 0;

    /* on the first tuple get the column numbers */

    while (moredata == true) {
        SPI_cursor_fetch(SPIportal, true, tuple_limit);
        if (total_tuples == 0) {
            pgr_fetch_column_info(info, 8);
        }
        size_t ntuples = SPI_processed;
        total_tuples += ntuples;
        PGR_DBG("SPI_processed %ld", ntuples);
        if (ntuples > 0) {
            if ((*jobs) == NULL)
                (*jobs) = (vrp_vroom_jobs_t *)palloc0(
                        total_tuples * sizeof(vrp_vroom_jobs_t));
            else
                (*jobs) = (vrp_vroom_jobs_t *)repalloc(
                        (*jobs),
                        total_tuples * sizeof(vrp_vroom_jobs_t));

            if ((*jobs) == NULL) {
                elog(ERROR, "Out of memory");
            }

            size_t t;
            SPITupleTable *tuptable = SPI_tuptable;
            TupleDesc tupdesc = SPI_tuptable->tupdesc;
            PGR_DBG("processing %ld", ntuples);
            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_jobs(&tuple, &tupdesc, info,
                        &(*jobs)[total_tuples - ntuples + t]);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = false;
        }
    }

    SPI_cursor_close(SPIportal);

    if (total_tuples == 0) {
        (*total_jobs) = 0;
        PGR_DBG("NO orders");
        return;
    }

    (*total_jobs) = total_tuples;
    PGR_DBG("Finish reading %ld jobs", (*total_jobs));
    time_msg("reading edges", start_t, clock());
}

void
vrp_get_vroom_jobs(
        char *jobs_sql,
        vrp_vroom_jobs_t **jobs,
        size_t *total_jobs) {
    vrp_get_vroom_jobs_general(jobs_sql, jobs, total_jobs);
}
