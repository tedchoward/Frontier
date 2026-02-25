#ifndef ODB_ERROR_INTERNAL_H
#define ODB_ERROR_INTERNAL_H

#include "odb_errors.h"

void odbSetLastError(odbErrorCode code, const char *message);
const char *odbGetLastErrorMessage(void);

#endif
