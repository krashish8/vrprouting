/*PGR-GNU*****************************************************************
File: vrp_costMatrix.sql

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

-- v0.4
CREATE FUNCTION vrp_costMatrix(
  TEXT,  -- coordinates_sql (required)

  OUT start_id BIGINT,
  OUT end_id BIGINT,
  OUT cost FLOAT)
RETURNS SETOF RECORD AS
$BODY$
  SELECT *
  FROM _vrp_costMatrix(_pgr_get_statement($1));
$BODY$
LANGUAGE SQL VOLATILE STRICT;


-- COMMENTS

COMMENT ON FUNCTION vrp_costMatrix(TEXT)
IS 'vrp_costMatrix
 - EXPERIMENTAL
 - Parameters:
   - Coordinates SQL with columns:
       id, x, y
 - Documentation:
   - ${PROJECT_DOC_LINK}/vrp_costMatrix.html
';
