
#ifndef __PGAST2_H__
#define __PGAST2_H__

/* include PostgreSQL library and PG macro definitions */
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"

#include <math.h>

#define deg2rad(angleDegrees) ((angleDegrees) * M_PI / 180.0)
#define rad2deg(angleRadians) ((angleRadians) * 180.0 / M_PI)

#include "ast.h"

// Functions
//PG_FUNCTION_INFO_V1(pgast_getregiondisc);

#endif /* __PGAST2_H__ */
