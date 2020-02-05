//
//  pgast_bounding_circle.c
//
//  Created by Demitri Muna on 2/5/20.
//  Copyright © 2020 Demitri Muna. All rights reserved.
//

//#include <stdlib.h>
#include "pgast.h"
#include "pgast_header2polygon.h"

PG_FUNCTION_INFO_V1(pgast_bounding_circle_header);
Datum
pgast_bounding_circle_header(PG_FUNCTION_ARGS) // (text fits_header)
{
	// get function args
	// -----------------
	char* fits_header = text_to_cstring(PG_GETARG_TEXT_PP(0)); // return a null-terminated C string
	
	// internal variables
	double centre[2];
	CIRCLE *circle = (CIRCLE*)palloc(sizeof(CIRCLE));
	
	astBegin;

	AstPolygon *sky_polygon = fitsheader2polygon(fits_header);
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
