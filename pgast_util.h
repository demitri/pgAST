
#ifndef __PGGAST_UTIL_H__
#define __PGGAST_UTIL_H__

void pgpolygon2astPoints(POLYGON *polygon, double *points);
void pgarray_to_double_array(ArrayType *array, double *data, int *nelements);

#endif /* __PGGAST_UTIL_H__ */
