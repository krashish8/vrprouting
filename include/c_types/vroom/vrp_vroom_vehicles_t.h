/*PGR-GNU*****************************************************************
File: vrp_vroom_vehicles_t.h

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
/*! @file */

#ifndef INCLUDE_C_TYPES_VROOM_VRP_VROOM_VEHICLES_T_H_
#define INCLUDE_C_TYPES_VROOM_VRP_VROOM_VEHICLES_T_H_
#pragma once

/* for int64_t */
#ifdef __cplusplus
#   include <cstdint>
#else
#   include <stdint.h>
#endif

#include "c_types/vroom/vrp_vroom_breaks_t.h"
#include "c_types/vroom/vrp_vroom_steps_t.h"

/**************************************************************************
 * vroom types
 * ***********************************************************************/
/*
 * vroom vehicles
 */
typedef struct {
    int64_t id;
    int64_t start_index;
    int64_t end_index;

    int64_t *capacity;
    size_t capacity_size;

    int *skills;
    size_t skills_size;

    int time_window_start;
    int time_window_end;

    vrp_vroom_breaks_t *breaks;
    size_t breaks_size;

    vrp_vroom_steps_t *steps;
    size_t steps_size;
} vrp_vroom_vehicles_t;


#endif  // INCLUDE_C_TYPES_VROOM_VRP_VROOM_VEHICLES_T_H_
