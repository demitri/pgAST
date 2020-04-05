//
//  pgast_bounding_circle.c
//
//  Created by Demitri Muna on 2/5/20.
//  Copyright © 2020 Demitri Muna. All rights reserved.
//

//#include <stdlib.h>
#include "pgast.h"
#include "pgast_util.h"
#include "pgast_header2polygon.h"

PG_FUNCTION_INFO_V1(pgast_bounding_circle_header);
Datum
pgast_bounding_circle_header(PG_FUNCTION_ARGS) // (text fits_header)
{
	// get function args
	// -----------------
	char* fits_header = text_to_cstring(PG_GETARG_TEXT_PP(0)); // return a null-terminated C string
	
	// internal variables
	// ------------------
	double centre[2];
	CIRCLE *circle = (CIRCLE*)palloc(sizeof(CIRCLE));
	
	astBegin;

	int npoints;
	AstPolygon *sky_polygon = fitsheader2polygon(fits_header, &npoints);
	if (sky_polygon == AST__NULL) {
		// for example, incomplete FITS header or header does not contain WCS, image, etc.
		astEnd;
		PG_RETURN_NULL();
	}

	astGetRegionDisc(sky_polygon, centre, &(circle->radius));
	
	circle->center.x = rad2deg(centre[0]);
	circle->center.y = rad2deg(centre[1]);
	circle->radius = rad2deg(circle->radius);
	
	astEnd;
	
	PG_RETURN_CIRCLE_P(circle);
}

PG_FUNCTION_INFO_V1(pgast_bounding_circle_polygon);
Datum
pgast_bounding_circle_polygon(PG_FUNCTION_ARGS) // (polygon polygon)
{
	// get function args
	// -----------------
	POLYGON *polygon;
	
	if (PG_ARGISNULL(0)) {
		PG_RETURN_NULL();
		//ereport(ERROR, (errmsg("pgAST: The polygon parameter must not be NULL.")));
	} else
		polygon = PG_GETARG_POLYGON_P(0);
	
	// internal variables
	// ------------------
	double centre[2];
	CIRCLE *circle = (CIRCLE*)palloc(sizeof(CIRCLE));
	int n_vert = polygon->npts; // number of vertices/coordinate pairs
	
	double *points = (double*)palloc(n_vert * 2 * sizeof(double));
	pgpolygon2astPoints(polygon, points);
	
	astBegin;

	AstSkyFrame *icrsFrame = astSkyFrame("System=ICRS, Equinox=J2000"); // naxes, options
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-zero-length"
	AstPolygon *ast_polygon = astPolygon(icrsFrame,		// AstFrame
										n_vert,			// number of points in region
										n_vert,			// dimension along second axis [2][n]
										points,			// points array
										NULL,			// uncertainty
										"");			// options
#pragma GCC diagnostic pop

	if (ast_polygon == AST__NULL) {
		// for example, incomplete FITS header or header does not contain WCS, image, etc.
		astEnd;
		PG_RETURN_NULL();
	}

	astGetRegionDisc(ast_polygon, centre, &(circle->radius));

	circle->center.x = rad2deg(centre[0]);
	circle->center.y = rad2deg(centre[1]);
	circle->radius = rad2deg(circle->radius);

	astEnd;

	pfree(points);
	
	PG_RETURN_CIRCLE_P(circle);
}

