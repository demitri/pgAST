//
//  pgast_bounding_circle.c
//
//  Created by Demitri Muna on 9 May 2020.
//  Copyright © 2020 Demitri Muna. All rights reserved.
//

#include <stdbool.h>

#include "pgast_util.h"
#include "pgast_header2polygon.h"

/**
 This function takes two polygons along with an associated FITS header for each and returns 'true' if they overlap, false if not.
 
 pgast_icrs_polygon_overlaps_hdu(polygon icrs_polygon1, text fits_header)
 
 @param icrs_polygon a polygon in the ICRS frame, J2000 epoch
 @param fits_header  a 2D image FITS header as a single C string; the WCS is applied to derived region
 */
PG_FUNCTION_INFO_V1(pgast_icrs_polygon_overlaps_hdu);
Datum
pgast_icrs_polygon_overlaps_hdu(PG_FUNCTION_ARGS) // (polygon a_polygon, text fits_header)
{
	// get function args
	// -----------------
	POLYGON *pgpolygon;
	char* fits_header = text_to_cstring(PG_GETARG_TEXT_PP(1)); // return a null-terminated C string
	
	if (PG_ARGISNULL(0)) {
		ereport(ERROR, (errmsg("pgAST: The polygon parameter must not be NULL.")));
		PG_RETURN_NULL();
	} else {
		pgpolygon = PG_GETARG_POLYGON_P(0);
	}
	
	// polygon 1 = provided polygon
	// polygon 2 = polygon from FITS header
	
	// internal variables
	// ------------------
	AstSkyFrame *icrsFrame;
	AstPolygon *astPolygon1, *astPolygon2;
	int overlap_value;
	int npoints_polygon2;

	// return value
	// ------------
	bool polygons_overlap;

	astBegin;

	// polygon 1
	// ---------
	icrsFrame = astSkyFrame("System=ICRS, Equinox=J2000");
	astPolygon1 = pgPolygon2astPolygon(pgpolygon, icrsFrame);

	if (astPolygon1 == AST__NULL) {
		ereport(DEBUG1, (errmsg("PostgreSQL polygon could not be converted to an AST polygon.")));
		astEnd;
		PG_RETURN_NULL();
	}
	
	// polygon 2
	// ---------
	astPolygon2 = fitsheader2polygon(fits_header, &npoints_polygon2);

	if (astPolygon2 == AST__NULL) {
		ereport(DEBUG1, (errmsg("AST polygon could not be created from FITS header")));
		astEnd;
		PG_RETURN_NULL();
	}
	
	overlap_value = astOverlap(astPolygon1, astPolygon2);
	
	ereport(DEBUG1, (errmsg("AST overlap value = %d", overlap_value)));

	// From the documentation:
	// 0 - The check could not be performed because the second Region could not be mapped
	//     into the coordinate system of the first Region.
	// 1 - There is no overlap between the two Regions.
	// 2 - The first Region is completely inside the second Region.
	// 3 - The second Region is completely inside the first Region.
	// 4 - There is partial overlap between the two Regions.
	// 5 - The Regions are identical to within their uncertainties.
	// 6 - The second Region is the exact negation of the first Region to within their uncertainties.

	polygons_overlap = (overlap_value == 2 || overlap_value == 3 || overlap_value == 4 || overlap_value == 5);
	
	astEnd;
	
	PG_RETURN_BOOL(polygons_overlap);
}
