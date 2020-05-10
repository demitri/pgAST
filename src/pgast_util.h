
#ifndef __PGGAST_UTIL_H__
#define __PGGAST_UTIL_H__

#include "pgast.h"
//#include "ast.h"

/* include PostgreSQL library and PG macro definitions */
//#include "postgres.h"
//#include "utils/geo_decls.h" // POLYGON support

void pgPolygon2astPoints(POLYGON *polygon, double *points);
AstPolygon* pgPolygon2astPolygon(POLYGON *polygon, AstSkyFrame *frame);

//void pgpolygon2astPolygon(POLYGON *polygon, double *points, AstFrame *frame);
//void pgarray_to_double_array(ArrayType *array, double *data, int *nelements);

#endif /* __PGGAST_UTIL_H__ */
