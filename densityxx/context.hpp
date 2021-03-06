// see LICENSE.md for license.
#pragma once

#include "densityxx/format.hpp"
#include "densityxx/memory.hpp"

namespace density {
    class context_t {
        static const size_t memory_teleport_buffer_size = 1 << 16;
        uint_fast64_t total_read, total_written;
        uint_fast64_t available_in_before, available_out_before;
    public:
#if DENSITY_WRITE_MAIN_FOOTER == DENSITY_YES && DENSITY_ENABLE_PARALLELIZABLE_DECOMPRESSIBLE_OUTPUT == DENSITY_YES
        static const size_t end_data_overhead = sizeof(main_footer_t);
#else
        static const size_t end_data_overhead = 0;
#endif
        teleport_t in;
        location_t out;
        main_header_t header;
        main_footer_t footer;

        DENSITY_INLINE context_t(void): in(memory_teleport_buffer_size), out() {}

        DENSITY_INLINE const uint_fast64_t get_total_read(void) const { return total_read; }
        DENSITY_INLINE const uint_fast64_t get_total_written(void) const { return total_written; }

        DENSITY_INLINE void update_input(const uint8_t *in, const uint_fast64_t szin)
        {   this->in.change_input_buffer(in, szin); }
        DENSITY_INLINE void update_output(uint8_t *out, const uint_fast64_t szout)
        {   this->out.encapsulate(out, szout); }
        DENSITY_INLINE uint_fast64_t output_available_for_use(void) const { return out.used(); }

        DENSITY_INLINE void init(const compression_mode_t compression_mode,
                                 const block_type_t block_type,
                                 const uint8_t *in, const uint_fast64_t available_in,
                                 uint8_t *out, const uint_fast64_t available_out)
        {   header.setup(compression_mode, block_type);
            total_read = total_written = 0;
            this->in.reset_staging_buffer();
            update_input(in, available_in);
            update_output(out, available_out); }

        DENSITY_INLINE context_t &before(void)
        {   available_in_before = in.available_bytes();
            available_out_before = out.available_bytes;
            return *this; }
        DENSITY_INLINE encode_state_t after(encode_state_t state)
        {   total_read += available_in_before - in.available_bytes();
            total_written += available_out_before - out.available_bytes;
            return state; }
        DENSITY_INLINE decode_state_t after(decode_state_t state)
        {   total_read += available_in_before - in.available_bytes_reserved(end_data_overhead);
            total_written += available_out_before - out.available_bytes;
            return state; }

        DENSITY_INLINE encode_state_t write_header(void)
        {   if (sizeof(header) > out.available_bytes) return encode_state_stall_on_output;
            out.write(&header, sizeof(header));
            total_written += sizeof(header);
            return encode_state_ready; }
        DENSITY_INLINE encode_state_t write_footer(uint32_t relative_position)
        {   if (end_data_overhead == 0) return encode_state_ready;
            footer.relative_position = relative_position;
            if (sizeof(footer) > out.available_bytes) return encode_state_stall_on_output;
            out.write(&footer, sizeof(footer));
            total_written += sizeof(footer);
            return encode_state_ready; }

        DENSITY_INLINE decode_state_t read_header(void)
        {   location_t *read_location = in.read_reserved(sizeof(header), end_data_overhead);
            if (read_location == NULL) return decode_state_stall_on_input;
            read_location->read(&header, sizeof(header));
            total_read += sizeof(header);
            return decode_state_ready; }
        DENSITY_INLINE decode_state_t read_footer(void)
        {   if (end_data_overhead == 0) return decode_state_ready;
            location_t *read_location = in.read_reserved(sizeof(footer), end_data_overhead);
            if (read_location == NULL) return decode_state_stall_on_input;
            read_location->read(&footer, sizeof(footer));
            total_read += sizeof(footer);
            return decode_state_ready; }
    };
}
