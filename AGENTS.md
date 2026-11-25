# pgAST: agent instructions (Codex and other tools)

**First step of every session:** read `AI/START_HERE.md` for what the project
is, the layout, build and test commands, conventions and open work. The rules
in `CLAUDE.md` apply to every agent, not only Claude; read it too. Neither
file is duplicated here.

Quick reference:

```bash
make -C src AST=/path/to/ast              # build
AST=/path/to/ast scripts/run_tests.sh     # build + throwaway cluster + pg_regress (PostgreSQL >= 18)
```
