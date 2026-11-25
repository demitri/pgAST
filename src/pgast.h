
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

/*
 Raise a PostgreSQL ERROR for a failure inside AST code.

 AST's error status is global to the backend. ereport(ERROR) longjmps out of
 the function, so if the status is left set, every AST call made later in the
 same session does nothing and the functions return NULL or garbage. This
 records the status, clears it, then raises. Call astEnd for the current AST
 context first (code after ereport(ERROR) never runs).
*/
#define pgast_ast_error(...) \
	do { \
		int pgast_ast_status_ = astStatus; \
		astClearStatus; \
		ereport(ERROR, (errmsg(__VA_ARGS__), errdetail("AST status: %d", pgast_ast_status_))); \
	} while (0)

/*
 End the AST context of a SQL function's success path. If any AST call in
 the function failed (AST calls do nothing once the status is set, so the
 result would be garbage), raise an error instead of returning.
*/
#define pgast_ast_end_checked() \
	do { \
		if (!astOK) { \
			astEnd; \
			pgast_ast_error("pgAST (%s): an AST call failed.", __func__); \
		} \
		astEnd; \
	} while (0)

#endif /* __PGAST2_H__ */
