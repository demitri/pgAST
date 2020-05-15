
#ifndef __PGAST2_H__
#define __PGAST2_H__

/* include PostgreSQL library and PG macro definitions */
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/array.h"
#include "utils/geo_decls.h" // POLYGON support

#include <math.h>

#define deg2rad(angleDegrees) ((angleDegrees) * M_PI / 180.0)
#define rad2deg(angleRadians) ((angleRadians) * 180.0 / M_PI)

#include "pgast_macros.h"

#include "ast.h"

#endif /* __PGAST2_H__ */
