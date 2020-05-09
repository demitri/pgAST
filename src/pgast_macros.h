
#ifndef __PGAST_MACROS_H__
#define __PGAST_MACROS_H__

// -------------------------------------------------------------------------
// This is taken from the intarray contrib header intarray/_int.h.
// $(PG_SRC)/contrib/intarray
// -------------------------------------------------------------------------
/* reject arrays we can't handle; to wit, those containing nulls */
#define CHECKARRVALID(x) \
    do { \
        if (ARR_HASNULL(x) && array_contains_nulls(x)) \
            ereport(ERROR, \
                    (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED), \
                     errmsg("array must not contain nulls"))); \
    } while(0)

#define ARRISEMPTY(x)  (ARRNELEMS(x) == 0)
// -------------------------------------------------------------------------

/* useful macros for accessing arrays */
#define ARRPTR_FLOAT4(x)  ( (float4 *) ARR_DATA_PTR(x) )
#define ARRPTR_DOUBLE(x)  ( (double *) ARR_DATA_PTR(x) )
#define ARRNELEMS(x)  ArrayGetNItems(ARR_NDIM(x), ARR_DIMS(x))


#endif // __PGAST_MACROS_H__
