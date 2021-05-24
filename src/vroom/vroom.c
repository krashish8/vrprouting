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

// #include "c_common/edges_input.h"
// #include "c_common/arrays_input.h"

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
        char *vrp_json,
        char *osrm_host,
        char *osrm_port,
        bool plan,
        bool geometry,

        vrp_vroom_rt **result_tuples,
        size_t *result_count) {
    pgr_SPI_connect();

    (*result_tuples) = NULL;
    (*result_count) = 0;

    clock_t start_t = clock();
    char *log_msg = NULL;
    char *notice_msg = NULL;
    char *err_msg = NULL;
    do_vrp_vroom(
            vrp_json,
            osrm_host,
            osrm_port,
            plan,
            geometry,

            result_tuples,
            result_count,
            &log_msg,
            &notice_msg,
            &err_msg);

    time_msg("processing pgr_vroom", start_t, clock());

    if (err_msg && (*result_tuples)) {
        pfree(*result_tuples);
        (*result_tuples) = NULL;
        (*result_count) = 0;
    }

    pgr_global_report(log_msg, notice_msg, err_msg);

    if (log_msg) pfree(log_msg);
    if (notice_msg) pfree(notice_msg);
    if (err_msg) pfree(err_msg);

    pgr_SPI_finish();
}


/** @brief Helps in converting postgres variables to C variables, and returns the result.
 *
 */

PGDLLEXPORT Datum _vrp_vroom(PG_FUNCTION_ARGS) {
    vrp_vroom_rt *result_tuples = NULL;
    size_t result_count = 0;


    /***********************************************************************
     *
     *   vrp_vroom(
     *       vrp_json JSON,
     *       osrm_host TEXT DEFAULT 'car:0.0.0.0',
     *       osrm_port TEXT DEFAULT 'car:5000'
     *       plan BOOLEAN DEFAULT FALSE,
     *       geometry BOOLEAN DEFAULT FALSE
     *   );
     *
     **********************************************************************/

    process(
            text_to_cstring(PG_GETARG_TEXT_P(0)),
            text_to_cstring(PG_GETARG_TEXT_P(1)),
            text_to_cstring(PG_GETARG_TEXT_P(2)),
            PG_GETARG_BOOL(3),
            PG_GETARG_BOOL(4),
            &result_tuples,
            &result_count);

    assert(result_count == 1);

    Datum result = CStringGetTextDatum(result_tuples[0].solution);

    PG_RETURN_TEXT_P(result);
}
