# pgAST Project Assessment

## Project Overview
- C-based PostgreSQL extension that wraps parts of the Starlink AST WCS library so astronomy-centric geometry can run inside SQL queries.
- Build pipeline lives in `src/Makefile`; it requires an external AST install provided through `AST=/path/to/ast` and links the AST static libraries before registering SQL objects from `src/pgast--1.1.0.sql`.
- Runtime helpers are concentrated in `pgast_util.c` (coordinate conversions and AST integration helpers) and `pgast_header2polygon.c` (converting FITS headers into AST polygons); sample code in `examples/` demonstrates standalone AST usage.

## Current Assessment
- Core functionality follows standard PostgreSQL extension patterns (`MODULE_big`, immutable/strict SQL bindings). FITS-to-polygon conversion logic is thorough and demonstrates deep familiarity with AST.
- Developer-only scaffolding (verbose logging, bundled `wcs_test()` function with a large FITS header literal) is still compiled and installed, creating noisy logs and unnecessary risk in production deployments.
- Documentation is limited to the README; regression hooks exist but automated tests are absent, leaving behavior verification entirely manual.

## Improvement Suggestions
- **Reduce noisy diagnostics:** Replace raw `fprintf`, `printf`, and unconditional `astShow` calls with `ereport` at controlled levels or guard them behind compile-time flags to prevent every query from spamming server logs.
- **Remove dev-only entry points:** Drop or quarantine the exported `wcs_test()` function and move sample FITS headers to `examples/`; keep the SQL-facing API production-ready.
- **Harden error reporting:** Where functions currently return `NULL` for recoverable issues (e.g., malformed headers in `pgast_bounding_circle_header`), consider raising `ereport(ERROR, ...)` with actionable messages, relying on `STRICT` to handle null inputs.
- **Enable regression coverage:** Activate the commented `REGRESS` target in `src/Makefile` and add psql-based regression scripts that exercise bounding geometry, region tests, and distance calculations to catch future regressions.
- **Document integration workflow:** Extend README or add docs describing AST installation expectations, logging configuration tips, and manual validation steps for complex routines like cutouts and overlaps.

