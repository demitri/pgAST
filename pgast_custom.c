//
//  pgast_custom.c
//  ast_sandboxes
//
//  Created by Demitri Muna on 2/2/20.
//  Copyright © 2020 Demitri Muna. All rights reserved.
//

#include <stdlib.h>
#include "pgast_custom.h"

AstPolygon* pgast_polygon_from_fitschan(AstFitsChan *fitsChan) //, AstPolygon **new_polygon)
{
	astBegin;
	
	int success;
	//new_polygon = AST__NULL;

	// copy the provided fitsChan to avoid modifing the provided one
	AstFitsChan *fitsChanCopy = astCopy(fitsChan);
	
	// read the WCS from the header
	AstFrameSet *wcsinfo = astRead( fitsChanCopy );

	// check if the fitsChan contains a 2D image
	// TODO: what other checks should be performed - if any? (e.g. leave to fail later?)
	int naxes;
	success = astGetFitsI(fitsChanCopy, "NAXIS", &naxes);
	if (success == 0 || naxes != 2) {
		astEnd;
		return 0;
	}
	  
	// get the image dimensions
	int dim1, dim2;
	success = astGetFitsI(fitsChanCopy, "NAXIS1", &dim1);
	if (success == 0) {
		astEnd;
		return 0;
	}
	success = astGetFitsI(fitsChanCopy, "NAXIS2", &dim2);
	if (success == 0) {
		astEnd;
		return 0;
	}

	const double point1[] = {0.5, 0.5};
	const double point2[] = {dim1+0.5, dim2+0.5};
	const char *options = "";

	AstBox *pixelbox = astBox(astGetFrame(wcsinfo, AST__BASE), 1, point1, point2, AST__NULL, options);
	
	AstFrame *skyFrame = astGetFrame(wcsinfo, AST__CURRENT);
	if (astIsASkyFrame(skyFrame) == 0) {
		astEnd;
		return 0;
	}
	
	// using the wcs mapping, convert the box from the pixel frame to a sky frame
	AstRegion *skybox = astMapRegion(pixelbox, wcsinfo, skyFrame);

	//printf("%s", astToString(skybox));
	
	int surface = 1;  // non-0 = fit points on surface (2D -> boundary) of region
	int npoints;	  // returned value
	int maxcoord = 2; // number of axis values per point (e.g. 2D image = 2)
	int maxpoint = 0; // 0 = number of points fits returned in "npoint"

	// First call - how many points needed for a mesh?
	//
	astGetRegionMesh(skybox,	// the region
					 surface,	// 1 = fit along boundary
					 maxpoint,	// 0 = tell me how many points needed
					 maxcoord,	// ignored
					 &npoints,	// returns the number of points needed
					 NULL);		// ignored

	// allocate points array of dimension: [maxcoord][npoints]
	double *mesh_points = (double*)malloc(maxcoord*npoints * sizeof(double));

	maxpoint = npoints; // length of second dimension of points array

	// Second call: astGetRegionMesh again, this time getting actual points array.
	//
	astGetRegionMesh(skybox,	// the region
					surface,	// 1 = fit along boundary
					maxpoint,	// length of the second dimension of points array
					maxcoord,	// number of axes of region (2D image = 2)
					&npoints,	// returns the number of points needed (no. of (x,y) pairs)
					mesh_points);	// the array of points

	// Create a new polygon from the 2D mesh points
	//
	AstFrame *basic_frame = astFrame(2, "unit(1)=deg,unit(2)=deg"); // # of axes, options
	AstPolygon *flat_polygon = astPolygon(basic_frame, //astGetFrame(wcsinfo, AST__BASE), // frame is copied
										  npoints,	// number of points in region
										  npoints, // dim - number of elements along the second dimension of the "points" array
										  mesh_points,	// 2D array of mesh points
										  AST__NULL, // uncertainty
										  "");		// options

	free(mesh_points);

	// Create new polygon with downsized points. Frame unit is in degrees.
	//
	AstPolygon *new_polygon = astDownsize(flat_polygon, 4.848e-6 /* maxerr */, 0 /* maxvert */);

	astExport(new_polygon);
	astEnd;

	return new_polygon;
}
