/*PGR-GNU*****************************************************************
File: coordinates_input.c

Copyright (c) 2022 pgRouting developers
Mail: project@pgrouting.org

Function's developer:
Copyright (c) 2022 Ashish Kumar
Mail: ashishkr23438 AT gmail DOT com

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

#include "c_common/coordinates_input.h"

static
void fetch_coordinates(
    HeapTuple *tuple,
    TupleDesc *tupdesc,
    Column_info_t *info,
    Coordinate_t *coord) {
  coord->id = get_MatrixIndex(tuple, tupdesc, info[0], 0);
  coord->x = spi_getCoordinate(tuple, tupdesc, info[1], 0);
  coord->y = spi_getCoordinate(tuple, tupdesc, info[2], 0);
}


static
void db_get_coordinates(
    char *sql,
    Coordinate_t **coordinates,
    size_t *total_coordinates,

    Column_info_t *info,
    const int column_count) {
  const int tuple_limit = 1000000;

  size_t total_tuples;

  void *SPIplan;
  SPIplan = pgr_SPI_prepare(sql);
  Portal SPIportal;
  SPIportal = pgr_SPI_cursor_open(SPIplan);

  bool moredata = true;
  (*total_coordinates) = total_tuples = 0;

  /* on the first tuple get the column numbers */

  while (moredata == true) {
    SPI_cursor_fetch(SPIportal, true, tuple_limit);
    if (total_tuples == 0) {
      pgr_fetch_column_info(info, column_count);
    }
    size_t ntuples = SPI_processed;
    total_tuples += ntuples;
    if (ntuples > 0) {
      if ((*coordinates) == NULL)
        (*coordinates) = (Coordinate_t *)palloc0(
            total_tuples * sizeof(Coordinate_t));
      else
        (*coordinates) = (Coordinate_t *)repalloc(
            (*coordinates),
            total_tuples * sizeof(Coordinate_t));

      if ((*coordinates) == NULL) {
        elog(ERROR, "Out of memory");
      }

      size_t t;
      SPITupleTable *tuptable = SPI_tuptable;
      TupleDesc tupdesc = SPI_tuptable->tupdesc;
      for (t = 0; t < ntuples; t++) {
        HeapTuple tuple = tuptable->vals[t];
        fetch_coordinates(&tuple, &tupdesc, info,
            &(*coordinates)[total_tuples - ntuples + t]);
      }
      SPI_freetuptable(tuptable);
    } else {
      moredata = false;
    }
  }

  SPI_cursor_close(SPIportal);

  if (total_tuples == 0) {
    (*total_coordinates) = 0;
    return;
  }

  (*total_coordinates) = total_tuples;
}


/**
 * @param[in] sql SQL query to execute
 * @param[out] rows C Container that holds the data
 * @param[out] total_rows Total rows recieved
 */
void
get_coordinates(
    char *sql,
    Coordinate_t **coordinates,
    size_t *total_coordinates) {
  int kColumnCount = 3;
  Column_info_t info[kColumnCount];

  for (int i = 0; i < kColumnCount; ++i) {
    info[i].colNumber = -1;
    info[i].type = 0;
    info[i].strict = true;
  }

  info[0].name = "id";
  info[1].name = "x";
  info[2].name = "y";

  info[0].eType = ANY_INTEGER;    // id
  info[1].eType = ANY_NUMERICAL;  // x
  info[2].eType = ANY_NUMERICAL;  // y

  db_get_coordinates(sql, coordinates, total_coordinates, info, kColumnCount);
}
