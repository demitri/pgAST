//
// pgast_cutout.c
//
//  Created by Demitri Muna on 3/22/21.
//  Copyright © 2021 Demitri Muna. All rights reserved.
//

//#include <math.h>
#include <tgmath.h> // type generic version, see: https://www.cplusplus.com/reference/ctgmath/
#include "pgast.h"
#include "pgast_util.h"

#define PIXEL2WORLD 1
#define WORLD2PIXEL 0

/**
 This function takes a sky position and a returns a polygon that describes a cutout with the provided side length.
 @param fits_header C string of FITS header used to construct the WCS
 @param ra right ascension coordinate in degrees
 @param dec declination coordinate in degrees
 @param box_side_length length of the cutout box in arcseconds
 */
PG_FUNCTION_INFO_V1(pgast_cutout);
Datum
pgast_cutout(PG_FUNCTION_ARGS) // (fits_header text, ra double, dec double, side_length double)
{
	// get function args
	// -----------------
	char* fits_header = text_to_cstring(PG_GETARG_TEXT_PP(0)); // return a null-terminated C string
	double ra  = PG_GETARG_FLOAT8(1);// in degrees
	double dec = PG_GETARG_FLOAT8(2); // in degrees
	double side_length = PG_GETARG_FLOAT8(3); // in arcsec
	
	// internal variables
	// ------------------
	POLYGON *pg_polygon;
	int base_size, size;
	
	double half_side_length = deg2rad(side_length/2.);
	FitsChan *fitsChan;
	AstFrameSet *wcs_frames;
	const double p0_pix[2], p0_world[2];
	const double p1_pix[2], p1_world[2];
	const double p2_pix[2], p2_world[2];
	const double x1_pix[2], x1_world[2];
	const double x2_pix[2], x2_world[2];
	int dx, dy; // lengths of sides of cutout in pixels
	double corners_pix_x[4], corners_pix_y[4];  // vertices of final polygon
	double corners_world_ra[4], corners_world_dec[4];
	
	/*
	
	Since all of the angular operations here happen on a sphere, Cartesian identities
	(e.g. Pythagorean Theorem) don't apply. Further, FITS pixels cannot be assumed to
	be square - the angular height may differ from the angular width.
	
	The AST function we are relying on here is astOffset. This takes two points and
	calculates a third point offset from the first by an angular distance. The points
	are connected by a great circle.
	
	Algorithm:
	1. Start at the position provided, p0.
	2. Determine a point p1 that is straight down on the Cartesian (pixel) grid.
	3. Find the point 1/2 box side length from p0 in the direction of p1; this is x1.
	4. Determine a point p2 that is along the horizontal pixel axis.
	5. Find the point 1/2 box side length from x1 towards p2; this is x2.
	6. Get the delta ra and delta dec by subtracting p0 - x2.
	7. Construct a polygon from x2 from the values in the previous step.
	
	Throughout this we will be converting between the pixel frame and the world frame.
	
	This is a diagram of the final cutout, which is within the original FITS pixel grid.
	
	            <------------  dx (pix) -------->
				----------------------------------   ^
				|                                |   |
				|                                |   |
				|                                |   |
				|               p0               |   dy (pix)
				|                                |   |
				|                                |   |
				|                                |   |
		p2		x2--------------x1----------------   
				
				                p1
	*/
	
	astBegin;
	
	// convert provided points into radians as expected by AST
	p0_world[0] = rad2deg(ra);
	p0_world[1] = rad2deg(dec);
	
	// ---------------------------------------------
	// Read the WCS info from the header.
	fitsChan = cstring2fitsChan(header);
	wcs_frames = astRead(fitsChan);
	
	if (wcs_frames == AST__NULL) {
		ereport(ERROR, (errmsg("pgAST error (pgast_cutout): No valid WCS could be read from the provided header.")));
		astEnd;
		return AST__NULL;
	}

	// The resulting object contains two frames:
	// the pixel frame and the "sky" frame.
	pixel_frame = astGetFrame(wcs_frames, AST__BASE);
	sky_frame = astGetFrame(wcs_frames, AST__CURRENT);
	if (astIsASkyFrame(sky_frame) == 0) { // returns 1 if yes, 0 if no
		ereport(ERROR, (errmsg("pgAST error (fitsheader2polygon): The provided WCS does not contain a sky frame.")));
		astEnd;
		return AST__NULL;
	}
	// ---------------------------------------------

	// calculate p0_pixel
	astTran2(wcs_frames,                   // mapping
	         1,                            // number of points to be transformed
			 &p0_world[0], &p0_world[1],   // untransformed points (in)
			 WORLD2PIXEL,                  // non-zero == forward transform
			 &p0_pixel[0], &p0_pixel[1]);  // transformed points (out)

	// find p1 by moving to bottom of pixel grid, i.e. straight down from p0
	p1_pixel[0] = p0_pix[0];
	p1_pixel[1] = -1; // use -1 instead of zero in case p0 is at edge of image
	
	// calculate p1_world
	astTran2(wcs_frames,                   // mapping
			 1,                            // number of points to be transformed
			 &p1_pixel[0], &p1_pixel[1],   // untransformed points (in)
			 PIXEL2WORLD,                  // non-zero == forward transform
			 &p1_world[0], &p1_world[1]);  // transformed points (out)

	// find x1 as an offset from p0 towards p1
	astOffset(wcs_frames, p0_world, p1_world, half_side_length, x1_world);
	
	// get pixel location x1_world -> x1_pixel
	astTran2(wcs_frames,                   // mapping
			 1,                            // number of points to be transformed
			 &x1_world[0], &x1_world[1],   // untransformed points (in)
			 WORLD2PIXEL,                  // non-zero == forward transform
			 &x1_pixel[0], &x1_pixel[1]);  // transformed points (out)

	// define p2: move along other pixel axis from x1_pixel
	p2_pix[0] = x1_pixel[0] - 10;  // arbitrary distance
	p2_pix[1] = x2_pixel[1]; // hold constant
	
	astTran2(wcs_frames,                   // mapping
			 1,                            // number of points to be transformed
			 &p2_pixel[0], &p2_pixel[1],   // untransformed points (in)
			 PIXEL2WORLD,                  // non-zero == forward transform
			 &p2_world[0], &p2_world[1]);  // transformed points (out)
	
	// find x2 as an offset from x1 to p2
	astOffset(wcs_frames, x1_world, p2_world, half_side_length, x2_world);
	
	// determine dx, dy - number of pixels corresponding to the length of the cutout on each dimension
	dx = (p0_pixel[0] - x2_pixel[0]) * 2.;
	dy = (p0_pixel[1] - x2_pixel[1]) * 2.;
	dx = sign(dx) * ceil(abs(dx)); // use ceil so that user specified length is the minimum
	dx = sign(dy) * ceil(abs(dy)); // 
	
	// corner 1
	corners_pix_x[0] = p0_pixel[0] - dx/2;
	corners_pix_y[0] = p0_pixel[1] - dy/2;
	
	// corner 2
	corners_pix_x[1] = corners_pix_x[0];
	corners_pix_y[1] = corners_pix_y[0] + dy;
	
	// corner 3
	corners_pix_x[2] = corners_pix_x[0] + dx;
	corners_pix_y[2] = corners_pix_y[0] + dy;
	// corner 4
	corners_pix_x[3] = corners_pix_x[0] + dx;
	corners_pix_y[3] = corners_pix_y[0];
	
	// convert all corner points to world
	astTran2(wcs_frames,
			 4,
			 &corners_pix_x, &corners_pix_y,
			 PIXEL2WORLD,
			 &corners_world_ra, &corners_world_dec);
	
	// Allocate space for PostgreSQL polygon.
	//
	base_size = sizeof(pg_polygon->p[0]) * 4;
	size = base_size + offsetof(POLYGON, p);
	pg_polygon = (POLYGON*)palloc0(size);
	SET_VARSIZE(pg_polygon, size);
	pg_polygon->npts = 4;
	
	// Fill points into polygon.
	// Note that PostgreSQL polygons points are float8.
	for (int i=0; i<4; i++) {
		pg_polygon->p[i].x = (float8) rad2deg(corners_world_ra[i]);
		pg_polygon->p[i].y = (float8) rad2deg(corners_world_dec[i]);
	}
	
	astEnd;
	
	PG_RETURN_POLYGON_P(pg_polygon);
}