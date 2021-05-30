/*PGR-GNU*****************************************************************
File: shipments_input.c

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

#include "c_common/vroom/shipments_input.h"
#include "c_common/vroom/time_windows_input.h"

#include "c_types/column_info_t.h"

#include "c_common/debug_macro.h"
#include "c_common/get_check_data.h"
#include "c_common/time_msg.h"


// TODO(ashish): At the end, check and remove all unnecessary includes

static
void fetch_shipments(
        HeapTuple *tuple,
        TupleDesc *tupdesc,
        Column_info_t info[8],
        vrp_vroom_shipments_t *shipment) {
    // TODO(ashish): Change BigInt to Int, wherever required.
    // TODO(ashish): Check for null in optional columns

    /*
     * The pickups
     */
    shipment->p_id = pgr_SPI_getBigInt(tuple, tupdesc, info[0]);
    shipment->p_location_index = pgr_SPI_getBigInt(tuple, tupdesc, info[1]);
    shipment->p_service = column_found(info[2].colNumber) ?
        pgr_SPI_getBigInt(tuple, tupdesc, info[2]) : 0;
    shipment->p_time_windows_size = 0;
    if (column_found(info[3].colNumber)) {
        char *p_time_windows_sql = pgr_SPI_getText(tuple, tupdesc, info[3]);
        PGR_DBG("p_time_windows_sql: %s", p_time_windows_sql);
        vrp_get_vroom_time_windows(p_time_windows_sql,
            &shipment->p_time_windows, &shipment->p_time_windows_size);
    }

    /*
     * The deliveries
     */
    shipment->d_id = pgr_SPI_getBigInt(tuple, tupdesc, info[4]);
    shipment->d_location_index = pgr_SPI_getBigInt(tuple, tupdesc, info[5]);
    shipment->d_service = column_found(info[6].colNumber) ?
        pgr_SPI_getBigInt(tuple, tupdesc, info[6]) : 0;
    shipment->d_time_windows_size = 0;
    if (column_found(info[7].colNumber)) {
        char *d_time_windows_sql = pgr_SPI_getText(tuple, tupdesc, info[7]);
        PGR_DBG("d_time_windows_sql: %s", d_time_windows_sql);
        vrp_get_vroom_time_windows(d_time_windows_sql,
            &shipment->d_time_windows, &shipment->d_time_windows_size);
    }

    shipment->amount_size = 0;
    shipment->amount = column_found(info[8].colNumber) ?
        pgr_SPI_getBigIntArr(tuple, tupdesc, info[8], &shipment->amount_size)
        : NULL;

    shipment->skills_size = 0;
    shipment->skills = column_found(info[9].colNumber) ?
        pgr_SPI_getBigIntArr(tuple, tupdesc, info[9], &shipment->skills_size)
        : NULL;

    shipment->priority = column_found(info[10].colNumber) ?
        pgr_SPI_getBigInt(tuple, tupdesc, info[10]) : 0;
}


static
void
vrp_get_vroom_shipments_general(
        char *shipments_sql,
        vrp_vroom_shipments_t **shipments,
        size_t *total_shipments) {
    clock_t start_t = clock();

    const int tuple_limit = 1000000;

    PGR_DBG("vrp_get_vroom_shipments data");
    PGR_DBG("%s", shipments_sql);

    const int column_count = 11;
    Column_info_t info[column_count];

    int i;
    for (i = 0; i < column_count; ++i) {
        info[i].colNumber = -1;
        info[i].type = 0;
        info[i].strict = false;
        info[i].eType = ANY_INTEGER;
    }

    /* pickup shipments */
    info[0].name = "p_id";
    info[1].name = "p_location_index";
    info[2].name = "p_service";
    info[3].name = "p_time_windows_sql";

    /* delivery shipments */
    info[4].name = "d_id";
    info[5].name = "d_location_index";
    info[6].name = "d_service";
    info[7].name = "d_time_windows_sql";

    info[8].name = "amount";
    info[9].name = "skills";
    info[10].name = "priority";

    // TODO(ashish): Check for ANY_INTEGER, INTEGER, etc types in info[x].name.
    //               Better change INTEGER to ANY_INTEGER

    // info[2].eType = INTEGER;
    info[3].eType = TEXT;

    // info[6].eType = INTEGER;
    info[7].eType = TEXT;
    info[8].eType = ANY_INTEGER_ARRAY;

    // info[9].eType = INTEGER_ARRAY;
    info[9].eType = ANY_INTEGER_ARRAY;
    // info[10].eType = INTEGER;


    /* id and location_index of pickup and delivery are mandatory */
    info[0].strict = true;
    info[1].strict = true;
    info[4].strict = true;
    info[5].strict = true;

    size_t total_tuples;

    void *SPIplan;
    SPIplan = pgr_SPI_prepare(shipments_sql);
    Portal SPIportal;
    SPIportal = pgr_SPI_cursor_open(SPIplan);

    bool moredata = true;
    (*total_shipments) = total_tuples = 0;

    /* on the first tuple get the column numbers */

    while (moredata == true) {
        SPI_cursor_fetch(SPIportal, true, tuple_limit);
        if (total_tuples == 0) {
            pgr_fetch_column_info(info, column_coun);
        }
        size_t ntuples = SPI_processed;
        total_tuples += ntuples;
        PGR_DBG("SPI_processed %ld", ntuples);
        if (ntuples > 0) {
            if ((*shipments) == NULL)
                (*shipments) = (vrp_vroom_shipments_t *)palloc0(
                        total_tuples * sizeof(vrp_vroom_shipments_t));
            else
                (*shipments) = (vrp_vroom_shipments_t *)repalloc(
                        (*shipments),
                        total_tuples * sizeof(vrp_vroom_shipments_t));

            if ((*shipments) == NULL) {
                elog(ERROR, "Out of memory");
            }

            size_t t;
            SPITupleTable *tuptable = SPI_tuptable;
            TupleDesc tupdesc = SPI_tuptable->tupdesc;
            PGR_DBG("processing %ld", ntuples);
            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_shipments(&tuple, &tupdesc, info,
                        &(*shipments)[total_tuples - ntuples + t]);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = false;
        }
    }

    SPI_cursor_close(SPIportal);

    if (total_tuples == 0) {
        (*total_shipments) = 0;
        PGR_DBG("NO shipments");
        return;
    }

    (*total_shipments) = total_tuples;
    PGR_DBG("Finish reading %ld shipments", (*total_shipments));
    time_msg("reading edges", start_t, clock());
}

void
vrp_get_vroom_shipments(
        char *shipments_sql,
        vrp_vroom_shipments_t **shipments,
        size_t *total_shipments) {
    vrp_get_vroom_shipments_general(shipments_sql, shipments, total_shipments);
}
