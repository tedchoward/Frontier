#ifndef OBJECTDB_VALUE_FORMAT_H
#define OBJECTDB_VALUE_FORMAT_H

#include "odb_engine.h"

char *objectdb_value_format_kind(odbValueType type);
char *objectdb_value_format_value(odbValueType type, const odbValueRecord *value);

#endif
