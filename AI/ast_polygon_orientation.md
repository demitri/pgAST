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

`Bounded` cannot tell the two regions apart: in AST 9.5.0 every Region on a
SkyFrame reports `Bounded = 1`. Code before 2026-11 relied on `Bounded`, plus
a centre-pixel test that read uninitialised memory and had its condition
inverted. It usually gave the right answer, but only by accident: a random
point is almost always in the larger region.

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
