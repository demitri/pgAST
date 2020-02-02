
#ifndef __PGAST_GETREGIONDISC_H__
#define __PGAST_GETREGIONDISC_H__

/* include PostgreSQL library and PG macro definitions */
#include "postgres.h"
#include "fmgr.h"

AstPolygon* fitsheader2polygon(const char *header);

#endif /* __PGAST_GETREGIONDISC_H__ */
