DROP TABLE IF EXISTS public.vroom_time_windows;
DROP TABLE IF EXISTS public.vroom_jobs;
DROP TABLE IF EXISTS public.vroom_shipments;
DROP TABLE IF EXISTS public.vroom_breaks;
DROP TABLE IF EXISTS public.vroom_vehicles;
DROP TABLE IF EXISTS public.vroom_matrix;

-- VROOM TWS TABLE CREATE start
CREATE TABLE public.vroom_time_windows (
  id BIGINT,
  tw_open INTEGER,
  tw_close INTEGER
);
-- VROOM TWS TABLE CREATE end

-- VROOM TWS TABLE ADD DATA start
INSERT INTO public.vroom_time_windows (
  id, tw_open, tw_close)
  VALUES
(1, 145000, 175000),
(2, 50000, 80000),
(3, 109000, 139000),
(4, 141000, 171000),
(5, 41000, 71000),
(6, 65000, 146000),
(7, 997000, 1068000),
(8, 15000, 67000),
(9, 170000, 225000),
(10, 621000, 702000),
(11, 825000, 870000),
(12, 255000, 324000),
(13, 357000, 410000),
(14, 534000, 605000),
(15, 727000, 782000),
(16, 10000, 12000),
(17, 10000, 11000),
(18, 0, 0),
(19, 10000, 10000);
-- VROOM TWS TABLE ADD DATA end


-- VROOM JOBS TABLE CREATE start
CREATE TABLE public.vroom_jobs (
  id BIGINT,
  location_index BIGINT,
  service INTEGER,
  delivery BIGINT[],
  pickup BIGINT[],
  skills INTEGER[],
  priority INTEGER,
  time_windows_sql TEXT
);
-- VROOM JOBS TABLE CREATE end

-- VROOM JOBS TABLE ADD DATA start
INSERT INTO public.vroom_jobs (
  id, location_index, service, delivery, pickup, skills, priority, time_windows_sql)
  VALUES
(1, 1, 10000, ARRAY[20], ARRAY[20], ARRAY[0], 0, $$SELECT * FROM vroom_time_windows WHERE id = 1$$),
(2, 2, 10000, ARRAY[30], ARRAY[30], ARRAY[0], 0, $$SELECT * FROM vroom_time_windows WHERE id = 2$$),
(3, 3, 10000, ARRAY[10], ARRAY[10], ARRAY[0], 0, $$SELECT * FROM vroom_time_windows WHERE id = 3$$),
(4, 3, 10000, ARRAY[40], ARRAY[40], ARRAY[0], 0, $$SELECT * FROM vroom_time_windows WHERE id = 4$$),
(5, 4, 10000, ARRAY[20], ARRAY[20], ARRAY[0], 0, $$SELECT * FROM vroom_time_windows WHERE id = 5$$);
-- VROOM JOBS TABLE ADD DATA end


-- VROOM SHIPMENTS TABLE CREATE start
CREATE TABLE public.vroom_shipments (
  p_id BIGINT,
  p_location_index BIGINT,
  p_service INTEGER,
  p_time_windows_sql TEXT,
  d_id BIGINT,
  d_location_index BIGINT,
  d_service INTEGER,
  d_time_windows_sql TEXT,
  amount BIGINT[],
  skills INTEGER[],
  priority INTEGER
);
-- VROOM SHIPMENTS TABLE CREATE end

-- VROOM SHIPMENTS TABLE ADD DATA start
INSERT INTO public.vroom_shipments (
  p_id, p_location_index, p_service, p_time_windows_sql,
  d_id, d_location_index, d_service, d_time_windows_sql,
  amount, skills, priority)
  VALUES
(6, 3, 90000, $$SELECT * FROM vroom_time_windows WHERE id = 6$$,
  7, 5, 90000, $$SELECT * FROM vroom_time_windows WHERE id = 7$$,
  ARRAY[10], ARRAY[0], 0),
(8, 5, 90000, $$SELECT * FROM vroom_time_windows WHERE id = 8$$,
  9, 6, 90000, $$SELECT * FROM vroom_time_windows WHERE id = 9$$,
  ARRAY[10], ARRAY[0], 0),
(10, 1, 90000, $$SELECT * FROM vroom_time_windows WHERE id = 10$$,
  11, 2, 90000, $$SELECT * FROM vroom_time_windows WHERE id = 11$$,
  ARRAY[20], ARRAY[0], 0),
(12, 1, 90000, $$SELECT * FROM vroom_time_windows WHERE id = 12$$,
  13, 4, 90000, $$SELECT * FROM vroom_time_windows WHERE id = 13$$,
  ARRAY[20], ARRAY[0], 0),
(14, 2, 90000, $$SELECT * FROM vroom_time_windows WHERE id = 14$$,
  15, 2, 90000, $$SELECT * FROM vroom_time_windows WHERE id = 15$$,
  ARRAY[10], ARRAY[0], 0);
-- VROOM SHIPMENTS TABLE ADD DATA end


-- VROOM BREAKS TABLE CREATE start
CREATE TABLE public.vroom_breaks (
  id BIGINT,
  time_windows_sql TEXT,
  service INTEGER
);
-- VROOM BREAKS TABLE CREATE end

-- VROOM BREAKS TABLE ADD DATA start
INSERT INTO public.vroom_breaks (
  id, time_windows_sql, service)
  VALUES
(16, $$SELECT * FROM vroom_time_windows WHERE id = 16$$, 0),
(17, $$SELECT * FROM vroom_time_windows WHERE id = 17$$, 10),
(18, $$SELECT * FROM vroom_time_windows WHERE id = 18$$, 0),
(19, $$SELECT * FROM vroom_time_windows WHERE id = 19$$, 0);
-- VROOM BREAKS TABLE ADD DATA end


-- VROOM VEHICLES TABLE CREATE start
CREATE TABLE public.vroom_vehicles (
  id BIGINT,
  start_index BIGINT,
  end_index BIGINT,
  capacity BIGINT[],
  skills INTEGER[],
  tw_open INTEGER,
  tw_close INTEGER,
  breaks_sql TEXT,
  steps_sql TEXT
);
-- VROOM VEHICLES TABLE CREATE end

-- VROOM VEHICLES TABLE ADD DATA start
INSERT INTO public.vroom_vehicles (
  id, start_index, end_index, capacity, skills,
  tw_open, tw_close, breaks_sql, steps_sql)
  VALUES
(1, 1, 1, ARRAY[200], ARRAY[0], 0, 1236000, $$SELECT * FROM vroom_breaks WHERE id = 16$$, NULL),
(2, 1, 3, ARRAY[200], ARRAY[0], 3600, 1236000, $$SELECT * FROM vroom_breaks WHERE id = 17$$, NULL),
(3, 1, 1, ARRAY[200], ARRAY[0], 0, 1236000, $$SELECT * FROM vroom_breaks WHERE id = 18$$, NULL),
(4, 3, 3, ARRAY[200], ARRAY[0], 0, 1236000, $$SELECT * FROM vroom_breaks WHERE id = 19$$, NULL);
-- VROOM VEHICLES TABLE ADD DATA end

-- VROOM MATRIX TABLE CREATE start
CREATE TABLE public.vroom_matrix (
  start_vid BIGINT,
  end_vid BIGINT,
  agg_cost INTEGER
);
-- VROOM MATRIX TABLE CREATE end

-- VROOM MATRIX TABLE ADD DATA start
INSERT INTO public.vroom_matrix (
  start_vid, end_vid, agg_cost)
  VALUES
(1, 1, 0), (1, 2, 2000), (1, 3, 3606), (1, 4, 3000), (1, 5, 4243), (1, 6, 5099),
(2, 1, 2000), (2, 2, 0), (2, 3, 5000), (2, 4, 3606), (2, 5, 5831), (2, 6, 5099),
(3, 1, 3606), (3, 2, 5000), (3, 3, 0), (3, 4, 2000), (3, 5, 1000), (3, 6, 3606),
(4, 1, 3000), (4, 2, 3606), (4, 3, 2000), (4, 4, 0), (4, 5, 3000), (4, 6, 2236),
(5, 1, 4243), (5, 2, 5831), (5, 3, 1000), (5, 4, 3000), (5, 5, 0), (5, 6, 4472),
(6, 1, 5099), (6, 2, 5099), (6, 3, 3606), (6, 4, 2236), (6, 5, 4472), (6, 6, 0);
-- VROOM MATRIX TABLE ADD DATA end
