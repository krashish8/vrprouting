/*PGR-GNU*****************************************************************
File: steps_input.c

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

#include "c_common/vroom/steps_input.h"

#include "c_types/column_info_t.h"

#include "c_common/debug_macro.h"
#include "c_common/get_check_data.h"
#include "c_common/time_msg.h"



static
void fetch_steps(
        HeapTuple *tuple,
        TupleDesc *tupdesc,
        Column_info_t *info,
        vrp_vroom_steps_t *step) {
    // TODO(ashish): Add constraint check for id = -1 and type, etc.
    step->id = pgr_SPI_getBigInt(tuple, tupdesc, info[0]);
    step->type = pgr_SPI_getBigInt(tuple, tupdesc, info[1]);

    step->service_at = column_found(info[2].colNumber) ?
        pgr_SPI_getBigInt(tuple, tupdesc, info[2]) : -1;
    step->service_after = column_found(info[3].colNumber) ?
        pgr_SPI_getBigInt(tuple, tupdesc, info[3]) : -1;
    step->service_before = column_found(info[4].colNumber) ?
        pgr_SPI_getBigInt(tuple, tupdesc, info[4]) : -1;
}


static
void
vrp_get_vroom_steps_general(
        char *steps_sql,
        vrp_vroom_steps_t **steps,
        size_t *total_steps) {
    clock_t start_t = clock();

    const int tuple_limit = 1000000;

    PGR_DBG("vrp_get_vroom_steps data");
    PGR_DBG("%s", steps_sql);

    const int column_count = 5;
    Column_info_t info[column_count];

    int i;
    for (i = 0; i < column_count; ++i) {
        info[i].colNumber = -1;
        info[i].type = 0;
        info[i].strict = false;
        info[i].eType = ANY_INTEGER;
    }

    info[0].name = "id";
    info[1].name = "type"; // TODO(ashish): Add constraint checks

    /* constraints on service time */
    info[2].name = "service_at";
    info[3].name = "service_after";
    info[4].name = "service_before";

    // TODO(ashish): Check for ANY_INTEGER, INTEGER, etc types in info[x].name.
    //               Better change INTEGER to ANY_INTEGER

    // info[1].eType = INTEGER;
    // info[2].eType = INTEGER;
    // info[3].eType = INTEGER;
    // info[4].eType = INTEGER;

    /* id and type are mandatory */
    info[0].strict = true;
    info[1].strict = true;

    size_t total_tuples;

    void *SPIplan;
    SPIplan = pgr_SPI_prepare(steps_sql);
    Portal SPIportal;
    SPIportal = pgr_SPI_cursor_open(SPIplan);

    bool moredata = true;
    (*total_steps) = total_tuples = 0;

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
            if ((*steps) == NULL)
                (*steps) = (vrp_vroom_steps_t *)palloc0(
                        total_tuples * sizeof(vrp_vroom_steps_t));
            else
                (*steps) = (vrp_vroom_steps_t *)repalloc(
                        (*steps),
                        total_tuples * sizeof(vrp_vroom_steps_t));

            if ((*steps) == NULL) {
                elog(ERROR, "Out of memory");
            }

            size_t t;
            SPITupleTable *tuptable = SPI_tuptable;
            TupleDesc tupdesc = SPI_tuptable->tupdesc;
            PGR_DBG("processing %ld", ntuples);
            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_steps(&tuple, &tupdesc, info,
                        &(*steps)[total_tuples - ntuples + t]);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = false;
        }
    }

    SPI_cursor_close(SPIportal);

    if (total_tuples == 0) {
        (*total_steps) = 0;
        PGR_DBG("NO steps");
        return;
    }

    (*total_steps) = total_tuples;
    PGR_DBG("Finish reading %ld steps", (*total_steps));
    time_msg("reading steps", start_t, clock());
}

void
vrp_get_vroom_steps(
        char *steps_sql,
        vrp_vroom_steps_t **steps,
        size_t *total_steps) {
    vrp_get_vroom_steps_general(steps_sql, steps, total_steps);
}
