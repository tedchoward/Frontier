#ifndef ODB_BYTEORDER_H
#define ODB_BYTEORDER_H

#include <stdint.h>

#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define SWAP_BYTE_ORDER 1
#else
#undef SWAP_BYTE_ORDER
#endif

static inline int16_t odb_swap16(int16_t v) {
	uint16_t x = (uint16_t)v;
	return (int16_t)((x >> 8) | (x << 8));
}

static inline int32_t odb_swap32(int32_t v) {
	uint32_t x = (uint32_t)v;
	return (int32_t)((x >> 24)
		| ((x & 0x00FF0000) >> 8)
		| ((x & 0x0000FF00) << 8)
		| (x << 24));
}

#ifdef SWAP_BYTE_ORDER
#define disktomemshort(x) do { (x) = odb_swap16((x)); } while (0)
#define disktomemlong(x) do { (x) = odb_swap32((x)); } while (0)
#define memtodiskshort(x) do { (x) = odb_swap16((x)); } while (0)
#define memtodisklong(x) do { (x) = odb_swap32((x)); } while (0)
#else
#define disktomemshort(x) do { } while (0)
#define disktomemlong(x) do { } while (0)
#define memtodiskshort(x) do { } while (0)
#define memtodisklong(x) do { } while (0)
#endif

#endif
