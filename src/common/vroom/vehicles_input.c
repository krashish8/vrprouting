/*PGR-GNU*****************************************************************
File: vehicles_input.c

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

#include "c_common/vroom/vehicles_input.h"
#include "c_common/vroom/breaks_input.h"
#include "c_common/vroom/steps_input.h"

#include "c_types/column_info_t.h"

#include "c_common/debug_macro.h"
#include "c_common/get_check_data.h"
#include "c_common/time_msg.h"


// TODO(ashish): At the end, check and remove all unnecessary includes

static
void fetch_vehicles(
        HeapTuple *tuple,
        TupleDesc *tupdesc,
        Column_info_t *info,
        vrp_vroom_vehicles_t *vehicle) {
    // TODO(ashish): Change BigInt to Int, wherever required.
    // TODO(ashish): Check for null in optional columns

    vehicle->id = pgr_SPI_getBigInt(tuple, tupdesc, info[0]);
    vehicle->start_index = pgr_SPI_getBigInt(tuple, tupdesc, info[1]);
    vehicle->end_index = pgr_SPI_getBigInt(tuple, tupdesc, info[2]);

    vehicle->capacity_size = 0;
    vehicle->capacity = column_found(info[3].colNumber) ?
        pgr_SPI_getBigIntArr(tuple, tupdesc, info[3], &vehicle->capacity_size)
        : NULL;

    vehicle->skills_size = 0;
    vehicle->skills = column_found(info[4].colNumber) ?
        pgr_SPI_getBigIntArr(tuple, tupdesc, info[4], &vehicle->skills_size)
        : NULL;

    // TODO(ashish): Find a better default value for time window start & end.
    vehicle->time_window_start = column_found(info[5].colNumber) ?
        pgr_SPI_getBigInt(tuple, tupdesc, info[5]) : -1;
    vehicle->time_window_end = column_found(info[6].colNumber) ?
        pgr_SPI_getBigInt(tuple, tupdesc, info[6]) : -1;

    vehicle->breaks_size = 0;
    if (column_found(info[7].colNumber)) {
        char *breaks_sql = pgr_SPI_getText(tuple, tupdesc, info[7]);
        PGR_DBG("breaks_sql: %s", breaks_sql);
        vrp_get_vroom_breaks(breaks_sql,
            &vehicle->breaks, &vehicle->breaks_size);
    }

    vehicle->steps_size = 0;
    if (column_found(info[8].colNumber)) {
        char *steps_sql = pgr_SPI_getText(tuple, tupdesc, info[8]);
        PGR_DBG("steps_sql: %s", steps_sql);
        vrp_get_vroom_steps(steps_sql,
            &vehicle->steps, &vehicle->steps_size);
    }
}


static
void
vrp_get_vroom_vehicles_general(
        char *vehicles_sql,
        vrp_vroom_vehicles_t **vehicles,
        size_t *total_vehicles) {
    clock_t start_t = clock();

    const int tuple_limit = 1000000;

    PGR_DBG("vrp_get_vroom_vehicles data");
    PGR_DBG("%s", vehicles_sql);

    const int column_count = 9;
    Column_info_t info[column_count];

    int i;
    for (i = 0; i < column_count; ++i) {
        info[i].colNumber = -1;
        info[i].type = 0;
        info[i].strict = false;
        info[i].eType = ANY_INTEGER;
    }

    info[0].name = "id";
    info[1].name = "start_index";
    info[2].name = "end_index";
    info[3].name = "capacity";
    info[4].name = "skills";
    info[5].name = "time_window_start";
    info[6].name = "time_window_end";
    info[7].name = "breaks_sql";
    info[8].name = "steps_sql";

    // TODO(ashish): Check for ANY_INTEGER, INTEGER, etc types in info[x].name.
    //               Better change INTEGER to ANY_INTEGER

    info[3].eType = ANY_INTEGER_ARRAY;

    // info[4].eType = INTEGER_ARRAY;
    info[4].eType = ANY_INTEGER_ARRAY;

    // info[5].eType = INTEGER;
    // info[6].eType = INTEGER;

    info[7].eType = TEXT;
    info[8].eType = TEXT;

    /* id, stand and end index are mandatory */
    info[0].strict = true;
    info[1].strict = true;
    info[2].strict = true;

    size_t total_tuples;

    void *SPIplan;
    SPIplan = pgr_SPI_prepare(vehicles_sql);
    Portal SPIportal;
    SPIportal = pgr_SPI_cursor_open(SPIplan);

    bool moredata = true;
    (*total_vehicles) = total_tuples = 0;

    /* on the first tuple get the column numbers */

    while (moredata == true) {
        SPI_cursor_fetch(SPIportal, true, tuple_limit);
        if (total_tuples == 0) {
            pgr_fetch_column_info(info, column_count);
        }
        size_t ntuples = SPI_processed;
        total_tuples += ntuples;
        PGR_DBG("SPI_processed %ld", ntuples);
        if (ntuples > 0) {
            if ((*vehicles) == NULL)
                (*vehicles) = (vrp_vroom_vehicles_t *)palloc0(
                        total_tuples * sizeof(vrp_vroom_vehicles_t));
            else
                (*vehicles) = (vrp_vroom_vehicles_t *)repalloc(
                        (*vehicles),
                        total_tuples * sizeof(vrp_vroom_vehicles_t));

            if ((*vehicles) == NULL) {
                elog(ERROR, "Out of memory");
            }

            size_t t;
            SPITupleTable *tuptable = SPI_tuptable;
            TupleDesc tupdesc = SPI_tuptable->tupdesc;
            PGR_DBG("processing %ld", ntuples);
            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_vehicles(&tuple, &tupdesc, info,
                        &(*vehicles)[total_tuples - ntuples + t]);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = false;
        }
    }

    SPI_cursor_close(SPIportal);

    if (total_tuples == 0) {
        (*total_vehicles) = 0;
        PGR_DBG("NO vehicles");
        return;
    }

    (*total_vehicles) = total_tuples;
    PGR_DBG("Finish reading %ld vehicles", (*total_vehicles));
    time_msg("reading vehicles", start_t, clock());
}

void
vrp_get_vroom_vehicles(
        char *vehicles_sql,
        vrp_vroom_vehicles_t **vehicles,
        size_t *total_vehicles) {
    vrp_get_vroom_vehicles_general(vehicles_sql, vehicles, total_vehicles);
}
