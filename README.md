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

### Debugging

Log messages can be placed in the C code. Each message should specify a log level, one of:

```
DEBUG5, DEBUG4, DEBUG3, DEBUG2, DEBUG1,
LOG, NOTICE, WARNING, ERROR
```

These are in descending order of verbosity. Use the `ereport` function to send a log message, specifying the log level (first parameter):

```c
ereport(NOTICE, (errmsg("Log message here.")));
```
This above is an abbreviated version of the more vebose method, which specifies an error code:

```c
ereport(NOTICE,
	    (
	     errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
		 (errmsg("Header:\n"))
		)
		);
```

And for an even longer version:

```c
ereport(ERROR,
            (
             errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
             errmsg("negative values are not allowed"),
             errdetail("value %d is negative", arg),
             errhint("make it positive")
            )
        );
```

See [this blog post](http://big-elephants.com/2015-10/writing-postgres-extensions-part-iii/) for more detail.

The settings in `postgresql.conf` determines which level of log messages are sent to the client and/or the server log. The details can be found [here](https://www.postgresql.org/docs/9.4/static/runtime-config-logging.html#RUNTIME-CONFIG-LOGGING-WHEN).

For example, to view `DEBUG1` messages and higher (less verbose) in the client (e.g. `psql`), place this line in `postgresql.conf` and restart the server:

```
client_min_messages = debug1
```

### Useful Links

* <http://apprize.info/php/postgresql_2/10.html>

