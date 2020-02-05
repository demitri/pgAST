-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pgast" to load this file. \quit

CREATE  OR REPLACE FUNCTION wcs_test() RETURNS integer
AS '$libdir/pgast'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION pgast_point_in_polygon(text, double precision, double precision) RETURNS boolean
-- (text fits_header_as_string, float ra_degrees, float dec_degrees)
AS 'MODULE_PATHNAME', 'pgast_point_in_polygon_header'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION pgast_point_in_polygon(polygon, double precision, double precision) RETURNS boolean
-- (polygon region_polygon, double precision ra_degrees, double precision dec_degrees)
AS 'MODULE_PATHNAME', 'pgast_point_in_polygon_poly'
LANGUAGE C IMMUTABLE STRICT;

--CREATE OR REPLACE FUNCTION pgast_bounding_circle(text) RETURNS ??
-- (text fits_header_as_string)
--AS '$libdir/pgast'
--LANGUAGE C IMMUTABLE STRICT;
