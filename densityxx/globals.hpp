// see LICENSE.md for license.
#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DENSITYXX_MAJOR_VERSION 0
#define DENSITYXX_MINOR_VERSION 12
#define DENSITYXX_REVISION      5

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LITTLE_ENDIAN_64(b)   ((uint64_t)b)
#define LITTLE_ENDIAN_32(b)   ((uint32_t)b)
#define LITTLE_ENDIAN_16(b)   ((uint16_t)b)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#if __GNUC__ * 100 + __GNUC_MINOR__ >= 403
#define LITTLE_ENDIAN_64(b)   __builtin_bswap64(b)
#define LITTLE_ENDIAN_32(b)   __builtin_bswap32(b)
#define LITTLE_ENDIAN_16(b)   __builtin_bswap16(b)
#else
#warning Using bulk byte swap routines. Expect performance issues.
#define LITTLE_ENDIAN_64(b)   ((((b) & 0xFF00000000000000ull) >> 56) | (((b) & 0x00FF000000000000ull) >> 40) | (((b) & 0x0000FF0000000000ull) >> 24) | (((b) & 0x000000FF00000000ull) >> 8) | (((b) & 0x00000000FF000000ull) << 8) | (((b) & 0x0000000000FF0000ull) << 24ull) | (((b) & 0x000000000000FF00ull) << 40) | (((b) & 0x00000000000000FFull) << 56))
#define LITTLE_ENDIAN_32(b)   ((((b) & 0xFF000000) >> 24) | (((b) & 0x00FF0000) >> 8) | (((b) & 0x0000FF00) << 8) | (((b) & 0x000000FF) << 24))
#define LITTLE_ENDIAN_16(b)   ((((b) & 0xFF00) >> 8) | (((b) & 0x00FF) << 8))
#endif
#else
#error Unknow endianness
#endif

#define DENSITY_MEMCPY  __builtin_memcpy
#define DENSITY_MEMMOVE  __builtin_memmove

#define DENSITY_LIKELY(x)  __builtin_expect(!!(x), 1)
#define DENSITY_UNLIKELY(x)  __builtin_expect(!!(x), 0)

#define DENSITY_BITSIZEOF(x) (8 * sizeof(x))

#define DENSITY_DICTIONARY_PREFERRED_RESET_CYCLE_SHIFT    6
#define DENSITY_DICTIONARY_PREFERRED_RESET_CYCLE          (1 << DENSITY_DICTIONARY_PREFERRED_RESET_CYCLE_SHIFT)

#define RESTRICT

namespace density {
    typedef enum {
        compression_mode_copy = 0,
        compression_mode_chameleon_algorithm = 1,
        compression_mode_cheetah_algorithm = 2,
        compression_mode_lion_algorithm = 3,
    } compression_mode_t;

    typedef enum {
        block_type_default = 0,                      // Standard, no integrity check
        block_type_with_hashsum_integrity_check = 1  // Add data integrity check to the stream
    } block_type_t;
}