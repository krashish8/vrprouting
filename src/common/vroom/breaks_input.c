/*PGR-GNU*****************************************************************
File: breaks_input.c

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

#include "c_common/vroom/breaks_input.h"
#include "c_common/vroom/time_windows_input.h"

#include "c_types/column_info_t.h"

#include "c_common/get_check_data.h"

#ifdef PROFILE
#include "c_common/time_msg.h"
#include "c_common/debug_macro.h"
#endif


static
void fetch_breaks(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    Vroom_break_t *vroom_break) {
  vroom_break->id = get_Idx(tuple, tupdesc, info[0], 0);

  char *time_windows_sql = spi_getText(tuple, tupdesc, info[1]);
  get_vroom_time_windows(time_windows_sql,
    &vroom_break->time_windows, &vroom_break->time_windows_size);

  vroom_break->service = get_Duration(tuple, tupdesc, info[2], 0);
}


static
void db_get_breaks(
    char *breaks_sql,
    Vroom_break_t **breaks,
    size_t *total_breaks,

    Column_info_t *info,
    const int column_count) {
#ifdef PROFILE
  clock_t start_t = clock();
  PGR_DBG("%s", breaks_sql);
#endif

  const int tuple_limit = 1000000;

  size_t total_tuples;

  void *SPIplan;
  SPIplan = pgr_SPI_prepare(breaks_sql);
  Portal SPIportal;
  SPIportal = pgr_SPI_cursor_open(SPIplan);

  bool moredata = true;
  (*total_breaks) = total_tuples = 0;

  /* on the first tuple get the column numbers */

  while (moredata == true) {
    SPI_cursor_fetch(SPIportal, true, tuple_limit);
    if (total_tuples == 0) {
      pgr_fetch_column_info(info, column_count);
    }
    size_t ntuples = SPI_processed;
    total_tuples += ntuples;
    if (ntuples > 0) {
      if ((*breaks) == NULL)
        (*breaks) = (Vroom_break_t *)palloc0(
            total_tuples * sizeof(Vroom_break_t));
      else
        (*breaks) = (Vroom_break_t *)repalloc(
            (*breaks),
            total_tuples * sizeof(Vroom_break_t));

      if ((*breaks) == NULL) {
        elog(ERROR, "Out of memory");
      }

      size_t t;
      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;
      for (t = 0; t < ntuples; t++) {
        HeapTuple tuple = tuptable->vals[t];
        fetch_breaks(&tuple, &tupdesc, info,
            &(*breaks)[total_tuples - ntuples + t]);
      }
      SPI_freetuptable(tuptable);
    } else {
      moredata = false;
    }
  }

  SPI_cursor_close(SPIportal);

  if (total_tuples == 0) {
    (*total_breaks) = 0;
    return;
  }

  (*total_breaks) = total_tuples;
#ifdef PROFILE
  time_msg("reading breaks", start_t, clock());
#endif
}



/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
get_vroom_breaks(
    char *sql,
    Vroom_break_t **rows,
    size_t *total_rows) {
  const int column_count = 3;
  Column_info_t info[column_count];

  for (int i = 0; i < column_count; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
    info[i].eType = ANY_INTEGER;
  }

  info[0].name = "id";
  info[1].name = "time_windows_sql";
  info[2].name = "service";

  // TODO(ashish): Check for ANY_INTEGER, INTEGER, etc types in info[x].name.
  //         Better change INTEGER to ANY_INTEGER

  info[1].eType = TEXT;
  // info[2].eType = INTEGER;

  /* service is not mandatory */
  info[2].strict = false;

  db_get_breaks(sql, rows, total_rows, info, column_count);
}
