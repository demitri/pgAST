# pgAST: Claude Code instructions

**First step of every session:** read `AI/START_HERE.md` (what the project is,
layout, build and test, conventions, open work). Do not duplicate it here.

## Rules

- **This repository is public.** Never name private projects, internal
  hosts or people in commits, code, docs or issues. Describe a downstream
  need generically ("a catalogue of sky regions").
- **Polygon orientation is decided explicitly.** Read
  `AI/ast_polygon_orientation.md` before writing code that builds an
  `AstPolygon`. Code that lets AST's vertex-order rule silently turn a
  footprint into "the rest of the sky" is a bug.
- **Fail loud.** AST failures raise `ereport(ERROR, ...)`; never map them to
  `NULL`, `false` or a log line.
- **Tests come with fixes.** Run `scripts/run_tests.sh` (see START_HERE) before
  committing C or SQL changes, and add a case to the existing test file for the
  behaviour you fixed. Expected values come from stated geometry, not from
  pasting what the code printed.
- Commits: short sentence-style subjects, as in the existing history. Stage by
  path; never `git add -A`.

## Review-policy addendum

Tickets go to this repository's GitHub issues (`demitri/pgAST`).
