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

#include "c_common/get_check_data.h"

#ifdef PROFILE
#include "c_common/time_msg.h"
#include "c_common/debug_macro.h"
#endif



static
void fetch_time_windows(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    Vroom_time_window_t *time_window) {
  time_window->start_time = get_Duration(tuple, tupdesc, info[0], 0);
  time_window->end_time = get_Duration(tuple, tupdesc, info[1], 0);

  if (time_window->start_time > time_window->end_time) {
    ereport(ERROR,
        (errmsg("Invalid time window (%d, %d)",
            time_window->start_time, time_window->end_time),
         errhint("Time window start time %d must be "
             "less than or equal to time window end time %d",
             time_window->start_time, time_window->end_time)));
  }
}


static
void db_get_time_windows(
    char *time_windows_sql,
    Vroom_time_window_t **time_windows,
    size_t *total_time_windows,

    Column_info_t *info,
    const int column_count) {
#ifdef PROFILE
  clock_t start_t = clock();
  PGR_DBG("%s", time_windows_sql);
#endif

  const int tuple_limit = 1000000;

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
      pgr_fetch_column_info(info, column_count);
    }
    size_t ntuples = SPI_processed;
    total_tuples += ntuples;
    if (ntuples > 0) {
      if ((*time_windows) == NULL)
        (*time_windows) = (Vroom_time_window_t *)palloc0(
            total_tuples * sizeof(Vroom_time_window_t));
      else
        (*time_windows) = (Vroom_time_window_t *)repalloc(
            (*time_windows),
            total_tuples * sizeof(Vroom_time_window_t));

      if ((*time_windows) == NULL) {
        elog(ERROR, "Out of memory");
      }

      size_t t;
      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;
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
    return;
  }

  (*total_time_windows) = total_tuples;
#ifdef PROFILE
  time_msg("reading time windows", start_t, clock());
#endif
}


/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
get_vroom_time_windows(
    char *sql,
    Vroom_time_window_t **rows,
    size_t *total_rows) {
  const int column_count = 2;
  Column_info_t info[column_count];

  for (int i = 0; i < column_count; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = ANY_INTEGER;
  }

  info[0].name = "start_time";
  info[1].name = "end_time";

  db_get_time_windows(sql, rows, total_rows, info, column_count);
}
