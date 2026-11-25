# pgAST: AI agent onboarding

**Keep this document current.** It is the single entry point for every AI
session (Claude Code via `CLAUDE.md`, Codex via `AGENTS.md`). When you change
the layout, the build, the test harness or a convention, update it here.

## What the project is

A PostgreSQL extension (C, PGXS) that exposes parts of the
[Starlink AST](https://github.com/Starlink/ast) WCS library to SQL: bounding
circles and polygons of FITS images, point-in-region tests, polygon/HDU
overlap, distances, cutouts. User documentation is `README.md`, which lists
the SQL API.

All geometry is on the celestial sphere, ICRS, in degrees, with polygon
vertices joined by great circles. The geometric types are PostgreSQL's
native `point`, `circle` and `polygon` (not PostGIS).

## Layout

```
src/                      extension source (C), Makefile, control file, SQL install script
  pgast--1.1.0.sql        SQL declarations of every function
  pgast_header2polygon.c  FITS header -> AstPolygon (the core of most functions)
  pgast_util.c            PostgreSQL polygon <-> AST conversions, distances
  pgast_region_overlaps.c pgast_icrs_polygon_overlaps_hdu
  test/sql, test/expected pg_regress tests (REGRESS in src/Makefile)
  deprecated/             reference only; not built
scripts/run_tests.sh      build + throwaway cluster + pg_regress
examples/                 standalone AST sandboxes (not part of the build)
docs/                     Doxygen config
AI/                       this directory: agent docs
  ast_polygon_orientation.md  how AST decides the inside of a polygon (read before touching polygons)
  archive/                notes from earlier (2025) Codex sessions; not ratified, kept for reference
```

## Build and test

Build (from `src/`): `make AST=/path/to/ast`. Install: `sudo make install`,
then `DROP EXTENSION pgast; CREATE EXTENSION pgast;` in psql.

Test: `AST=/path/to/ast scripts/run_tests.sh`. It builds, installs into a
staging directory, starts a throwaway cluster that loads the extension from
there (`extension_control_path` / `dynamic_library_path`, so it needs
PostgreSQL >= 18 and no root), and runs `make installcheck`. Pass
`PG_CONFIG=/path/to/pg_config` to pick the PostgreSQL. On macOS the script
replaces PGXS's `PG_SYSROOT` with the current SDK when the SDK PostgreSQL was
built against has gone (happens after Xcode updates).

`src/pgast.control` uses the bare `module_pathname = 'pgast'` (not
`$libdir/pgast`) so that `dynamic_library_path` is honoured; keep it that way.

Test expectations must come from geometry the test states (image size, pixel
scale, centre), never from pasting what the code printed.

## AST versions

The code builds and passes the tests against AST 9.2.14 and 9.5.0 (the
latest release, 2026-09-22). The `SHLIB_LINK` list in `src/Makefile` links the
static AST libraries; the 9.5.0 build produces the same set.

## Conventions

- Fail loud: an AST failure is an error, not `NULL`, `false` or a DEBUG log
  line. The README documents the deliberate `NULL` returns (a header that is
  not a 2D image with both axes at least 2 pixels long); keep that list and
  the code in step.
- AST failures are raised with `pgast_ast_error(...)` (`src/pgast.h`), never a
  bare `ereport(ERROR, ...)`: AST's error status is global to the backend, and
  if an error leaves it set, every later AST call in that session silently
  does nothing. The macro clears it. Call `astEnd` for the current AST
  context first, because nothing after the error runs.
- Polygon orientation: see `AI/ast_polygon_orientation.md`. Any code that
  builds an AstPolygon from vertices it did not choose must decide the
  inside explicitly.

## Open work

- GitHub issue #1: `pgast_icrs_polygon_overlaps_polygon`. A review is on the
  issue. The open decision is the vertex-order convention for stored polygons
  (see `AI/ast_polygon_orientation.md`).
- Production hygiene (from `AI/archive/2025-codex-issue-summary.md`, not yet
  done): `fprintf`/`printf` in `pgast_pgast.c`, `pgast_util.c` and
  `pgast_header2polygon.c` write to the server's stdout/stderr on every call;
  `wcs_test()` is installed as a public function.
