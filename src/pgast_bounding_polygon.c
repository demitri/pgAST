//
// pgast_bounding_polygon.c
//
//  Created by Demitri Muna on 2/12/20.
//  Copyright © 2020 Demitri Muna. All rights reserved.
//

#include "pgast.h"
#include "pgast_header2polygon.h"

/**
 This function takes a FITS header as a single string and returns the polygon that describes the perimeter of the image.
 @param text FITS header as a single string
 */
PG_FUNCTION_INFO_V1(pgast_bounding_polygon);
Datum
pgast_bounding_polygon(PG_FUNCTION_ARGS) // (text fits_header_as_string)
{
	// get function args
	// -----------------
	char* fits_header = text_to_cstring(PG_GETARG_TEXT_PP(0)); // return a null-terminated C string

	// internal variables
	// ------------------
	POLYGON *pg_polygon;
	int base_size, size;
	int maxpoint;
	int npoints;
	int maxcoord = 2; // number of axis values per point (e.g. 2D image = 2)
	double *points;
	
	astBegin;
	
	AstPolygon *sky_polygon = fitsheader2polygon(fits_header, &npoints);
	if (sky_polygon == AST__NULL) {
		// for example, incomplete FITS header or header does not contain WCS, image, etc.
		ereport(DEBUG1, (errmsg("AST polygon could not be created from header")));
		astEnd;
		PG_RETURN_NULL();
	}

	ereport(DEBUG1, (errmsg("returned polygon has %d coordinates", npoints)));

	// --------------------------------------------------------------------------
	// Get points from polygon.
	// -----------------------
	// Need to run twice - first time to know how many points will
	// be returned (to allocate the points array), then to get the points.
	//
// 	maxpoint = 0; // means to just return the point count
// 	astGetRegionPoints(sky_polygon,
// 					   maxpoint,	// maxpoint = 0, just return the number of points
// 					   maxcoord,	// ignored
// 					   &npoints,	// number of points (ignored)
// 					   NULL);		// points (ignored)
	
	points = (double*)palloc0(maxcoord*npoints * sizeof(double));
	maxpoint = npoints; // length of second dimension of points array
	
	// second call - get points
	astGetRegionPoints(sky_polygon,
					   maxpoint,	// length of second dimension
					   maxcoord,
					   &npoints,
					   points);		// array of points
	
	// returned array is [x1, x2, x3, ..., xn, y1, y2, y3, ... yn] in RADIANS
	// --------------------------------------------------------------------------
	
	ereport(DEBUG1, (errmsg("points[0] = %.5f, %.5f", points[0], points[1])));

	// Allocate space for PostgreSQL polygon.
	//
	base_size = sizeof(pg_polygon->p[0]) * npoints;
	size = base_size + offsetof(POLYGON, p);
	pg_polygon = (POLYGON*)palloc0(size);
	SET_VARSIZE(pg_polygon, size);
	pg_polygon->npts = npoints;

	// Fill points into polygon.
	// Note that PostgreSQL polygons points are float8.
	//
	for (int i=0; i < npoints; i++) {
		ereport(DEBUG1, (errmsg("point[%d] = %.9f", i, rad2deg(points[i]))));
		pg_polygon->p[i].x = (float8) rad2deg(points[i]);
		pg_polygon->p[i].y = (float8) rad2deg(points[npoints + i]);;
	}

	astEnd;
	
	PG_RETURN_POLYGON_P(pg_polygon);
}
