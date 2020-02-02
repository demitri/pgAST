
//#include <math.h>

#include "ast.h" // Starlink AST library header
//#include "postgres.h"
#include "pgast_pgast.h"
#include "pgast_header2polygon.h"

//#define deg2rad(angleDegrees) ((angleDegrees) * M_PI / 180.0)
//#define rad2deg(angleRadians) ((angleRadians) * 180.0 / M_PI)


AstPolygon* fitsheader2polygon(const char *header) //, double *polygon, int *npoints)
{
	//int npoints;
	
	// Inputs
	// ------
	// header  : FITS header as single string
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
	
	int dim1, dim2;
	int surface, num_points, maxcoord, maxpoint;
	AstFitsChan *fitsChan;
	AstBox *pixelbox, *skybox;
	AstFrameSet *wcs_frames;
	AstFrame *pixel_frame, *sky_frame, *basic_frame;
	AstPolygon *flat_polygon, *reduced_flat_polygon, *new_sky_polygon;
	double *points, *mesh_points;
	
	astBegin;
	
	// Create an empty FITS channel (AST object that holds a FITS header)
#pragma GCC diagnostic ignored "-Wformat-zero-length"
	fitsChan = astFitsChan( NULL, NULL, "");
	astPutCards( fitsChan, header ); // add all cards at once
#pragma GCC diagnostic warning "-Wformat-zero-length"
	
	// Read the WCS info from the header
	wcs_frames = astRead(fitsChan);

	//ereport(DEBUG1,(errmsg("pgast: Header:")));
	//ereport(DEBUG1,(errmsg("%s", header)));
	
	// The resulting object contains two frames:
	// the pixel frame and the "sky" frame.
	pixel_frame = astGetFrame(wcs_frames, AST__BASE);
	sky_frame = astGetFrame(wcs_frames, AST__CURRENT);
	
	// get the image dimensions, read from header
	{
		int success;
		success = astGetFitsI(fitsChan, "NAXIS1", &dim1);
		if (success != 1) {
			ereport(DEBUG1, (errmsg("Error: astGetFits did not find 'NAXIS1' keyword.")));
		}
		success = astGetFitsI(fitsChan, "NAXIS2", &dim2);
		if (success != 1) {
			ereport(DEBUG1, (errmsg("Error: astGetFits did not find 'NAXIS2' keyword.")));
		}
	}
	// Create a "box" object that covers the extent of the
	// image in pixel coordinates.
	//
	{
		const double point1[] = {0.5, 0.5};
		const double point2[] = {dim1+0.5, dim2+0.5};
#pragma GCC diagnostic ignored "-Wformat-zero-length"
		pixelbox = astBox(pixel_frame,
						  1,
						  point1,
						  point2,
						  AST__NULL,
						  ""); // no options needed
#pragma GCC diagnostic warning "-Wformat-zero-length"
	}							  
	// Map the box in the pixel frame onto the sky frame.
	// The function knows to go from pixels-> sky
	// since that is the frame 'pixelbox' is in.
	// (Note that a frame set can also act as a mapping.)
	skybox = astMapRegion(pixelbox,
						  wcs_frames,   // map
						  wcs_frames);  // frame

	// if skybox == AST__NULL : handle error...

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
	//num_points; // defined as parameter
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
	
	// The returned array is in the form:
	// [x1, x2, ..., xn, y1, y2, ..., yn] in RADIANS.
	// (or more specifically, [ra1, ..., ran, dec1, ..., decn]
	
#pragma GCC diagnostic ignored "-Wformat-zero-length"
	// Create a new polygon from the 2D mesh points
	// in a flat (pixel) frame (i.e. Cartesean).
	//
	basic_frame = astFrame(2, "unit(1)=deg,unit(2)=deg"); // # of axes, options
	flat_polygon = astPolygon(basic_frame,
							  num_points,	// number of points in region
							  num_points,	// dim - number of elements along 2nd dimension of points array
							  mesh_points,	// 2D array of mesh points
							  AST__NULL,	// uncertainty
							  "");			// options
#pragma GCC diagnostic warning "-Wformat-zero-length"

	pfree(mesh_points);
	
	// Create a new polygon with downsized points.
	// This removes points where the polygon is close
	// to a Cartesean straight line up to the error specified.
	//
	{
		double max_downsize_err = 4.848e-6; // 1 arcsec in radians
		reduced_flat_polygon = astDownsize(flat_polygon, max_downsize_err, 0);
	}

	// Get the points of this new reduced polygon.
	// Again, this must be done in three steps: find out
	// how many points there are, allocate the points array,
	// then get the points.
	
	astGetRegionPoints(reduced_flat_polygon,	// the region
					   0,						// maxpoint = 0, just return the point count
					   maxcoord,				// ignored here
					   &num_points,				// number of points needed
					   NULL);
					   
	// allocate points array
	points = (double*)palloc(maxcoord*num_points * sizeof(double));

	maxpoint = num_points; // length of second dimension of points array
	maxcoord = num_points;
	
	astGetRegionPoints(reduced_flat_polygon,	// the region
					   maxpoint,
					   maxcoord,
					   &num_points,
					   points);		// returned array

	// print all points (for debugging)
	//for (int i=0; i < num_points; i++) {
	//	printf("deg = (%.9f, %.9f) | rad = (%.9f, %.9f)\n", rad2deg(points[i]), rad2deg(points[i+num_points]), points[i], points[i+num_points]); // (x, y)
	//}

#pragma GCC diagnostic ignored "-Wformat-zero-length"

	// Finally, create a new polygon in the sky frame with these points.
	//
	new_sky_polygon = astPolygon(sky_frame,
								 num_points,
								 num_points,
								 points,
								 AST__NULL,
								 "");
#pragma GCC diagnostic warning "-Wformat-zero-length"

	pfree(points);
	
	if (!astGetI(new_sky_polygon, "Bounded"))
		astNegate(new_sky_polygon);
		
	// values to return
	//*npoints = num_points;
	//polygon = new_sky_polygon;
	
	return new_sky_polygon;
	
	astEnd;
}
