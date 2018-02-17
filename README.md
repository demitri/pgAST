# pgAST
The Starlink AST library as a PostgreSQL plugin.

### Building and Installing the Extension

From top level of PgAST directory:

```bash
make clean; make
sudo make install
```

Then from `psql`:

```sql
DROP EXTENSION pgast;
CREATE EXTENSION pgast;
```

