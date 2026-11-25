# Issue Summary: Tighten pgAST Extension for Production Use

## Problem
The pgAST extension exposes a useful Starlink AST wrapper, but the current build still ships developer-only scaffolding (verbose stderr logging, sample `wcs_test()` entry point with embedded FITS header) and lacks automated validation and deployment guidance. This results in noisy server logs, unnecessary SQL-visible surface area, and no safety net for regressions.

## Scope
- Audit and remove or gate developer diagnostics (`fprintf`, `printf`, `astShow`) and ensure production-safe logging via `ereport`.
- Drop or relocate the `wcs_test()` helper and bundled FITS header so only supported functions are installed.
- Improve error handling paths that silently return `NULL` to client code.
- Enable a regression test harness (`REGRESS` in `src/Makefile`) with basic SQL scripts covering core geometry operations.
- Extend docs with integration guidance (AST install expectations, logging configuration, manual validation hints).

## Impact
These changes will make the extension quieter in production, reduce risk from debug artifacts, provide clearer feedback to users, and add a foundation for future regression testing and documentation.

