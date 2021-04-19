
#ifndef __PGAST_HEADER2POLYGON_H__
#define __PGAST_HEADER2POLYGON_H__

AstPolygon* fitsheader2polygon(const char *header, int *npoints); //, double *polygon, int *npoints)

// TODO: move this function to a more central location
void normalize_ast_array(AstFrame *frame, double *points, int npairs); // npairs = number of coo    rdinate pairs

#endif /* __PGAST_HEADER2POLYGON_H__ */

