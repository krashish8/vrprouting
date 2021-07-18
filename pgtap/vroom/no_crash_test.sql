BEGIN;

-- SELECT CASE WHEN min_version('0.2.0') THEN plan (59) ELSE plan(1) END;
select plan(14);

PREPARE jobs AS
SELECT * FROM vroom_jobs;

PREPARE shipments AS
SELECT * FROM vroom_shipments;

PREPARE vehicles AS
SELECT * FROM vroom_vehicles;

PREPARE matrix AS
SELECT * FROM vroom_matrix;

CREATE OR REPLACE FUNCTION no_crash()
RETURNS SETOF TEXT AS
$BODY$
DECLARE
params TEXT[];
params_new TEXT[];
subs TEXT[];
jobs_params TEXT[];
jobs_new TEXT;
separator TEXT;
BEGIN
  -- IF NOT min_version('0.2.0') THEN
  --   RETURN QUERY
  --   SELECT skip(1, 'Function is new on 0.2.0');
  --   RETURN;
  -- END IF;

  RETURN QUERY
  SELECT isnt_empty('jobs', 'Should be not empty to tests be meaningful');
  RETURN QUERY
  SELECT isnt_empty('shipments', 'Should be not empty to tests be meaningful');
  RETURN QUERY
  SELECT isnt_empty('vehicles', 'Should be not empty to tests be meaningful');
  RETURN QUERY
  SELECT isnt_empty('matrix', 'Should be not empty to tests be meaningful');

  params = ARRAY[
    '$$SELECT * FROM vroom_jobs$$',
    '$$SELECT * FROM vroom_shipments$$',
    '$$SELECT * FROM vroom_vehicles$$',
    '$$SELECT * FROM vroom_matrix$$'
  ]::TEXT[];
  subs = ARRAY[
    'NULL',
    'NULL',
    'NULL',
    'NULL'
  ]::TEXT[];

  jobs_params := ARRAY[
    'id',
    'location_index',
    'service',
    'delivery',
    'pickup',
    'skills',
    'priority',
    'time_windows_sql'
  ]::TEXT[];

  RETURN query SELECT * FROM no_crash_test('vrp_vroom', params, subs);

  -- FOR i IN 8..array_length(jobs_params, 1) LOOP
  --   jobs_new := '$$SELECT';
  --   separator := ' ';
  --   FOR j in 1..array_length(jobs_params, 1) LOOP
  --     IF i = j THEN
  --       jobs_new := jobs_new || separator || 'NULL AS ' || jobs_params[j];
  --     ELSE
  --       jobs_new := jobs_new || separator || jobs_params[j];
  --     END IF;
  --     separator := ', ';
  --   END LOOP;
  --   jobs_new := jobs_new || ' FROM vroom_jobs$$';
  --   params_new := params;
  --   params_new[1] := jobs_new;
  --   RETURN query SELECT * FROM no_crash_test('vrp_vroom', params_new, subs);
  --   -- RAISE WARNING '%', jobs_new;
  -- END LOOP;


END
$BODY$
LANGUAGE plpgsql VOLATILE;


SELECT * FROM no_crash();

ROLLBACK;
