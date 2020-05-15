-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pgast" to load this file. \quit

CREATE  OR REPLACE FUNCTION wcs_test() RETURNS integer
AS '$libdir/pgast'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION pgast_point_in_polygon(double precision, double precision, text) RETURNS boolean
-- (text fits_header_as_string, float ra_degrees, float dec_degrees)
AS 'MODULE_PATHNAME', 'pgast_point_in_polygon_header'
COST 150
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION pgast_point_in_polygon(double precision, double precision, polygon) RETURNS boolean
-- (polygon region_polygon, double precision ra_degrees, double precision dec_degrees)
COST 150
AS 'MODULE_PATHNAME', 'pgast_point_in_polygon_poly'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION pgast_bounding_circle(text) RETURNS CIRCLE
-- (text fits_header_as_string)
AS 'MODULE_PATHNAME', 'pgast_bounding_circle_header'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION pgast_bounding_circle(polygon) RETURNS CIRCLE
AS 'MODULE_PATHNAME', 'pgast_bounding_circle_polygon'
LANGUAGE C IMMUTABLE STRICT;

-- Return a PostgreSQL "polygon" object that bounds the region defined by
-- the provided FITS header (as a single string).
CREATE OR REPLACE FUNCTION pgast_bounding_polygon(text) RETURNS POLYGON
AS 'MODULE_PATHNAME', 'pgast_bounding_polygon'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION pgast_icrs_polygon_overlaps_hdu(polygon, text) RETURNS bool
AS 'MODULE_PATHNAME', 'pgast_icrs_polygon_overlaps_hdu'
LANGUAGE C IMMUTABLE STRICT;
