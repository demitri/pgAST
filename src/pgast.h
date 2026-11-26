
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

// pgAST requires AST 9.5.0 or later (the version it is tested with).
#if AST__VMAJOR < 9 || (AST__VMAJOR == 9 && AST__VMINOR < 5)
#error "pgAST requires Starlink AST 9.5.0 or later; set AST= to such an installation."
#endif

/*
 AST contexts (astBegin/astEnd) are counted, so that contexts left open by
 an error can be closed. ereport(ERROR) longjmps past every astEnd between
 the error and the SQL function's entry, leaving those contexts, and the AST
 objects they own, open for the life of the backend. pgast_ast_enter(), used
 at the start of every SQL function, first closes any contexts an earlier
 call left open (pgast_ast_error closes them itself; this covers other
 errors, e.g. from PostgreSQL) and clears any error status, then begins this
 call's context. Never call astBegin/astEnd directly (scripts/run_tests.sh checks).
*/
extern int pgast_ast_depth;
#define PGAST_AST_MESSAGES_SIZE 2048
extern char pgast_ast_messages[PGAST_AST_MESSAGES_SIZE];

#define pgast_ast_begin() \
	do { astBegin; pgast_ast_depth++; } while (0)

#define pgast_ast_end() \
	do { astEnd; pgast_ast_depth--; } while (0)

// For SQL entry points only: it closes every open context, so it must never
// run inside code called from another pgAST function (use pgast_ast_begin).
#define pgast_ast_enter() \
	do { \
		astClearStatus; \
		while (pgast_ast_depth > 0) pgast_ast_end(); \
		pgast_ast_messages[0] = '\0'; \
		pgast_ast_begin(); \
	} while (0)


/*
 SQLSTATE codes for pgAST errors (PostgreSQL's standard codes, so callers
 can catch them by name, e.g. EXCEPTION WHEN invalid_parameter_value):

 PGAST_BAD_INPUT    22023 invalid_parameter_value: the input cannot be used
                    (an unreadable or unsuitable FITS header or WCS, a
                    position or box beyond where the WCS is defined, a
                    polygon AST cannot build, a region with no bounding
                    circle).
 PGAST_AST_FAILURE  XX000 internal_error: AST failed on input that should
                    have worked.
*/
#define PGAST_BAD_INPUT   ERRCODE_INVALID_PARAMETER_VALUE
#define PGAST_AST_FAILURE ERRCODE_INTERNAL_ERROR

/*
 AST's own error messages, captured for the error's DETAIL. pgast_ast_puterr
 is registered with astSetPutErr in _PG_init (pgast_pgast.c), so AST delivers
 its messages here instead of to the server's stderr. pgast_ast_enter clears
 the buffer at the start of each call.
*/
void pgast_ast_puterr(int status, const char *message);

/*
 Raise a PostgreSQL ERROR, with SQLSTATE `code` (PGAST_BAD_INPUT or
 PGAST_AST_FAILURE), for a failure in or around AST code.

 AST's error status is global to the backend, and ereport(ERROR) longjmps
 past every astEnd between here and the SQL entry point. So this records the
 status, clears it, closes every open AST context (freeing their objects),
 and then raises. The DETAIL is the message(s) AST reported during this
 call, or else AST's status number if it is set. The message arguments are
 evaluated after the contexts are closed: never pass a string owned by an
 AST object (e.g. from astGetC).
*/
#define pgast_ast_error(code, ...) \
	do { \
		int pgast_ast_status_ = astStatus; \
		astClearStatus; \
		while (pgast_ast_depth > 0) pgast_ast_end(); \
		if (pgast_ast_messages[0] != '\0') \
			ereport(ERROR, (errcode(code), errmsg(__VA_ARGS__), errdetail("%s", pgast_ast_messages))); \
		else if (pgast_ast_status_ != 0) \
			ereport(ERROR, (errcode(code), errmsg(__VA_ARGS__), errdetail("AST status %d.", pgast_ast_status_))); \
		else \
			ereport(ERROR, (errcode(code), errmsg(__VA_ARGS__))); \
	} while (0)

/*
 End the AST context of a SQL function's success path. If any AST call in
 the function failed (AST calls do nothing once the status is set, so the
 result would be garbage), raise an error instead of returning.
*/
#define pgast_ast_end_checked() \
	do { \
		if (!astOK) { \
			pgast_ast_end(); \
			pgast_ast_error(PGAST_AST_FAILURE, "pgAST (%s): an AST call failed.", __func__); \
		} \
		pgast_ast_end(); \
	} while (0)

#endif /* __PGAST2_H__ */
