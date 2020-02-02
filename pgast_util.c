
#include "_pgast_pgast.h"

void pgarray_to_double_array(ArrayType *array, double *data, int *nelements)
{
	Oid data_type;	
	int16 data_type_width;
	bool type_passed_by_value;
	char type_alignment_code;
	bool *null_flags;
	Datum *datum_array;	// array contents as PostgreSQL "Datum" objects
	
  	// get the data type of the elements
  	// (see pg_type.h) for all possible values
  	// 
  	// INT2OID    = int16_t  = small int
  	// INT4OID    = int32_t  = integer
  	// INT8OID    = int64_t  = bigint
  	// FLOAT4OID  = float_t  = real (float)
  	// FLOAT8OID  = double_t = double
  	// NUMERICOID = numeric
  	//
  	data_type = ARR_ELEMTYPE(array);
  	
  	// get "data_type_width" (other values not used)
  	get_typlenbyvalalign(data_type,
  						 &data_type_width,
  						 &type_passed_by_value,
  						 &type_alignment_code);
  	
  	// extract elements of array into an array of "Datum" objects
  	deconstruct_array(array,
  					  data_type,
  					  data_type_width,
  					  type_passed_by_value,
  					  type_alignment_code,
  					  &data,
  					  &null_flags,
  					  &nelements);

	data = (double*)palloc(nelements * sizeof(double);
	
  	switch (data_type) {
  	/*
  		case NUMERICOID:
  			for (uint32_t i=0; i < n; i++) {
  				double v = DatumGetFloat8(data[i]);
  				ereport(DEBUG1, (errmsg("d[%d] = %f", i, v)));
  			}
  			break;
*/

  		case INT2OID:
  			for (uint32_t i=0; i < n; i++) {
  				data[i] = (double)DatumGetInt16(data[i]);
  				ereport(DEBUG1, (errmsg("d[%d] = %f", i, v)));
  			}
  			break;

  		case INT4OID:
  			for (uint32_t i=0; i < n; i++) {
  				data[i] = (double)DatumGetInt32(data[i]);
  				ereport(DEBUG1, (errmsg("d[%d] = %f", i, v)));
  			}
  			break;

  		case INT8OID:
  			for (uint32_t i=0; i < n; i++) {
  				data[i] = (double)DatumGetInt64(data[i]);
  				ereport(DEBUG1, (errmsg("d[%d] = %f", i, v)));
  			}
  			break;

  		case FLOAT4OID:
  			for (uint32_t i=0; i < n; i++) {
  				data[i] = (double)DatumGetFloat4(data[i]);
  				ereport(DEBUG1, (errmsg("d[%d] = %f", i, v)));
  			}
  			break;

  		case FLOAT8OID:
  			for (uint32_t i=0; i < n; i++) {
  				data[i] = DatumGetFloat8(data[i]);
  				ereport(DEBUG1, (errmsg("d[%d] = %f", i, v)));
  			}
  			break;
  			
  		default:
			ereport(DEBUG1, (errmsg("Need to handle this datatype.")));
			break;
	}	
}
