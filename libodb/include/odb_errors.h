#ifndef ODB_ERRORS_H
#define ODB_ERRORS_H

typedef enum odbErrorCode {
	ODB_ERR_NONE = 0,
	ODB_ERR_NOT_IMPLEMENTED = 1,
	ODB_ERR_INVALID_ARG = 2,
	ODB_ERR_IO = 3,
	ODB_ERR_INTERNAL = 4
} odbErrorCode;

#endif
