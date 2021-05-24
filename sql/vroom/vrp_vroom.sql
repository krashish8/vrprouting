/*PGR-GNU*****************************************************************
File: _vrp_vroom.sql

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

-- v0.0
CREATE FUNCTION vrp_vroom(
    TEXT, -- vrp_json (required)

    osrm_host TEXT DEFAULT 'car:0.0.0.0',
    osrm_port TEXT DEFAULT 'car:5000',
    plan BOOLEAN DEFAULT FALSE,
    geometry BOOLEAN DEFAULT FALSE,

    OUT seq INTEGER,
    OUT solution TEXT)
RETURNS SETOF RECORD AS
$BODY$
    SELECT *
    FROM _vrp_vroom(_pgr_get_statement($1), _pgr_get_statement($2), _pgr_get_statement($3), $4, $5);
$BODY$
LANGUAGE SQL VOLATILE STRICT;

-- COMMENTS

COMMENT ON FUNCTION vrp_vroom(TEXT, TEXT, TEXT, BOOLEAN, BOOLEAN)
IS 'vrp_vroom
 - EXPERIMENTAL
 - Parameters:
   - vrp_json: JSON object with following keys:
     - jobs: Array of job objects describing the places to visit.
     - shipments: Array of shipment objects describing pickup and delivery tasks.
     - vehicles: Array of vehicle objects describing the available vehicles.
 - Optional Parameters:
   - osrm_host: default := car:0.0.0.0
   - osrm_port: default := car:5000
   - plan: default := FALSE
   - geometry: default := FALSE
- Documentation:
   - ${PROJECT_DOC_LINK}/vrp_vroom.html
';
