# How AST decides the inside of a polygon

Read this before writing code that builds an `AstPolygon`, or that turns a
PostgreSQL `polygon` into one.

## The rule

A boundary on the sphere divides the sky into two regions. AST takes the
inside to be the region to the **left** of the boundary as the vertices are
traversed in the order supplied, **seen from inside the celestial sphere**
(SUN/211, Polygon class; the IVOA STC convention, AST >= 5.2).

Seen from inside the sphere with north up, east is to the left. So a small
box listed in increasing-RA order along its bottom edge, e.g.

    (149.99, 1.99) (150.01, 1.99) (150.01, 2.01) (149.99, 2.01)

is clockwise as seen on the sky, and AST reads it as **the whole sky except
the box**. The order that describes the box itself runs the other way
(decreasing RA along the bottom edge).

## What AST does with the vertices

The Polygon constructor (`EnsureInside` in AST's `polygon.c`) may store the
vertices reversed and set the `Negated` attribute, so that the stored
vertices always describe the smaller region. The **meaning** of the input
order is kept: the reversal is cancelled by the negation. Two consequences:

- `astGetRegionPoints` returns the stored order. For a negated Polygon those
  vertices describe the other region. Code that hands vertices to
  PostgreSQL must reverse them when `Negated` is set
  (`pgast_bounding_polygon.c` does this).
- `astNegate` changes only the flag, never the vertices.

`Bounded` cannot tell the two regions apart. In AST 9.5.0's source
(`GetBounded` in `polygon.c`) every Polygon on a SkyFrame is bounded. Probes
on 2026-11-25 agree for both 9.2.14 and 9.5.0: polygons built directly on an
ICRS SkyFrame and polygons built from the test FITS headers all reported
`Bounded = 1`, whichever region they described.

Code before 2026-11 relied on `Bounded` (which, on those probes, never
fired), plus a centre-pixel test that read uninitialised memory and had its
condition inverted. For every test header, at both parities and in both AST
versions, the polygon built from the mesh was the rest of the sky (it
contained the antipode, not the centre), and the inverted test with a
garbage point is what reversed it. It usually gave the right answer, but
only by accident: a random point is almost always in the larger region.
Removing the `Bounded` check is safe whether or not it can fire for other
headers, because the centre test now decides the orientation by itself.

The increasing-RA example above was checked against AST 9.2.14 and 9.5.0
with `pgast_point_in_polygon`: the centre is outside and the antipode is
inside.

## Where pgAST decides the inside

- **FITS headers** (`fitsheader2polygon`): the mesh order depends on the WCS
  (pixel parity, for one), so the code tests whether the centre pixel is
  inside, and rebuilds from the reversed vertices if it is not. If neither
  order contains the centre, it raises an error.
  `src/test/sql/header_region.sql` covers both parities.
- **Stored PostgreSQL polygons** (`pgPolygon2astPolygon`,
  `pgast_point_in_polygon_poly`): the vertices are used **as given**, so the
  AST rule above applies to the caller's data. Polygons that came from
  `pgast_bounding_polygon` are in the right order; hand-entered polygons may
  not be. Choosing the convention for stored polygons (normalize to the
  smaller region, or require the AST order and reject the other) is the open
  decision on GitHub issue #1.

## AST quirk: the near-exact antipode

AST's point-in-polygon test (`astTran2` on the Region, and `astPointInRegion`)
can report the near-exact antipode of a small polygon as inside it. The
geometry is well defined there (the antipode of a small box is plainly
outside); the fault is in AST's implementation. Measured 2026-11-25 under AST
9.2.14 and 9.5.0: for a 43-arcsec cutout box around (150, 2), AST reported
(330, -2) as inside. On a 0.01-degree grid of 81 points over the box's
antipodal image, that was the only such point, and points 1 degree from it
were outside.

`pgast_point_in_sky_region` (`pgast_util.c`), used by `pgast_point_in_polygon`,
corrects for it: for a small, un-negated Region whose bounding disc
demonstrably contains it, an "inside" answer must also lie within that disc.
`astGetRegionDisc` describes the *stored* polygon even when the Region is
negated (measured), which is why negated Regions are excluded. Tests use a
point 90 degrees away as their far-away point, plus one antipode case that
covers the correction.
