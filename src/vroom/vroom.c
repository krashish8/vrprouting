/*PGR-GNU*****************************************************************
File: vroom.c

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

/** @file vroom.c
 * @brief Connecting code with postgres.
 *
 */

#include <assert.h>
#include <stdbool.h>
#include "c_common/postgres_connection.h"
// #include "utils/array.h"

#include "c_common/debug_macro.h"
#include "c_common/e_report.h"
#include "c_common/time_msg.h"

#include "c_types/vroom/vrp_vroom_rt.h"
#include "c_types/vroom/vrp_vroom_jobs_t.h"
#include "c_types/vroom/vrp_vroom_shipments_t.h"
#include "c_types/vroom/vrp_vroom_vehicles_t.h"

// #include "c_common/edges_input.h"
// #include "c_common/arrays_input.h"
#include "c_common/vroom/jobs_input.h"

#include "drivers/vroom/vroom_driver.h"

PGDLLEXPORT Datum _vrp_vroom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(_vrp_vroom);

/** @brief Static function, loads the data from postgres to C types for further processing.
 *
 * It first connects the C function to the SPI manager. Then converts
 * the postgres array to C array and loads the edges belonging to the graph
 * in C types. Then it calls the function `do_vrp_vroom` defined
 * in the `vroom_driver.h` file for further processing.
 * Finally, it frees the memory and disconnects the C function to the SPI manager.
 *
 * @param vrp_json       JSON object describing the problem instance
 * @param osrm_host      OSRM routing server host in the form of PROFILE:HOST
 * @param osrm_port      OSRM routing server port in the form of PROFILE:PORT.
 * @param plan           whether the mode is plan mode
 * @param geometry       whether to add detailed route geometry and indicators
 * @param result_tuples  the rows in the result
 * @param result_count   the count of rows in the result
 *
 * @returns void
 */
static
void
process(
        char *jobs_sql,
        char *shipments_sql,
        char *vehicles_sql,
        char *matrix_sql,
        bool plan,

        vrp_vroom_rt **result_tuples,
        size_t *result_count) {
    pgr_SPI_connect();

    (*result_tuples) = NULL;
    (*result_count) = 0;

    vrp_vroom_jobs_t *jobs = NULL;
    size_t total_jobs = 0;
    vrp_get_vroom_jobs(jobs_sql, &jobs, &total_jobs);

#if 0
    PGR_DBG("Total jobs found: %d", total_jobs);

    PGR_DBG("id: %ld", jobs->id);
    PGR_DBG("location_index: %ld", jobs->location_index);
    PGR_DBG("service: %ld", jobs->service);
    PGR_DBG("delivery: %ld", *(jobs->delivery));
    PGR_DBG("pickup: %ld", *(jobs->pickup));
    PGR_DBG("skills: %ld", *(jobs->skills));
    PGR_DBG("priority: %ld", jobs->priority);

    for(int i = 0; i < jobs->time_windows_size; i++) {
        PGR_DBG("(%d) time_windows start: %ld", i, (*(jobs->time_windows + i)).start_time);
        PGR_DBG("(%d) time_windows end: %ld", i, (*(jobs->time_windows + i)).end_time);
    }
#endif

    vrp_vroom_shipments_t *shipments = NULL;
    size_t total_shipments = 0;
    vrp_get_vroom_shipments(shipments_sql, &shipments, &total_shipments);

    PGR_DBG("Total shipments found: %d", total_shipments);

    PGR_DBG("p_id: %ld", shipments->p_id);
    PGR_DBG("p_location_index: %ld", shipments->p_location_index);
    PGR_DBG("p_service: %ld", shipments->p_service);
    for(int i = 0; i < shipments->p_time_windows_size; i++) {
        PGR_DBG("(%d) p_time_windows start: %ld", i, (*(shipments->p_time_windows + i)).start_time);
        PGR_DBG("(%d) p_time_windows end: %ld", i, (*(shipments->p_time_windows + i)).end_time);
    }
    PGR_DBG("d_id: %ld", shipments->d_id);
    PGR_DBG("d_location_index: %ld", shipments->d_location_index);
    PGR_DBG("d_service: %ld", shipments->d_service);
    for(int i = 0; i < shipments->d_time_windows_size; i++) {
        PGR_DBG("(%d) d_time_windows start: %ld", i, (*(shipments->d_time_windows + i)).start_time);
        PGR_DBG("(%d) d_time_windows end: %ld", i, (*(shipments->d_time_windows + i)).end_time);
    }
    PGR_DBG("amount: %ld", *(shipments->amount));
    PGR_DBG("skills: %ld", *(shipments->skills));
    PGR_DBG("priority: %ld", shipments->priority);



#if 0
    vrp_vroom_shipments_t *shipments = NULL;
    size_t total_shipments = 0;

    vrp_vroom_vehicles_t *vehicles = NULL;
    size_t total_vehicles = 0;

    pgr_edge_t *matrix_cells = NULL;
    size_t total_cells = 0;

    vrp_get_vroom_shipments(shipments_sql, &shipments, &total_shipments);
    vrp_get_vroom_vehicles(vehicles_sql, &vehicles, &total_vehicles);
    vrp_get_vroom_matrix(matrix_sql, &matrix_cells, &total_cells);

    clock_t start_t = clock();
    char *log_msg = NULL;
    char *notice_msg = NULL;
    char *err_msg = NULL;

    do_pgr_depthFirstSearch(
            edges, total_edges,
            rootsArr, size_rootsArr,

            directed,
            max_depth,

            result_tuples,
            result_count,
            &log_msg,
            &notice_msg,
            &err_msg);

    time_msg("processing vrp_vroom", start_t, clock());

    if (err_msg && (*result_tuples)) {
        pfree(*result_tuples);
        (*result_tuples) = NULL;
        (*result_count) = 0;
    }

    pgr_global_report(log_msg, notice_msg, err_msg);

    if (log_msg) pfree(log_msg);
    if (notice_msg) pfree(notice_msg);
    if (err_msg) pfree(err_msg);
    if (edges) pfree(edges);
#endif

    pgr_SPI_finish();
}


/** @brief Helps in converting postgres variables to C variables, and returns the result.
 *
 */

PGDLLEXPORT Datum _vrp_vroom(PG_FUNCTION_ARGS) {
    FuncCallContext     *funcctx;
    TupleDesc           tuple_desc;

    /**********************************************************************/
    vrp_vroom_rt *result_tuples = NULL;
    size_t result_count = 0;
    /**********************************************************************/

    if (SRF_IS_FIRSTCALL()) {
        MemoryContext   oldcontext;
        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

        /***********************************************************************
         *
         *   vrp_vroom(
         *       jobs_sql TEXT,
         *       shipments_sql TEXT,
         *       vehicles_sql TEXT,
         *       matrix ARRAY[ARRAY[INTEGER],
         *       plan BOOLEAN DEFAULT FALSE
         *   );
         *
         **********************************************************************/

        process(
                text_to_cstring(PG_GETARG_TEXT_P(0)),
                text_to_cstring(PG_GETARG_TEXT_P(1)),
                text_to_cstring(PG_GETARG_TEXT_P(2)),
                text_to_cstring(PG_GETARG_TEXT_P(3)),
                PG_GETARG_BOOL(4),
                &result_tuples,
                &result_count);

        /**********************************************************************/


#if PGSQL_VERSION > 95
        funcctx->max_calls = result_count;
#else
        funcctx->max_calls = (uint32_t)result_count;
#endif
        funcctx->user_fctx = result_tuples;
        if (get_call_result_type(fcinfo, NULL, &tuple_desc)
                != TYPEFUNC_COMPOSITE) {
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("function returning record called in context "
                         "that cannot accept type record")));
        }

        funcctx->tuple_desc = tuple_desc;
        MemoryContextSwitchTo(oldcontext);
    }

    funcctx = SRF_PERCALL_SETUP();
    tuple_desc = funcctx->tuple_desc;
    result_tuples = (vrp_vroom_rt*) funcctx->user_fctx;

    if (funcctx->call_cntr < funcctx->max_calls) {
        HeapTuple    tuple;
        Datum        result;
        Datum        *values;
        bool*        nulls;

        /***********************************************************************
         *
         *   OUT seq BIGINT,
         *   OUT vehicles_seq BIGINT,
         *   OUT vehicles_id BIGINT,
         *   OUT step_seq BIGINT,
         *   OUT step_type INTEGER,
         *   OUT task_id BIGINT,
         *   OUT arrival INTEGER,
         *   OUT duration INTEGER,
         *   OUT service_time INTEGER,
         *   OUT waiting_time INTEGER,
         *   OUT load BIGINT
         *
         **********************************************************************/

        size_t num  = 11;
        values = palloc(num * sizeof(Datum));
        nulls = palloc(num * sizeof(bool));


        size_t i;
        for (i = 0; i < num; ++i) {
            nulls[i] = false;
        }

        values[0] = Int64GetDatum(funcctx->call_cntr + 1);
        values[1] = Int64GetDatum(funcctx->call_cntr + 1);
        values[2] = Int64GetDatum(funcctx->call_cntr + 1);
        values[3] = Int64GetDatum(funcctx->call_cntr + 1);
        values[4] = Int64GetDatum(funcctx->call_cntr + 1);
        values[5] = Int64GetDatum(funcctx->call_cntr + 1);
        values[6] = Int64GetDatum(funcctx->call_cntr + 1);
        values[7] = Int64GetDatum(funcctx->call_cntr + 1);
        values[8] = Int64GetDatum(funcctx->call_cntr + 1);
        values[9] = Int64GetDatum(funcctx->call_cntr + 1);
        values[10] = Int64GetDatum(funcctx->call_cntr + 1);
        // values[1] = Int64GetDatum(result_tuples[funcctx->call_cntr].depth);
        // values[2] = Int64GetDatum(result_tuples[funcctx->call_cntr].from_v);
        // values[3] = Int64GetDatum(result_tuples[funcctx->call_cntr].node);
        // values[4] = Int64GetDatum(result_tuples[funcctx->call_cntr].edge);
        // values[5] = Float8GetDatum(result_tuples[funcctx->call_cntr].cost);
        // values[6] = Float8GetDatum(result_tuples[funcctx->call_cntr].agg_cost);

        /**********************************************************************/

        tuple = heap_form_tuple(tuple_desc, values, nulls);
        result = HeapTupleGetDatum(tuple);
        SRF_RETURN_NEXT(funcctx, result);
    } else {
        SRF_RETURN_DONE(funcctx);
    }
}
