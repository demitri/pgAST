-- Regions derived from a FITS header: the region must be the image footprint,
-- never the rest of the sky.
--
-- Test images: 100 x 100 pixels, TAN projection, 0.001 deg/pixel, reference
-- pixel at the image centre, so each image spans about +/-0.05 deg in RA and
-- Dec around its centre. Both pixel parities are tested (CD1_1 < 0 is the
-- usual east-left orientation; CD1_1 > 0 is mirrored), because the parity
-- decides the order in which the boundary vertices come out of AST.

CREATE EXTENSION pgast;

-- Build a FITS header string (80-character cards, concatenated).
CREATE FUNCTION test_header(crval1 float8, crval2 float8, cd1_1 float8)
RETURNS text LANGUAGE sql IMMUTABLE AS $$
    SELECT string_agg(rpad(card, 80), '' ORDER BY n)
    FROM unnest(ARRAY[
        'SIMPLE  =                    T',
        'BITPIX  =                  -32',
        'NAXIS   =                    2',
        'NAXIS1  =                  100',
        'NAXIS2  =                  100',
        'CTYPE1  = ''RA---TAN''',
        'CTYPE2  = ''DEC--TAN''',
        'CRPIX1  =                 50.5',
        'CRPIX2  =                 50.5',
        format('CRVAL1  = %20s', crval1),
        format('CRVAL2  = %20s', crval2),
        format('CD1_1   = %20s', cd1_1),
        'CD1_2   =                  0.0',
        'CD2_1   =                  0.0',
        'CD2_2   =                0.001',
        'END'
    ]) WITH ORDINALITY AS t(card, n)
$$;

CREATE TABLE images (name text, header text);
INSERT INTO images VALUES
    ('east_left', test_header(150, 2, -0.001)),
    ('mirrored',  test_header(150, 2,  0.001));

-- Point in the header-derived region.
SELECT name,
       pgast_point_in_polygon(150.0,  2.0,  header) AS centre,
       pgast_point_in_polygon(150.04, 2.04, header) AS near_corner,
       pgast_point_in_polygon(151.0,  2.0,  header) AS one_deg_east,
       pgast_point_in_polygon(330.0, -2.0,  header) AS antipode
FROM images ORDER BY name;

-- The polygon returned by pgast_bounding_polygon must describe the same
-- footprint when it is read back as a polygon.
SELECT name,
       pgast_point_in_polygon(150.0,  2.0,  pgast_bounding_polygon(header)) AS centre,
       pgast_point_in_polygon(151.0,  2.0,  pgast_bounding_polygon(header)) AS one_deg_east,
       pgast_point_in_polygon(330.0, -2.0,  pgast_bounding_polygon(header)) AS antipode
FROM images ORDER BY name;

-- Overlap of a stored footprint with a header-derived region.
SELECT a.name AS polygon_of, b.name AS header_of,
       pgast_icrs_polygon_overlaps_hdu(pgast_bounding_polygon(a.header), b.header) AS same_place,
       pgast_icrs_polygon_overlaps_hdu(pgast_bounding_polygon(a.header),
                                       test_header(160, 2, -0.001)) AS ten_deg_away
FROM images a CROSS JOIN images b ORDER BY a.name, b.name;

-- Bounding circle: centred on the image, radius = half the diagonal,
-- 0.05 * sqrt(2) = 0.0707 deg.
SELECT name,
       round((center(pgast_bounding_circle(header)))[0]::numeric, 3) AS ra,
       round((center(pgast_bounding_circle(header)))[1]::numeric, 3) AS dec,
       round(radius(pgast_bounding_circle(header))::numeric, 3) AS radius
FROM images ORDER BY name;

-- A footprint larger than half the sky: 300 x 150 pixels, CAR projection,
-- 1 deg/pixel, centred on (0, 0), so it spans RA -150..+150 and Dec -75..+75.
-- (180, 0) lies in the 60-degree gap between its east and west edges. AST
-- stores such a polygon negated, so pgast_bounding_polygon must return the
-- vertices in the order that describes the footprint, not the gap.
CREATE FUNCTION test_header_big() RETURNS text LANGUAGE sql IMMUTABLE AS $$
    SELECT string_agg(rpad(card, 80), '' ORDER BY n)
    FROM unnest(ARRAY[
        'SIMPLE  =                    T',
        'BITPIX  =                  -32',
        'NAXIS   =                    2',
        'NAXIS1  =                  300',
        'NAXIS2  =                  150',
        'CTYPE1  = ''RA---CAR''',
        'CTYPE2  = ''DEC--CAR''',
        'CRPIX1  =                150.5',
        'CRPIX2  =                 75.5',
        'CRVAL1  =                  0.0',
        'CRVAL2  =                  0.0',
        'CDELT1  =                 -1.0',
        'CDELT2  =                  1.0',
        'END'
    ]) WITH ORDINALITY AS t(card, n)
$$;

SELECT pgast_point_in_polygon(0.0,   0.0, test_header_big()) AS centre,
       pgast_point_in_polygon(180.0, 0.0, test_header_big()) AS in_gap,
       pgast_point_in_polygon(0.0,   0.0, pgast_bounding_polygon(test_header_big())) AS polygon_centre,
       pgast_point_in_polygon(180.0, 0.0, pgast_bounding_polygon(test_header_big())) AS polygon_in_gap;

-- A header in another celestial system: results are still ICRS. The image
-- is centred on Galactic (l, b) = (0, 0), which is ICRS (RA, Dec) =
-- (266.40499, -28.93617) (IAU definition of the Galactic system).
CREATE FUNCTION test_header_galactic() RETURNS text LANGUAGE sql IMMUTABLE AS $$
    SELECT replace(replace(test_header(0, 0, -0.001),
                           '''RA---TAN''', '''GLON-TAN'''),
                   '''DEC--TAN''', '''GLAT-TAN''')
$$;

SELECT pgast_point_in_polygon(266.40499, -28.93617, test_header_galactic()) AS icrs_centre,
       pgast_point_in_polygon(0.0, 0.0, test_header_galactic()) AS galactic_numbers,
       round((center(pgast_bounding_circle(test_header_galactic())))[0]::numeric, 3) AS circle_ra,
       round((center(pgast_bounding_circle(test_header_galactic())))[1]::numeric, 3) AS circle_dec,
       pgast_point_in_polygon(266.40499, -28.93617, pgast_bounding_polygon(test_header_galactic())) AS polygon_icrs_centre;

-- pgast_cutout with the Galactic header: the requested centre is ICRS. A
-- 36 arcsec (0.01 deg) cutout around the image centre must contain that
-- centre and not a point 0.02 deg away.
SELECT pgast_point_in_polygon(266.40499, -28.93617,
           pgast_cutout(test_header_galactic(), 266.40499, -28.93617, 36.0)) AS cutout_centre,
       pgast_point_in_polygon(266.40499, -28.91617,
           pgast_cutout(test_header_galactic(), 266.40499, -28.93617, 36.0)) AS cutout_0p02_north;

-- An AST failure must raise an error, and must not leave AST's (global)
-- error status set: if it did, every later call in the same session would
-- silently do nothing. 'RA---QQQ' is not a FITS projection code.
-- (Terse verbosity: the AST status number in the error detail is not part of
-- what is tested.)
\set VERBOSITY terse
SELECT pgast_bounding_polygon(replace(test_header(150, 2, -0.001), '''RA---TAN''', '''RA---QQQ''')) IS NULL AS unknown_projection;
SELECT pgast_bounding_polygon(replace(test_header(150, 2, -0.001), 'NAXIS1  =                  100', 'NAXIS1  =                ''abc''')) IS NULL AS malformed_naxis1;
SELECT pgast_cutout(replace(test_header(150, 2, -0.001), '''RA---TAN''', '''RA---QQQ'''), 150.0, 2.0, 36.0) IS NULL AS cutout_unknown_projection;
SELECT pgast_point_in_polygon(150.0, 2.0, test_header(150, 2, -0.001)) AS centre_after_error;
\set VERBOSITY default
