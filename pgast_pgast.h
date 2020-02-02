
#ifndef __PGAST_H__
#define __PGAST_H__

/* include PostgreSQL library and PG macro definitions */
#include "postgres.h"
#include "fmgr.h"

#include <math.h>

#define deg2rad(angleDegrees) ((angleDegrees) * M_PI / 180.0)
#define rad2deg(angleRadians) ((angleRadians) * 180.0 / M_PI)

//PG_FUNCTION_INFO_V1(wcs_test);
//PG_FUNCTION_INFO_V1(point_in_polygon);

AstPolygon* fitsheader2polygon(const char *header);
//AstRegion* points_to_region(const char* header, double *points);

#endif /* __PGAST_H__ */
