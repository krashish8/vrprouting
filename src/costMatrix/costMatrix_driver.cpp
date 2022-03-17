/*PGR-GNU*****************************************************************
File: costMatrix_driver.cpp

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

#include "drivers/costMatrix_driver.h"

#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <limits>
#include <cmath>

#include "c_common/pgr_alloc.hpp"
#include "cpp_common/pgr_assert.h"

// Calculate the cost using the Haversine formula
// https://en.wikipedia.org/wiki/Haversine_formula
FloatingCost haversine_distance(
    Coordinate x1, Coordinate y1,
    Coordinate x2, Coordinate y2) {
  const double EARTH_RADIUS = 6371000.0;  // Earth radius in meters
  const double DEG_TO_RAD = M_PI / 180.0;

  Coordinate lon1 = x1 * DEG_TO_RAD;
  Coordinate lon2 = x2 * DEG_TO_RAD;
  Coordinate lat1 = y1 * DEG_TO_RAD;
  Coordinate lat2 = y2 * DEG_TO_RAD;

  Coordinate dlat = lat2 - lat1;
  Coordinate dlon = lon2 - lon1;

  Coordinate a = std::pow(std::sin(dlat / 2), 2) +
      std::cos(lat1) * std::cos(lat2) * std::pow(std::sin(dlon / 2), 2);
  FloatingCost c = 2 * std::asin(std::sqrt(a));
  return EARTH_RADIUS * c;
}

std::vector <CostMatrix_rt>
get_matrix(std::vector <Coordinate_t> vertices) {
  std::vector <CostMatrix_rt> matrix;

  for (size_t i = 0; i < vertices.size(); i++) {
    for (size_t j = 0; j < vertices.size(); j++) {
      CostMatrix_rt cost_matrix;
      cost_matrix.start_id = vertices[i].id;
      cost_matrix.end_id = vertices[j].id;
      cost_matrix.cost = 0;
      if (i != j) {
        cost_matrix.cost = haversine_distance(
            vertices[i].x, vertices[i].y,
            vertices[j].x, vertices[j].y);
      } else {
        cost_matrix.cost = 0;
      }
      matrix.push_back(cost_matrix);
    }
  }

  return matrix;
}

void
do_vrp_costMatrix(
    Coordinate_t *coordinates, size_t total_coordinates,

    CostMatrix_rt **return_tuples,
    size_t *return_count,

    char ** log_msg,
    char ** notice_msg,
    char ** err_msg) {
  std::ostringstream log;
  std::ostringstream err;
  std::ostringstream notice;
  try {
    pgassert(!(*log_msg));
    pgassert(!(*notice_msg));
    pgassert(!(*err_msg));
    pgassert(!(*return_tuples));
    pgassert(!(*return_count));
    pgassert(coordinates);
    pgassert(total_coordinates);

    std::vector <Coordinate_t> vertices(coordinates, coordinates + total_coordinates);

    std::vector <CostMatrix_rt> results = get_matrix(vertices);
    auto count = results.size();

    (*return_tuples) = pgr_alloc(count, (*return_tuples));
    for (size_t i = 0; i < count; i++) {
      *((*return_tuples) + i) = results[i];
    }

    (*return_count) = count;

    pgassert(*err_msg == NULL);
    *log_msg = log.str().empty()?
      *log_msg :
      pgr_msg(log.str().c_str());
    *notice_msg = notice.str().empty()?
      *notice_msg :
      pgr_msg(notice.str().c_str());
  } catch (AssertFailedException &except) {
    (*return_tuples) = pgr_free(*return_tuples);
    (*return_count) = 0;
    err << except.what();
    *err_msg = pgr_msg(err.str().c_str());
    *log_msg = pgr_msg(log.str().c_str());
  } catch (std::exception &except) {
    (*return_tuples) = pgr_free(*return_tuples);
    (*return_count) = 0;
    err << except.what();
    *err_msg = pgr_msg(err.str().c_str());
    *log_msg = pgr_msg(log.str().c_str());
  } catch(...) {
    (*return_tuples) = pgr_free(*return_tuples);
    (*return_count) = 0;
    err << "Caught unknown exception!";
    *err_msg = pgr_msg(err.str().c_str());
    *log_msg = pgr_msg(log.str().c_str());
  }
}
