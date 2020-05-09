# pgAST

### The Starlink AST library as a PostgreSQL plug-in.

Author: Demitri Muna

pgAST is a PostgreSQL plug-in that implements some functionality from the [Starlink AST](https://github.com/Starlink/ast), a library for handling world coordinate systems (WCS) in astronomy. Only a few functions are currently defined; more will be added over time.

## Building and Installing the Extension

Building pgAST requires the [Starlink AST Library](https://github.com/Starlink/ast) to be installed. Note the path where this is installed to use for the `make` command (i.e. replace `/usr/local/ast` below with your location).

From top level of PgAST directory:

```bash
make clean; make AST=/usr/local/ast
sudo make install
```

Then from `psql`:

```sql
DROP EXTENSION pgast;
CREATE EXTENSION pgast;
```

## Documentation

### Data Types
Note that many of these functions accept and return native PostgreSQL geometric types (i.e. *not* PostGIS), such as [Circles](https://www.postgresql.org/docs/current/datatype-geometric.html#DATATYPE-CIRCLE), [Polygons](https://www.postgresql.org/docs/current/datatype-geometric.html#DATATYPE-POLYGON), and [Points](https://www.postgresql.org/docs/current/datatype-geometric.html#id-1.5.7.16.5). A reference of geometric functions supported by the database for use with these objects can be found [here](https://www.postgresql.org/docs/12/functions-geometry.html).

All geometries are on the celestial sphere and in the ICRS coordinate frame unless otherwise noted.

### API

The functions defined by this extension are:

```
pgast_bounding_circle(text fits_header)
pgast_bounding_polygon(text fits_header_as_string)
pgast_point_in_polygon(polygon polygon, float8 ra, float8 dec)
pgast_bounding_circle(text fits_header_as_string)
pgast_bounding_circle(polygon polygon)
```


### `pgast_bounding_circle`

##### Returns the minimum bounding circle of a 2D image or a polygon.

This function has two signatures; it accepts a `text` value of a FITS header or a PostgreSQL Polygon object.

```sql
pgast_bounding_circle(fits_header_as_string)
pgast_bounding_circle(polygon)
```

The return value is a native PostgreSQL Circle record.

The FITS header must be provided as a single string, i.e. each 80 character record concatenated. Only the header records that define the WCS and pixel dimensions of the image are necessary, but it's usually easier to pass in the full header. The header as a string can be stored in the database in another column or concatenated using another function. The circle center point is defined as (ra,dec) and the radius is in degrees.

Values of the circle can be read as:

```
-- radius in degrees
SELECT RADIUS(pgast_bounding_circle(fits_header_as_string)) FROM ...
```

```sql
-- return the center of the circle (Point object) as ra,dec
SELECT POINT(pgast_bounding_circle(fits_header_as_string)) FROM ...
```

### `pgast_bounding_polygon`

##### Return a polygon that describes the perimeter of a 2D FITS image.

The return value is a native PostgreSQL Polygon record.

```sql
pgast_bounding_polygon(text fits_header_as_string)
```

### `pgast_point_in_polygon`

##### Test whether an (ra,dec) point is within a given polygon.

Returns `true` or `false`.

```sql
pgast_point_in_polygon(polygon polygon, float8 ra, float8 dec)
```

## Developer Notes

This section is intended for those who are interested in working with the source code.

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
This above is an abbreviated version of the more verbose method, which specifies an error code:

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

## Useful Links

* <http://apprize.info/php/postgresql_2/10.html>

