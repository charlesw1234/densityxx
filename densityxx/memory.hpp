// see LICENSE.md for license.
#pragma once

#include "globals.hpp"

namespace density {
    class location_t {
    public:
        uint8_t *pointer;
        uint_fast64_t available_bytes;
        uint_fast64_t initial_available_bytes;

        inline void consume(uint_fast64_t sz)
        {   pointer += sz; available_bytes -= sz; }
        inline void read(void *data, uint_fast64_t szdata)
        {   DENSITY_MEMCPY(data, pointer, szdata); consume(szdata); }
        inline void write(const void *data, uint_fast64_t szdata)
        {   DENSITY_MEMCPY(pointer, data, szdata); consume(szdata); }

        inline void encapsulate(uint8_t *RESTRICT pointer, const uint_fast64_t bytes) {
            this->pointer = pointer;
            available_bytes = bytes;
            initial_available_bytes = bytes;
        }
        inline uint_fast64_t used(void) const
        {   return initial_available_bytes - available_bytes; }
    };
    class teleport_t {
    public:
        uint8_t *original_pointer;
        uint8_t *write_pointer;
        location_t staging;
        location_t direct;

        inline teleport_t(const uint_fast64_t size)
        {
            staging.pointer = (uint8_t *)malloc(size);
            staging.available_bytes = 0;
            write_pointer = original_pointer = staging.pointer;
            direct.available_bytes = 0;
        }
        inline ~teleport_t()
        {
            free(staging.original_pointer);
        }
    private:
        inline void rewind_staging_pointers(void)
        {   staging.pointer = write_pointer = original_pointer; }
    public:
        inline void
        change_input_buffer(const uint8_t *RESTRICT in, const uint_fast64_t availableIn)
        {   direct.encapsulate((uint8_t *)in, availableIn); }

        inline void copy_from_direct_buffer_to_staging_buffer(void)
        {   DENSITY_MEMCPY(write_pointer, direct.pointer, direct.available_bytes);
            write_pointer += direct.available_bytes;
            staging.available_bytes += direct.available_bytes;
            direct.pointer += direct.available_bytes;
            direct.available_bytes = 0; }

        inline void reset_staging_buffer(void)
        {   rewind_staging_pointers(); staging.available_bytes = 0; }

        inline location_t *read(const uint_fast64_t bytes)
        {   uint_fast64_t addonBytes;
            if (DENSITY_UNLIKELY(staging.available_bytes)) {
                if (staging.available_bytes >= bytes) {
                    return &staging;
                } else if (DENSITY_UNLIKELY((addonBytes = (bytes - staging.available_bytes)) <= direct.available_bytes)) {
                    if (staging.available_bytes <= direct.initial_available_bytes - direct.available_bytes) { // Revert to direct buffer reading
                        reset_staging_buffer();
                        direct.pointer -= staging.available_bytes;
                        direct.available_bytes += staging.available_bytes;
                        return &direct;
                    } else { // Copy missing bytes from direct input buffer
                        DENSITY_MEMCPY(write_pointer, direct.pointer, addonBytes);
                        write_pointer += addonBytes;
                        staging.available_bytes += addonBytes;
                        direct.pointer += addonBytes;
                        direct.available_bytes -= addonBytes;
                        return &staging;
                    }
                } else { // Copy as mush as we can from direct input buffer
                    copy_from_direct_buffer_to_staging_buffer();
                    return NULL;
                }
            } else {
                if (DENSITY_LIKELY(direct.available_bytes >= bytes)) {
                    return &direct;
                } else { // Copy what we have in our staging buffer
                    rewind_staging_pointers();
                    copy_from_direct_buffer_to_staging_buffer();
                    return NULL;
                }
            }
        }

        inline location_t *
        read_reserved(const uint_fast64_t bytes, const uint_fast64_t reserved)
        {   return read(bytes + reserved); }

        inline location_t *
        read_remaining_reserved(const uint_fast64_t reserved)
        {   return read_reserved(available_bytes_reserved(reserved), reserved); }

        inline uint_fast64_t available_bytes(void) const
        {   return staging.available_bytes + direct.available_bytes; }

        inline uint_fast64_t
        available_bytes_reserved(const uint_fast64_t reserved) const
        {   const uint_fast64_t contained = available_bytes();
            return DENSITY_UNLIKELY(reserved >= contained) ? 0: contained - reserved; }

        inline void
        copy(location_t *RESTRICT out, const uint_fast64_t bytes)
        {   uint_fast64_t fromStaging = 0;
            uint_fast64_t fromDirect = 0;

            if (staging.available_bytes) {
                if (bytes <= staging.available_bytes) {
                    fromStaging = bytes;
                } else if (bytes <= staging.available_bytes + direct.available_bytes) {
                    fromStaging = staging.available_bytes;
                    fromDirect = bytes - staging.available_bytes;
                } else {
                    fromStaging = staging.available_bytes;
                    fromDirect = direct.available_bytes;
                }
            } else {
                if (bytes <= direct.available_bytes) {
                    fromDirect = bytes;
                } else {
                    fromDirect = direct.available_bytes;
                }
            }

            DENSITY_MEMCPY(out->pointer, staging.pointer, fromStaging);
            if (fromStaging == staging.available_bytes) reset_staging_buffer();
            else {
                staging.pointer += fromStaging;
                staging.available_bytes -= fromStaging;
            }
            out->pointer += fromStaging;
            out->available_bytes -= fromStaging;

            DENSITY_MEMCPY(out->pointer, direct.pointer, fromDirect);
            direct.pointer += fromDirect;
            direct.available_bytes -= fromDirect;
            out->pointer += fromDirect;
            out->available_bytes -= fromDirect;
        }
            
        inline void copy_remaining(location_t *out)
        {   return copy(out, available_bytes()); }
    };
}