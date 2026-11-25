
//#include <math.h>

#include "ast.h" // Starlink AST library header
//#include "postgres.h"
#include "pgast_pgast.h"
#include "pgast_header2polygon.h"

//#define deg2rad(angleDegrees) ((angleDegrees) * M_PI / 180.0)
//#define rad2deg(angleRadians) ((angleRadians) * 180.0 / M_PI)

#define PIXEL2WORLD 1
#define WORLD2PIXEL 0

AstPolygon* fitsheader2polygon(const char *header, int *npoints) //, double *polygon, int *npoints)
{
	//int npoints;
	
	// Inputs
	// ------
	// header  : FITS header as single string
	//
	// Outputs
	// -------
	// polygon : 
	// npoints : number of points in the resulting polygon 

	// Notes
	// -----
	// * This function will allocate the memory needed 
	//   to hold the points in the polygon. It is the responsibility
	//   of the caller to free the memory!
	//
	// * It's not strictly necessary to pass the full FITS header.
	//   At the very least, the header keywords needed are:
	//   NAXIS, NAXIS1, NAXIS2,
	//   CTYPE1, CTYPE2, CRVAL1, CRVAL2, CUNIT1, CUNIT2,
	//   CRPIX1, CRPIX2, CD1_1, CD1_2, CD2_1, CD2_2
	//
	
	
	// initialize status
//	int status = 0;
	
	int dim1, dim2; // image dimensions (NAXIS1, NAXIS2)
	int surface, num_points, maxcoord, maxpoint;
	AstFitsChan *fitsChan;
	AstBox *pixelbox, *skybox;
	AstFrameSet *wcs_frames;
	AstFrame *pixel_frame, *sky_frame, *basic_frame;
	AstPolygon *flat_polygon, *reduced_flat_polygon, *sky_frame_polygon;
	double *points, *mesh_points;
	
	astBegin;
	
	//ereport(DEBUG1, (errmsg("fitsheader2polygon: start")));
	
	// Create an empty FITS channel (AST object that holds a FITS header)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Wformat-zero-length"
	fitsChan = astFitsChan( NULL, NULL, "");
#pragma GCC diagnostic pop
	astPutCards( fitsChan, header ); // add all cards at once

//	ereport(DEBUG1, (errmsg("fitsheader2polygon: after astPutCards")));
	
	//ereport(DEBUG1,  (errmsg("header:\n%s", header)));
	
	if (!astOK) {
		printf("pgAST error (fitsheader2polygon): An error occurred when reading the FITS header (status=%d).\n", astStatus);
		astEnd;
		return AST__NULL;
	}

	// rewind the FitsChan, see section 16.4 in sun211.pdf
	// This sets the current position of the cards to the first one.
	astClear(fitsChan, "Card");

	//ereport(DEBUG1,(errmsg("pgast: Header:")));
	//ereport(DEBUG1,(errmsg("%s", header)));
	
//	ereport(DEBUG1, (errmsg("fitsheader2polygon: after getFrames")));

	// Read the image dimensions from the header
	// -----------------------------------------
	{
		// read NAXIS value
		// ----------------
		int naxes;
		int success;
		success = astGetFitsI(fitsChan, "NAXIS", &naxes); // returns 0 if not found, 1 otherwise
		if (success == 0) {
			ereport(DEBUG1, (errmsg("pgAST error (fitsheader2polygon): astGetFits did not find 'NAXIS' keyword.")));
			astEnd;
			return AST__NULL;
		}
		else if (naxes != 2) {
			ereport(DEBUG1, (errmsg("pgAST error (fitsheader2polygon): header is not a 2D image (NAXIS=%d).", naxes)));
			astEnd;
			return AST__NULL;
		}
		// read NAXIS1 value
		// -----------------
		success = astGetFitsI(fitsChan, "NAXIS1", &dim1);
		if (success == 0) {
			ereport(DEBUG1, (errmsg("pgAST error (fitsheader2polygon): astGetFits did not find 'NAXIS1' keyword.")));
			astEnd;
			return AST__NULL;
		}
		else if (dim1 < 2) {
			ereport(DEBUG1, (errmsg("pgAST error (fitsheader2polygon): Each axis must have dimension >= 2 (NAXIS1=%d).", dim1)));
			astEnd;
			return AST__NULL;
		}

		// read NAXIS2 value
		// -----------------
		success = astGetFitsI(fitsChan, "NAXIS2", &dim2);
		if (success == 0) {
			ereport(DEBUG1, (errmsg("pgAST error (fitsheader2polygon): astGetFits did not find 'NAXIS2' keyword.")));
			astEnd;
			return AST__NULL;
		}
		else if (dim2 < 2) {
			ereport(DEBUG1, (errmsg("pgAST error (fitsheader2polygon): Each axis must have dimension >= 2 (NAXIS2=%d).", dim2)));
			astEnd;
			return AST__NULL;
		}
	}

	// ---------------------------------------------
	// Read the WCS info from the header.
	// It's better to get specific values from the header before calling
	// astRead as the function may delete the header if it's considered
	// part of the WCS information.
	astClear(fitsChan, "Card"); // reset position to start of header
	wcs_frames = astRead(fitsChan);
	
	if (wcs_frames == AST__NULL) {
		ereport(ERROR, (errmsg("pgAST error (fitsheader2polygon): No valid WCS could be read from the provided header.")));
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

	
	// Create a "box" object that covers the extent of the
	// image in pixel coordinates.
	//
	{
		const double point1[] = {0.5, 0.5};
		const double point2[] = {dim1+0.5, dim2+0.5};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Wformat-zero-length"
		pixelbox = astBox(pixel_frame,
						  1,
						  point1,
						  point2,
						  AST__NULL,
						  ""); // no options needed
#pragma GCC diagnostic pop

		if (pixelbox == AST__NULL) {
			ereport(WARNING, (errmsg("pgAST error (fitsheader2polygon): astBox returned NULL. (status=%d", astStatus)));
			astEnd;
			return AST__NULL;
		}
	}

//	ereport(DEBUG1, (errmsg("fitsheader2polygon: after astbox")));

	// Map the box in the pixel frame onto the sky frame.
	// The function knows to go from pixels-> sky
	// since that is the frame 'pixelbox' is in.
	// (Note that a frame set can also act as a mapping.)
	skybox = astMapRegion(pixelbox,
						  wcs_frames, // map
						  sky_frame); // frame

//	ereport(DEBUG1, (errmsg("fitsheader2polygon: after astMapRegion (skybox=%p)", skybox)));

	// if skybox == AST__NULL : handle error...
	if (skybox == AST__NULL) {
		ereport(WARNING, (errmsg("pgAST error (fitsheader2polygon): astMapRegion returned NULL. (status=%d", astStatus)));
		astEnd;
		return AST__NULL;
	}

	// Create a mesh: this is an array of points evenly
	// distributed along the boundary of the region.
	// The spacing of the points is determined by the
	// attribute "MeshSize", which is approximately the
	// number of points (default=200 for 2D images).
	// The points are in RA/dec.
	//
	// This call must be made twice:
	//    - call once to find out how many points will be needed for the mesh
	//    - allocate an array of that many double values
	//    - call again with allocated array
	//
	surface = 1;  // non-zero = fit points on  surface (2D -> boundary) of region
	//num_points; // return value (declared above)
	maxcoord = 2; // number of axis values per point (e.g. 2D image = 2)
	maxpoint = 0; // 0 = number of points fits returned in "npoint"
	
	// First call - how many points needed for a mesh?
	//
	astGetRegionMesh(skybox,	// the region
					 surface,	// 1 = fit along boundary
					 maxpoint,	// 0 = how many points needed?
					 maxcoord,	// ignored
					 &num_points,  // returns the number of points needed (no. of (x,y) pairs)
					 NULL);		// ignored
	
	// if skybox == AST__NULL : handle error...
	if (!astOK) {
		ereport(WARNING, (errmsg("pgAST error (fitsheader2polygon): astGetRegionMesh (1) failed. (status=%d)", astStatus)));
		astEnd;
		return AST__NULL;
	}

	// allocate array for points -> [maxcoord][num_points]
	//ereport(DEBUG1, (errmsg("pgast: about to allocate [%d][%d][%d]", maxcoord, num_points, sizeof(double))));
	mesh_points = (double*)palloc(maxcoord*num_points * sizeof(double));
	
	maxpoint = num_points; // length of second dimension of points array
	
	// Second call - get mesh points
	astGetRegionMesh(skybox,	// 
					 surface,	// 1 = fit along boundary
					 maxpoint,	// length of 2nd dimension in points array
					 maxcoord,	// number of axes of region (2D image = 2)
					 &num_points,	// returns the number of points needed (no. of (x,y) pairs)
					 mesh_points); // the array of points
	
	//for (int i=0; i < 2*num_points; i++)
	//	ereport(WARNING, (errmsg("%f", rad2deg(mesh_points[i]))));
	
	// Normalize the points to [0,360); needed if the original frame is a basic frame
	// This is not needed for a sky frame as d = d + 360 in that case, but doesn't hurt.
	for (int i=0; i < num_points; i++) {
		//astNorm(skybox, &mesh_points[2*i]);
		//astNorm(skyBox, {mesh_points[i], mesh_points[2*i]});
	}

	//ereport(WARNING, (errmsg("===")));
	
	//for (int i=0; i < num_points; i++)
	//	ereport(WARNING, (errmsg("%f, %f", rad2deg(mesh_points[i]), rad2deg(mesh_points[2*i]))));
	
	if (!astOK) {
		ereport(WARNING, (errmsg("pgAST error (fitsheader2polygon): astGetRegionMesh (2) failed. (status=%d", astStatus)));
		pfree(mesh_points);
		astEnd;
		return AST__NULL;
	}
	
	// The returned array is in the form:
	// [x1, x2, ..., xn, y1, y2, ..., yn] in RADIANS.
	// (or more specifically, [ra1, ..., ran, dec1, ..., decn]
	
//	ereport(DEBUG1, (errmsg("fitsheader2polygon: after astGetRegionMesh (2) (skybox=%p)", skybox)));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Wformat-zero-length"
	// Create a new polygon from the 2D mesh points
	// in a flat (pixel) frame (i.e. Cartesean).
	//
	basic_frame = astFrame(2, "unit(1)=deg,unit(2)=deg"); // # of axes, options
	flat_polygon = astPolygon(basic_frame,	// frame is copied
							  num_points,	// number of points in region
							  num_points,	// dim - number of elements along 2nd dimension of points array
							  mesh_points,	// 2D array of mesh points
							  AST__NULL,	// uncertainty
							  "");			// options
#pragma GCC diagnostic pop

	//	ereport(DEBUG1, (errmsg("fitsheader2polygon: after create astPolygon (flat_polygon=%p)", flat_polygon)));

	pfree(mesh_points);

	//	ereport(DEBUG1, (errmsg("fitsheader2polygon: after pfree(mesh_points)")));

	// Create a new polygon with downsized points.
	// This removes points where the polygon is close
	// to a Cartesean straight line up to the error specified.
	// "reduced_flat_polygon" is a polygon in a frame with axes in degrees,
	// but is not a sky frame.
	//
	{
		double max_downsize_err = 4.848e-6; // 1 arcsec in radians
		reduced_flat_polygon = astDownsize(flat_polygon, max_downsize_err, 0); // -> AstPolygon
		// ereport(DEBUG1, (errmsg("fitsheader2polygon: after astDownsize (reduced_flat_polygon=%p)", reduced_flat_polygon)));

		// This is not the final polygon; we want it in the sky frame.
	}

	// ---
	
	if (!astGetI(reduced_flat_polygon, "Bounded")) {
		astNegate(reduced_flat_polygon);
	}
	

	// Get the points of this new reduced polygon.
	// -------------------------------------------
	
	// Again, this must be done in three steps:
	//    * find out how many points there are
	//    * allocate the points array
	//    * get the points.
	
	astGetRegionPoints(reduced_flat_polygon,	// the region
					   0,						// maxpoint = 0, just return the point count
					   maxcoord,				// ignored here
					   &num_points,				// number of points needed
					   NULL);					// NULL = just return number of points needed
					   
	//	ereport(DEBUG1, (errmsg("fitsheader2polygon: after astGetRegionPoints (3)")));

	// values to return
	*npoints = num_points;
	
	//astExport(reduced_flat_polygon);
	//astEnd;
	//return reduced_flat_polygon;
		
	// allocate points array
	points = (double*)palloc(maxcoord*num_points * sizeof(double));

	maxpoint = num_points; // length of second dimension of points array
	//maxcoord = num_points;
	
	// get the points
	astGetRegionPoints(reduced_flat_polygon,	// the region
					   maxpoint,	// length of the second dimension
					   maxcoord,	// number of axes of region (2D image = 2)
					   &num_points,
					   points);		// returned array

	//ereport(DEBUG1, (errmsg("astNorm before: %f %f", points[0], points[1])));

	// normalize the points
	normalize_ast_array(sky_frame, points, num_points);

	//ereport(DEBUG1, (errmsg("astNorm after : %f %f", points[0], points[1])));

	//	ereport(DEBUG1, (errmsg("fitsheader2polygon: after astGetRegionPoints (4)")));

	// print all points (for debugging)
	//for (int i=0; i < num_points; i++) {
	//	printf("deg = (%.9f, %.9f) | rad = (%.9f, %.9f)\n", rad2deg(points[i]), rad2deg(points[i+num_points]), points[i], points[i+num_points]); // (x, y)
	//}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-zero-length"
#pragma GCC diagnostic ignored "-Wformat-security"
	// Finally, create a new polygon in the sky frame with these points.
	//
	sky_frame_polygon = astPolygon(sky_frame,	// frame is copied
								   num_points,	// number of points in region
								   num_points,	// dim - number of elements along 2nd dimension of points array
								   points,		// 2D array of mesh points
								   AST__NULL,	// uncertainty
								   "");			// options
#pragma GCC diagnostic pop

//	ereport(DEBUG1, (errmsg("fitsheader2polygon: after sky_frame_polygon created (sky_frame_polygon=%p)", sky_frame_polygon)));

	//pfree(points);

//	ereport(DEBUG1, (errmsg("fitsheader2polygon: after pfree(points)")));

	// Which side of the boundary is the inside?
	//
	// AST takes the inside of a Polygon to be the region to the LEFT of the
	// boundary as the vertices are traversed in the order supplied, seen from
	// inside the celestial sphere. The order of the mesh points above depends
	// on the WCS (e.g. on the pixel parity), so the polygon just built may be
	// the image footprint or the rest of the sky. ("Bounded" cannot tell the
	// two apart: in current AST every Region on a SkyFrame is bounded.)
	//
	// Test the centre pixel. If it is outside, rebuild the polygon from the
	// vertices in reverse order, which describes the other region. Reversing
	// the vertices (rather than calling astNegate) keeps the vertex order
	// meaningful on its own, so a polygon stored from pgast_bounding_polygon
	// describes the same region when it is read back.
	{
		const double pixel_x[1] = { (dim1 + 1) / 2.0 }; // FITS pixel centres run from 1 to NAXISn
		const double pixel_y[1] = { (dim2 + 1) / 2.0 };
		double centre_x[1], centre_y[1];
		double centre[2];

		astTran2(wcs_frames,			// mapping: pixel (base) -> sky (current)
				 1,						// number of points
				 pixel_x, pixel_y,		// input (pixel) coordinates
				 PIXEL2WORLD,			// forward transformation
				 centre_x, centre_y);	// output (sky) coordinates

		if (!astOK || centre_x[0] == AST__BAD || centre_y[0] == AST__BAD) {
			astEnd;
			ereport(ERROR, (errmsg("pgAST (fitsheader2polygon): the WCS does not map the centre pixel (%.1f, %.1f) to the sky.",
								   pixel_x[0], pixel_y[0])));
		}
		centre[0] = centre_x[0];
		centre[1] = centre_y[0];

		if (!astPointInRegion(sky_frame_polygon, centre)) {
			// structure of points array is: [ra1, ra2, ..., ran, dec1, dec2, ..., decn]
			double *reversed_points = (double*)palloc(maxcoord * num_points * sizeof(double));
			for (int i=0; i < num_points; i++) {
				reversed_points[i] = points[num_points - 1 - i];
				reversed_points[num_points + i] = points[2*num_points - 1 - i];
			}

			sky_frame_polygon = astAnnul(sky_frame_polygon);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-zero-length"
#pragma GCC diagnostic ignored "-Wformat-security"
			sky_frame_polygon = astPolygon(sky_frame,
										   num_points,
										   num_points,
										   reversed_points,
										   AST__NULL,
										   "");
#pragma GCC diagnostic pop
			pfree(reversed_points);

			if (!astOK || !astPointInRegion(sky_frame_polygon, centre)) {
				astEnd;
				ereport(ERROR, (errmsg("pgAST (fitsheader2polygon): neither vertex order gives a polygon that contains "
									   "the centre pixel; the WCS is not well behaved over the image (AST status=%d).", astStatus)));
			}
		}
	}
		
//	ereport(DEBUG1, (errmsg("fitsheader2polygon: after bounded check (num_points=%d)", num_points)));

	// values to return
	*npoints = num_points;

//	ereport(DEBUG1, (errmsg("fitsheader2polygon: before astExport")));

	astExport(sky_frame_polygon);
	
	astEnd;

	pfree(points);

//	ereport(DEBUG1, (errmsg("fitsheader2polygon: before return")));

	return sky_frame_polygon;
}

/*
 The function astNorm is used to normalize ra,dec points on [0,360) and [-90,90],
 respectively (except in radians).
 Unfortunately, it only works on one point at a time. This is a convenience function
 to take arrays of points like those returned by astGetRegionPoints and astDownsize
 and normalizes the whole array.
 
 Note that these arrays are arranged by [ra1, ra2, ..., ran, dec1, dec2, ..., decn]
 */
void normalize_ast_array(AstFrame *frame, double *points, int npairs) // npairs = number of coordinate pairs
{
	double coordinate[2];
		
	// signature: void astNorm( AstFrame ∗this, double value[] )
	
	for (int i=0; i < npairs; i++) {
		
		coordinate[0] = points[i];
		coordinate[1] = points[npairs+i];
		
		astNorm(frame, coordinate);
		
		points[i] = coordinate[0];
		points[npairs+i] = coordinate[1];
	}
}

