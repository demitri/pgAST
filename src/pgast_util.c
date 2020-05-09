
#include "pgast.h"
#include "pgast_util.h"

/*
 Convert a POLYGON object to an array of points suitable to
 be passed to AST classes. This function assumes:
 
 - points are in degrees
 - the points array has been preallocated
 */
void pgpolygon2astPoints(POLYGON *polygon, double *points)
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

/*
Convert a POLYGON object to an AstPolygon object. This function assumes:
    - points are in degrees
    - the points array has been preallocated
*/
void pgpolygon2astPolygon(POLYGON *polygon, double *points, AstFrame *frame)
{
	int n_vert = polygon->npts; // number of vertices/coordinate pairs (i.e. Point)
	Point *polygon_points = polygon->p;
	
	astBegin;
	
	AstFrame *local_frame = frame;
	if (frame == AST_NULL) {
		// no frame provided; create a basic frame
		local_frame = astFrame(2, "unit(1)=deg,unit(2)=deg"); // # of axes, options
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Wformat-zero-length"
	polygon = astPolygon(frame,				// frame region is defined
						 n_vert,			// number of points in polygon
						 n_vert,			// number of elements along the second dimension of points array
						 polygon_points,	// the points
						 AST_NULL,			// uncertainty
						 "");				// options
#pragma GCC diagnostic pop
	
	astExport(polygon);
	astEnd;
	
	return polygon;
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

