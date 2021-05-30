/*PGR-GNU*****************************************************************
File: shipments_input.h

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

#ifndef INCLUDE_C_COMMON_VROOM_SHIPMENTS_INPUT_H_
#define INCLUDE_C_COMMON_VROOM_SHIPMENTS_INPUT_H_
#pragma once

#include <stddef.h>
#include "c_types/vroom/vrp_vroom_shipments_t.h"

/** @brief Reads the VROOM shipments
 *
 * @param[in] shipments_sql
 * @param[out] shipments
 * @param[out] total_shipments
 */
void
vrp_get_vroom_shipments(
        char *shipments_sql,
        vrp_vroom_shipments_t **shipments,
        size_t *total_shipments);

#endif  // INCLUDE_C_COMMON_VROOM_SHIPMENTS_INPUT_H_
