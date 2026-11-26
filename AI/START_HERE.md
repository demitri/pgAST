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

AST 9.5.0 (released 2026-09-22) is the minimum: `src/pgast.h` stops the build
against anything older (owner's ruling, 2026-11-25). The code was also tested
against 9.2.14 before that ruling, and AI/ast_polygon_orientation.md records
where the two versions behave differently. The `SHLIB_LINK` list in
`src/Makefile` links the static AST libraries.

## Conventions

- Fail loud: an AST failure is an error, not `NULL`, `false` or a DEBUG log
  line. A bad header in a query over many rows aborts the whole statement;
  that is the intended behaviour (owner's ruling, 2026-11-25), so there is no
  NULL-returning variant. The README documents the deliberate `NULL` returns (a header that is
  not a 2D image with both axes at least 2 pixels long); keep that list and
  the code in step.
- AST failures are raised with `pgast_ast_error(code, ...)` (`src/pgast.h`),
  never a bare `ereport(ERROR, ...)`: AST's error status is global to the
  backend, and if an error leaves it set, every later AST call in that
  session silently does nothing. The macro clears it. `code` is
  `PGAST_BAD_INPUT` (22023) when the input cannot be used, or
  `PGAST_AST_FAILURE` (XX000) when AST fails on usable input; the README's
  Errors table is the user-facing list. AST's own messages are captured
  (`astSetPutErr` in `_PG_init`) into the error's DETAIL.
- AST contexts are counted: SQL functions start with `pgast_ast_enter()`,
  helpers use `pgast_ast_begin()`, and everything ends with `pgast_ast_end()`
  (or `pgast_ast_end_checked()` on a SQL function's success path, which raises
  if any AST call failed). An error longjmps past the `astEnd` calls between
  it and the SQL entry point; `pgast_ast_enter()` closes the contexts left
  open that way at the next call. `scripts/run_tests.sh` refuses raw
  `astBegin;`/`astEnd;` in `src/*.c`. `pgast_ast_error` itself closes every
  open context before raising, so AST errors leave nothing behind; the
  cleanup in `pgast_ast_enter` covers errors raised by PostgreSQL inside an
  AST context (e.g. out of memory), whose objects live until the next call.
- AST marks coordinates it cannot compute as `AST__BAD` without setting its
  error status; code that uses transformed coordinates must check for it
  (see `cutout_require_good` in `pgast_cutout.c`).
- Everything is ICRS: code that reads a WCS sets `System=ICRS` on the
  FrameSet right after `astRead`, so headers in other systems (Galactic,
  FK4, ...) give ICRS positions.
- Code that a review or a reader might take for dead but is not (or that
  fires but has no effect) carries a comment at the site saying what was
  measured, and when (owner's ruling, 2026-11-25). Do not remove such code
  on a reading of the source; measure it on the AST version in use.
- Polygon orientation: see `AI/ast_polygon_orientation.md`. Any code that
  builds an AstPolygon from vertices it did not choose must decide the
  inside explicitly.

## Open work

- GitHub issue #1: `pgast_icrs_polygon_overlaps_polygon`. Before it is
  implemented, decide the vertex-order convention for stored polygons. Today
  they follow AST's rule as given, which the README documents; the choice is
  between keeping that, normalizing to the smaller region, or rejecting the
  other order (see `AI/ast_polygon_orientation.md`). Related: AST accepts a
  "polygon" with fewer than three distinct vertices without error, and
  pgAST does not validate stored polygons.
- Production hygiene (from `AI/archive/2025-codex-issue-summary.md`, not yet
  done): `fprintf`/`printf` in `pgast_pgast.c`, `pgast_util.c` and
  `pgast_header2polygon.c` write to the server's stdout/stderr on every call;
  `wcs_test()` is installed as a public function.
