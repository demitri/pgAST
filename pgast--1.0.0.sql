-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pgast" to load this file. \quit

CREATE  OR REPLACE FUNCTION wcs_test() RETURNS integer
AS '$libdir/pgast'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION pgast_point_in_polygon(text, float, float) RETURNS boolean
AS '$libdir/pgast'
LANGUAGE C IMMUTABLE STRICT;

