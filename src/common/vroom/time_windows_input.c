/*PGR-GNU*****************************************************************
File: time_windows_input.c

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

#include "c_common/vroom/time_windows_input.h"

#include "c_types/column_info_t.h"

#include "c_common/debug_macro.h"
#include "c_common/get_check_data.h"
#include "c_common/time_msg.h"



static
void fetch_time_windows(
        HeapTuple *tuple,
        TupleDesc *tupdesc,
        Column_info_t info[2],
        vrp_vroom_time_windows_t *time_window) {
    time_window->start_time = pgr_SPI_getBigInt(tuple, tupdesc, info[0]);
    time_window->end_time = pgr_SPI_getBigInt(tuple, tupdesc, info[1]);

    if (time_window->start_time > time_window->end_time) {
        elog(ERROR, "Time window start time %d must be less than the "
                     "Time window end time %d",
                     time_window->start_time, time_window->end_time);
    }
}


static
void
vrp_get_vroom_time_windows_general(
        char *time_windows_sql,
        vrp_vroom_time_windows_t **time_windows,
        size_t *total_time_windows) {
    clock_t start_t = clock();

    const int tuple_limit = 1000000;

    PGR_DBG("vrp_get_vroom_time_windows data");
    PGR_DBG("%s", time_windows_sql);

    Column_info_t info[2];

    int i;
    for (i = 0; i < 2; ++i) {
        info[i].colNumber = -1;
        info[i].type = 0;
        info[i].strict = true;
        info[i].eType = ANY_INTEGER;
    }

    info[0].name = "start_time";
    info[1].name = "end_time";

    size_t total_tuples;

    void *SPIplan;
    SPIplan = pgr_SPI_prepare(time_windows_sql);
    Portal SPIportal;
    SPIportal = pgr_SPI_cursor_open(SPIplan);

    bool moredata = true;
    (*total_time_windows) = total_tuples = 0;

    /* on the first tuple get the column numbers */

    while (moredata == true) {
        SPI_cursor_fetch(SPIportal, true, tuple_limit);
        if (total_tuples == 0) {
            pgr_fetch_column_info(info, 2);
        }
        size_t ntuples = SPI_processed;
        total_tuples += ntuples;
        PGR_DBG("SPI_processed %ld", ntuples);
        if (ntuples > 0) {
            if ((*time_windows) == NULL)
                (*time_windows) = (vrp_vroom_time_windows_t *)palloc0(
                        total_tuples * sizeof(vrp_vroom_time_windows_t));
            else
                (*time_windows) = (vrp_vroom_time_windows_t *)repalloc(
                        (*time_windows),
                        total_tuples * sizeof(vrp_vroom_time_windows_t));

            if ((*time_windows) == NULL) {
                elog(ERROR, "Out of memory");
            }

            size_t t;
            SPITupleTable *tuptable = SPI_tuptable;
            TupleDesc tupdesc = SPI_tuptable->tupdesc;
            PGR_DBG("processing %ld", ntuples);
            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_time_windows(&tuple, &tupdesc, info,
                        &(*time_windows)[total_tuples - ntuples + t]);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = false;
        }
    }

    SPI_cursor_close(SPIportal);

    if (total_tuples == 0) {
        (*total_time_windows) = 0;
        PGR_DBG("NO orders");
        return;
    }

    (*total_time_windows) = total_tuples;
    PGR_DBG("Finish reading %ld time_windows", (*total_time_windows));
    time_msg("reading edges", start_t, clock());
}

void
vrp_get_vroom_time_windows(
        char *time_windows_sql,
        vrp_vroom_time_windows_t **time_windows,
        size_t *total_time_windows) {
    vrp_get_vroom_time_windows_general(time_windows_sql, time_windows, total_time_windows);
}
