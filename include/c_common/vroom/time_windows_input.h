/*PGR-GNU*****************************************************************
File: time_windows_input.h

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

#ifndef INCLUDE_C_COMMON_VROOM_TIME_WINDOWS_INPUT_H_
#define INCLUDE_C_COMMON_VROOM_TIME_WINDOWS_INPUT_H_
#pragma once

#include <stddef.h>
#include "c_types/vroom/vrp_vroom_time_windows_t.h"

/** @brief Reads the VROOM time windows
 *
 * @param[in] time_windows_sql
 * @param[out] time_windows
 * @param[out] total_time_windows
 */
void
vrp_get_vroom_time_windows(
        char *time_windows_sql,
        vrp_vroom_time_windows_t **time_windows,
        size_t *total_time_windows);

#endif  // INCLUDE_C_COMMON_VROOM_TIME_WINDOWS_INPUT_H_
