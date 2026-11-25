
//#include "ast.h"
#include "pgast.h"
#include "pgast_util.h"

// Number of open AST contexts; see pgast_ast_enter() in pgast.h.
int pgast_ast_depth = 0;
#include <math.h>

#define degToRad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0)
#define radToDeg(angleInRadians) ((angleInRadians) * 180.0 / M_PI)
/**
 Convert a POLYGON object to an array of points suitable to
 be passed to AST classes. This function assumes:
 
 - polygon points are in degrees
 - the points array has been preallocated
 
 @param polygon
 @points points a preallocated array
 */
void pgPolygon2astPoints(POLYGON *polygon, double *points)
{
	int n_vert = polygon->npts; // number of vertices/coordinate pairs (i.e. Point)
	Point *polygon_points = polygon->p;
	
	//points = (double*)palloc(n_vert * 2 * sizeof(double));
	for (int i=0; i < n_vert; i++) {
		//fprintf(stderr, "p%d: (%0.4f, %0.4f)\n", i, (points[i]).x, (points[i]).y);
		points[i] = deg2rad((polygon_points[i]).x);
		points[n_vert + i] = deg2rad((polygon_points[i]).y);
	}
}

/**
 Is the point (ra, dec), in radians, inside a Region on the sky?

 AST's own test (astTran2 on the Region) can report the near-exact antipode
 of a small polygon as inside it (measured; see AI/ast_polygon_orientation.md).
 So when the Region is small, an "inside" answer must also lie within the
 Region's bounding disc. The disc is used only when it is known to bound the
 Region: the Region is not negated (for a negated Region, astGetRegionDisc
 describes the stored polygon, i.e. the outside; measured), the radius is
 under 90 degrees, and AST places the disc's own centre inside the Region (a
 test far from the antipode). Otherwise AST's answer stands. The extra test
 can only turn an "inside" into "outside", and only for a point beyond a
 disc that encloses the Region, so it never rejects a point that is inside.
 */
int pgast_point_in_sky_region(AstRegion *region, double ra, double dec)
{
	double ra_out, dec_out;
	astTran2(region, 1, &ra, &dec, 1, &ra_out, &dec_out);
	if (!astOK)
		pgast_ast_error("pgAST (pgast_point_in_sky_region): astTran2 failed.");
	if (ra_out == AST__BAD)
		return 0;
	if (astGetI(region, "Negated"))
		return 1;

	double centre[2], radius;
	astGetRegionDisc(region, centre, &radius);
	if (!astOK || radius == AST__BAD)
		pgast_ast_error("pgAST (pgast_point_in_sky_region): astGetRegionDisc failed.");
	if (radius >= M_PI / 2)
		return 1;

	double cx_out, cy_out;
	astTran2(region, 1, &centre[0], &centre[1], 1, &cx_out, &cy_out);
	if (!astOK)
		pgast_ast_error("pgAST (pgast_point_in_sky_region): astTran2 failed.");
	if (cx_out == AST__BAD)
		return 1; // disc centre not inside: the disc is not known to bound the Region

	const double point[2] = { ra, dec };
	const double distance = astDistance(region, centre, point);
	if (!astOK || distance == AST__BAD)
		pgast_ast_error("pgAST (pgast_point_in_sky_region): astDistance failed.");
	return distance <= radius * (1.0 + 1e-9) + 1e-12;
}

/**
 Fill in the bounding box of a PostgreSQL polygon from its points, as
 PostgreSQL's own polygon input does (make_bound_box in geo_ops.c, which is
 not exported). PostgreSQL's polygon operators (&&, @>, box(polygon), ...)
 read this box first, so a polygon built in C without it gives wrong answers.
 */
void pgast_polygon_set_boundbox(POLYGON *polygon)
{
	BOX *box = &polygon->boundbox;
	box->low.x = box->high.x = polygon->p[0].x;
	box->low.y = box->high.y = polygon->p[0].y;
	for (int i=1; i < polygon->npts; i++) {
		if (polygon->p[i].x < box->low.x)  box->low.x  = polygon->p[i].x;
		if (polygon->p[i].x > box->high.x) box->high.x = polygon->p[i].x;
		if (polygon->p[i].y < box->low.y)  box->low.y  = polygon->p[i].y;
		if (polygon->p[i].y > box->high.y) box->high.y = polygon->p[i].y;
	}
}

/**
Convert a FITS header, provided in the form of a single C string, into a FitsChan object.
This function assumes it will be called in between astBegin/astEnd.
*/
AstFitsChan* cstring2fitsChan(const char *header)
{
	AstFitsChan *fitsChan;
	
// Create an empty FITS channel (AST object that holds a FITS header)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Wformat-zero-length"
	fitsChan = astFitsChan( NULL, NULL, "");
#pragma GCC diagnostic pop
	astPutCards( fitsChan, header ); // add all cards at once

	//ereport(DEBUG1,  (errmsg("header:\n%s", header)));
	
	if (!astOK) {
		pgast_ast_error("pgAST error (cstring2fitsChan): An error occurred when reading the FITS header.");
	}

	// rewind the FitsChan, see section 16.4 in sun211.pdf
	// This sets the current position of the cards to the first one.
	astClear(fitsChan, "Card");

	return fitsChan;
}

/**
Convert a POLYGON object to an AstPolygon object. This function assumes:
    - points are in degrees

 @param polygon
 @param frame an ASTSkyFrame; if NULL, assumes ICRS J2000
*/
AstPolygon* pgPolygon2astPolygon(POLYGON *polygon, AstSkyFrame *frame) //, double *points, AstFrame *frame)
{
	AstPolygon *ast_polygon;

	double *points;
	int n_vert = polygon->npts; // number of vertices/coordinate pairs (i.e. Point)
//	Point *polygon_points = polygon->p;
	
	pgast_ast_begin();
	
	points = (double*)palloc(n_vert * 2 * sizeof(double));
	pgPolygon2astPoints(polygon, points);
	
	AstSkyFrame *local_frame = frame; // not in the, you know, astro sense...
	if (frame == AST__NULL) {
		// no frame provided; create a basic frame
		//local_frame = astFrame(2, "unit(1)=deg,unit(2)=deg"); // # of axes, options
		local_frame = astSkyFrame("System=ICRS, Equinox=J2000");
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Wformat-zero-length"
	ast_polygon = astPolygon(local_frame,		// frame region is defined
							 n_vert,			// number of points in polygon
							 n_vert,			// number of elements along the second dimension of points array
							 points,			// the points
							 AST__NULL,			// uncertainty
							 "");				// options
#pragma GCC diagnostic pop
	
	pfree(points);

	if (!astOK) {
		pgast_ast_end();
		pgast_ast_error("pgAST (pgPolygon2astPolygon): AST could not build a polygon from the PostgreSQL polygon.");
	}

	astExport(ast_polygon);
	pgast_ast_end();
	
	return ast_polygon;
}

/*
void pgarray_to_double_array(ArrayType *array, double *data, int *nelements)
{
	Oid data_type;	
	int16 data_type_width;
	bool type_passed_by_value;
	char type_alignment_code;
	bool *null_flags;
	Datum *datum_array;	// array contents as PostgreSQL "Datum" objects
	
  	// get the data type of the elements
  	// (see pg_type.h) for all possible values
  	// 
  	// INT2OID    = int16_t  = small int
  	// INT4OID    = int32_t  = integer
  	// INT8OID    = int64_t  = bigint
  	// FLOAT4OID  = float_t  = real (float)
  	// FLOAT8OID  = double_t = double
  	// NUMERICOID = numeric
  	//
  	data_type = ARR_ELEMTYPE(array);
  	
  	// get "data_type_width" (other values not used)
  	get_typlenbyvalalign(data_type,
  						 &data_type_width,
  						 &type_passed_by_value,
  						 &type_alignment_code);
  	
  	// extract elements of array into an array of "Datum" objects
  	deconstruct_array(array,
  					  data_type,
  					  data_type_width,
  					  type_passed_by_value,
  					  type_alignment_code,
  					  &data,
  					  &null_flags,
  					  &nelements);

	data = (double*)palloc(nelements * sizeof(double);
	
  	switch (data_type) {
//  		case NUMERICOID:
//  			for (uint32_t i=0; i < n; i++) {
//  				double v = DatumGetFloat8(data[i]);
//  				ereport(DEBUG1, (errmsg("d[%d] = %f", i, v)));
//  			}
//  			break;

  		case INT2OID:
  			for (uint32_t i=0; i < n; i++) {
  				data[i] = (double)DatumGetInt16(data[i]);
  				ereport(DEBUG1, (errmsg("d[%d] = %f", i, v)));
  			}
  			break;

  		case INT4OID:
  			for (uint32_t i=0; i < n; i++) {
  				data[i] = (double)DatumGetInt32(data[i]);
  				ereport(DEBUG1, (errmsg("d[%d] = %f", i, v)));
  			}
  			break;

  		case INT8OID:
  			for (uint32_t i=0; i < n; i++) {
  				data[i] = (double)DatumGetInt64(data[i]);
  				ereport(DEBUG1, (errmsg("d[%d] = %f", i, v)));
  			}
  			break;

  		case FLOAT4OID:
  			for (uint32_t i=0; i < n; i++) {
  				data[i] = (double)DatumGetFloat4(data[i]);
  				ereport(DEBUG1, (errmsg("d[%d] = %f", i, v)));
  			}
  			break;

  		case FLOAT8OID:
  			for (uint32_t i=0; i < n; i++) {
  				data[i] = DatumGetFloat8(data[i]);
  				ereport(DEBUG1, (errmsg("d[%d] = %f", i, v)));
  			}
  			break;
  			
  		default:
			ereport(DEBUG1, (errmsg("Need to handle this datatype.")));
			break;
	}	
}
*/

/*
 This function is a wrapper around the astDistance function
 that calculates the great circle distance of two points on
 a sphere. Input and output values are in degrees.
 
 Three function signatures:
 x0, y0, x1, y1
 x0, y0, pg_point
 pg_point1, pg_point2
 */
PG_FUNCTION_INFO_V1(pgast_distance);
Datum
pgast_distance(PG_FUNCTION_ARGS) // double x0, y0, x1, y1
{
	// get function args
	// -----------------
	double x0 = PG_GETARG_FLOAT8(0);
	double y0 = PG_GETARG_FLOAT8(1);
	double x1 = PG_GETARG_FLOAT8(2);
	double y1 = PG_GETARG_FLOAT8(3);

	// internal variables
	// ------------------
	const double point1[2]= {deg2rad(x0), deg2rad(y0)};
	const double point2[2]= {deg2rad(x1), deg2rad(y1)};
	double distance;

	pgast_ast_enter();
	
	// Since we're not converting points from one frame to another,
	// just pick a sky frame to work in.
	AstSkyFrame *icrs_frame = astSkyFrame("System=ICRS, Equinox=J2000");

	distance = rad2deg(astDistance(icrs_frame, point1, point2));
	
	pgast_ast_end_checked();

	PG_RETURN_FLOAT8(distance);
}

PG_FUNCTION_INFO_V1(pgast_distance_radec_point); // x0,y0,point1
Datum
pgast_distance_radec_point(PG_FUNCTION_ARGS)
{
	// get function args
	// -----------------
	double x0 = PG_GETARG_FLOAT8(0);
	double y0 = PG_GETARG_FLOAT8(1);
	Point *p = PG_GETARG_POINT_P(2);
	
	// internal variables
	// ------------------
	const double point1[2]= {deg2rad(x0), deg2rad(y0)};
	const double point2[2]= {deg2rad(p->x), deg2rad(p->y)};
	double distance;
	
	pgast_ast_enter();
	
	// Since we're not converting points from one frame to another,
	// just pick a sky frame to work in.
	AstSkyFrame *icrs_frame = astSkyFrame("System=ICRS, Equinox=J2000");
	
	distance = rad2deg(astDistance(icrs_frame, point1, point2));
	
	pgast_ast_end_checked();
	
	PG_RETURN_FLOAT8(distance);
}

PG_FUNCTION_INFO_V1(pgast_distance_points); // point1,point2
Datum
pgast_distance_points(PG_FUNCTION_ARGS)
{
	// get function args
	// -----------------
	Point *p1 = PG_GETARG_POINT_P(0);
	Point *p2 = PG_GETARG_POINT_P(1);
	
	// internal variables
	// ------------------
	const double point1[2]= {deg2rad(p1->x), deg2rad(p1->y)};
	const double point2[2]= {deg2rad(p2->x), deg2rad(p2->y)};
	double distance;
	
	pgast_ast_enter();
	
	// Since we're not converting points from one frame to another,
	// just pick a sky frame to work in.
	AstSkyFrame *icrs_frame = astSkyFrame("System=ICRS, Equinox=J2000");
	
	distance = rad2deg(astDistance(icrs_frame, point1, point2));
	
	pgast_ast_end_checked();
	
	PG_RETURN_FLOAT8(distance);
}

