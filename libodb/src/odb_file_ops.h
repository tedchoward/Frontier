#ifndef ODB_FILE_OPS_H
#define ODB_FILE_OPS_H

#include "odb_compat.h"

boolean filegeteof(hdlfilenum fnum, long *eof);
boolean fileseteof(hdlfilenum fnum, long eof);
boolean filesetposition(hdlfilenum fnum, long position);
boolean filewrite(hdlfilenum fnum, long ctbytes, void *pdata);
boolean fileread(hdlfilenum fnum, long ctbytes, void *pdata);
boolean fileflush(hdlfilenum fnum);

#endif
