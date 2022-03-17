/*PGR-GNU*****************************************************************
File: _vrp_costMatrix.sql

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
CREATE FUNCTION _vrp_costMatrix(
  coordinates_sql TEXT,

  OUT start_id BIGINT,
  OUT end_id BIGINT,
  OUT cost FLOAT)
RETURNS SETOF RECORD AS
 'MODULE_PATHNAME'
LANGUAGE C VOLATILE;

-- COMMENTS

COMMENT ON FUNCTION _vrp_costMatrix(TEXT)
IS 'pgRouting internal function';
