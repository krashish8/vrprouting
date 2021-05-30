/*PGR-GNU*****************************************************************
File: matrix_input.c

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

#include "c_common/vroom/matrix_input.h"

#include "c_types/column_info_t.h"

#include "c_common/debug_macro.h"
#include "c_common/get_check_data.h"
#include "c_common/time_msg.h"



static
void fetch_matrix_cell(
        HeapTuple *tuple,
        TupleDesc *tupdesc,
        Column_info_t *info,
        vrp_vroom_matrix_cell_t *distance) {
    distance->start_index = pgr_SPI_getBigInt(tuple, tupdesc, info[0]);
    distance->end_index = pgr_SPI_getBigInt(tuple, tupdesc, info[1]);
    distance->agg_cost = pgr_SPI_getBigInt(tuple, tupdesc, info[2]);
}


static
void
vrp_get_vroom_matrix_cell_general(
        char *matrix_sql,
        vrp_vroom_matrix_cell_t **distances,
        size_t *total_distances) {
    clock_t start_t = clock();

    const int tuple_limit = 1000000;

    PGR_DBG("vrp_get_vroom_matrix data");
    PGR_DBG("%s", matrix_sql);

    const int column_count = 3;
    Column_info_t info[column_count];

    int i;
    for (i = 0; i < column_count; ++i) {
        info[i].colNumber = -1;
        info[i].type = 0;
        info[i].strict = true;
        info[i].eType = ANY_INTEGER;
    }

    info[0].name = "start_index";
    info[1].name = "end_index";
    info[2].name = "agg_cost";

    // info[3].eType = INTEGER;

    size_t total_tuples;

    void *SPIplan;
    SPIplan = pgr_SPI_prepare(matrix_sql);
    Portal SPIportal;
    SPIportal = pgr_SPI_cursor_open(SPIplan);

    bool moredata = true;
    (*total_distances) = total_tuples = 0;

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
            if ((*distances) == NULL)
                (*distances) = (vrp_vroom_matrix_cell_t *)palloc0(
                        total_tuples * sizeof(vrp_vroom_matrix_cell_t));
            else
                (*distances) = (vrp_vroom_matrix_cell_t *)repalloc(
                        (*distances),
                        total_tuples * sizeof(vrp_vroom_matrix_cell_t));

            if ((*distances) == NULL) {
                elog(ERROR, "Out of memory");
            }

            size_t t;
            SPITupleTable *tuptable = SPI_tuptable;
            TupleDesc tupdesc = SPI_tuptable->tupdesc;
            PGR_DBG("processing %ld", ntuples);
            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_matrix_cell(&tuple, &tupdesc, info,
                        &(*distances)[total_tuples - ntuples + t]);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = false;
        }
    }

    SPI_cursor_close(SPIportal);

    if (total_tuples == 0) {
        (*total_distances) = 0;
        PGR_DBG("NO distance");
        return;
    }

    (*total_distances) = total_tuples;
    PGR_DBG("Finish reading %ld distances", (*total_distances));
    time_msg("reading distances", start_t, clock());
}

void
vrp_get_vroom_matrix_cell(
        char *matrix_sql,
        vrp_vroom_matrix_cell_t **distances,
        size_t *total_distances) {
    vrp_get_vroom_matrix_cell_general(matrix_sql, distances, total_distances);
}
