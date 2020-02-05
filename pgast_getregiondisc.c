
#include "pgast.h"

PG_FUNCTION_INFO_V1(pgast_getregiondisc);
Datum
pgast_getregiondisc(PG_FUNCTION_ARGS)
{
	// get function args
	char* fits_header;

	// create a null-terminated C string from a PostgreSQL text value
	// (pg function from utils/builtins.h)
	fits_header = text_to_cstring(PG_GETARG_TEXT_P(0));
	
	astBegin;
	
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Wformat-zero-length"
	AstFitsChan *fitschan = astFitsChan( NULL, NULL, ""); /* create an empty channel */
#pragma GCC diagnostic pop
	
	astPutCards( fitschan, fits_header ); /* add all cards at once */

	AstFrame *wcsinfo = astRead( fitschan );
	
	int32_t is_sky_frame = astIsASkyFrame(wcsinfo);
	
	PG_RETURN_INT32(is_sky_frame);
	
	astEnd;
}
