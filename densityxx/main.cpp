#include "densityxx/main.hpp"

namespace density {
    // encode.
    encode_state_t
    encode_t::init(location_t *RESTRICT out, const compression_mode_t mode,
                   const block_type_t block_type)
    {
        encode_state_t encode_state;
        compression_mode = mode;
        this->block_type = block_type;
        total_read = total_written = 0;
#if DENSITY_WRITE_MAIN_HEADER == DENSITY_YES
        if ((encode_state = write_header(out, mode, block_type)))
            return exit_process(encode_process_write_header, encode_state);
#endif
        switch (mode) {
        case compression_mode_copy:
            block_encode.init(NULL, block_type);
            break;
#if 0 // FIXME.
        case compression_mode_chameleon_algorithm:
            block_encode.init(new chameleon_block_encode_t(), block_type);
            break;
        case compression_mode_cheetah_algorithm:
            block_encode.init(new cheetah_block_encode_t(), block_type);
            break;
        case compression_mode_lion_algorithm:
            block_encode.init(new lion_block_encode_t(), block_type);
            break;
#endif
        default: break;
        }
        return exit_process(encode_process_write_blocks, encode_state_ready);
    }

    encode_state_t
    encode_t::continue_(teleport_t *RESTRICT in, location_t *RESTRICT out)
    {
        block_encode_state_t block_encode_state;
        uint_fast64_t available_in_before, available_out_before;
        // Dispatch
        switch (process) {
        case encode_process_write_blocks: goto write_blocks;
        default: return encode_state_error;
        }
    write_blocks:
        available_in_before = in->available_bytes();
        available_out_before = out->available_bytes;
        block_encode_state = block_encode.continue_(in, out);
        update_totals(in, out, available_in_before, available_out_before);
        switch (block_encode_state) {
        case block_encode_state_ready: break;
        case block_encode_state_stall_on_input:
            return exit_process(encode_process_write_blocks, encode_state_stall_on_input);
        case block_encode_state_stall_on_output:
            return exit_process(encode_process_write_blocks, encode_state_stall_on_output);
        case block_encode_state_error: return encode_state_error;
        }
        goto write_blocks;
    }

    encode_state_t
    encode_t::finish(teleport_t *RESTRICT in, location_t *RESTRICT out)
    {
        encode_state_t encode_state;
        block_encode_state_t block_encode_state;
        uint_fast64_t available_in_before, available_out_before;
        // Dispatch
        switch (process) {
        case encode_process_write_blocks:  goto write_blocks;
        case encode_process_write_footer:  goto write_footer;
        default:  return encode_state_error;
        }
    write_blocks:
        available_in_before = in->available_bytes();
        available_out_before = out->available_bytes;
        block_encode_state = block_encode.finish(in, out);
        update_totals(in, out, available_in_before, available_out_before);
        switch (block_encode_state) {
        case block_encode_state_ready: break;
        case block_encode_state_stall_on_output:
            return exit_process(encode_process_write_blocks, encode_state_stall_on_output);
        default:  return encode_state_error;
        }
    write_footer:
#if DENSITY_WRITE_MAIN_FOOTER == DENSITY_YES && DENSITY_ENABLE_PARALLELIZABLE_DECOMPRESSIBLE_OUTPUT == DENSITY_YES
        if ((encode_state = write_footer(out)))
            return exit_process(encode_process_write_footer, encode_state);
#endif
        return encode_state_ready;
    }

    encode_state_t
    encode_t::write_header(location_t *RESTRICT out, const compression_mode_t compression_mode,
                           const block_type_t block_type)
    {
        main_header_t main_header;
        if (sizeof(main_header_t) > out->available_bytes)
            return encode_state_stall_on_output;
        main_header_parameters_t parameters;
        memset(&parameters, 0, sizeof(parameters));
#if DENSITY_ENABLE_PARALLELIZABLE_DECOMPRESSIBLE_OUTPUT == DENSITY_YES
        parameters.as_bytes[0] = DENSITY_DICTIONARY_PREFERRED_RESET_CYCLE_SHIFT;
#endif
        total_written += main_header.write(out, compression_mode, block_type, parameters);
        return encode_state_ready;
    }

    encode_state_t
    encode_t::write_footer(location_t *RESTRICT out)
    {
        main_footer_t main_footer;
        if (sizeof(main_footer_t) > out->available_bytes)
            return encode_state_stall_on_output;
        if (block_encode.total_read > 0) {
            uint32_t sz = block_encode.total_read - block_encode.in_start;
            total_written += main_footer.write(out, sz);
        }
        return encode_state_ready;
    }

    // decode.
    decode_state_t
    decode_t::init(teleport_t *in)
    {
        decode_state_t decode_state;
        total_read = total_written = 0;
#if DENSITY_WRITE_MAIN_HEADER == DENSITY_YES
        if ((decode_state = read_header(in)))
            return exit_process(decode_process_read_header, decode_state);
#endif
        switch (header.compression_mode) {
        case compression_mode_copy:
            block_decode.init(NULL, (block_type_t)header.block_type, header.parameters,
                              DENSITY_DECODE_END_DATA_OVERHEAD);
            break;
#if 0
        case compression_mode_chameleon_algorithm:
            block_decode.init(compression_mode_chameleon_algorithm,
                              (block_type_t)header.block_type, header.parameters,
                              DENSITY_DECODE_END_DATA_OVERHEAD);
            break;
        case density_compression_mode_cheetah_algorithm:
            block_decode.init(&compression_mode_cheetah_algorithm,
                              (block_type_t)header.block_type, header.parameters,
                              DENSITY_DECODE_END_DATA_OVERHEAD);
            break;
        case density_compression_mode_lion_algorithm:
            block_decode.init(compression_mode_lion_algorithm,
                              (block_type_t)header.block_type, header.parameters,
                              DENSITY_DECODE_END_DATA_OVERHEAD);
            break;
#endif
        default:
            return decode_state_error;
        }
        return exit_process(decode_process_read_blocks, decode_state_ready);
    }
    
    decode_state_t
    decode_t::continue_(teleport_t *RESTRICT in, location_t *RESTRICT out)
    {
        block_decode_state_t block_decode_state;
        uint_fast64_t available_in_before, available_out_before;
        switch (process) {
        case decode_process_read_blocks: goto read_blocks;
        default:  return decode_state_error;
        }
    read_blocks:
        available_in_before = in->available_bytes_reserved(DENSITY_DECODE_END_DATA_OVERHEAD);
        available_out_before = out->available_bytes;

        block_decode_state = block_decode.continue_(in, out);
        update_totals(in, out, available_in_before, available_out_before);
        switch (block_decode_state) {
        case block_decode_state_ready: break;
        case block_decode_state_stall_on_input:
            return exit_process(decode_process_read_blocks, decode_state_stall_on_input);
        case block_decode_state_stall_on_output:
            return exit_process(decode_process_read_blocks, decode_state_stall_on_output);
        case block_decode_state_integrity_check_fail:
            return exit_process(decode_process_read_blocks, decode_state_integrity_check_fail);
        case block_decode_state_error:  return decode_state_error;
        }
        goto read_blocks;
    }

    decode_state_t
    decode_t::finish(teleport_t *RESTRICT in, location_t *RESTRICT out)
    {
        decode_state_t decode_state;
        block_decode_state_t block_decode_state;
        uint_fast64_t available_in_before, available_out_before;
        switch (process) {
        case decode_process_read_blocks:  goto read_blocks;
        case decode_process_read_footer:  goto read_footer;
        default:  return decode_state_error;
        }
    read_blocks:
        available_in_before = in->available_bytes_reserved(DENSITY_DECODE_END_DATA_OVERHEAD);
        available_out_before = out->available_bytes;
        block_decode_state = block_decode.finish(in, out);
        update_totals(in, out, available_in_before, available_out_before);
        switch (block_decode_state) {
        case block_decode_state_ready: break;
        case block_decode_state_stall_on_output:
            return exit_process(decode_process_read_blocks, decode_state_stall_on_output);
        case block_decode_state_integrity_check_fail:
            return exit_process(decode_process_read_blocks, decode_state_integrity_check_fail);
        default:   return decode_state_error;
        }
    read_footer:
#if DENSITY_WRITE_MAIN_FOOTER == DENSITY_YES && DENSITY_ENABLE_PARALLELIZABLE_DECOMPRESSIBLE_OUTPUT == DENSITY_YES
        if ((decode_state = read_footer(in))) return decode_state;
#endif
        //if (state->header.compression_mode != compression_mode_copy) block_decode.free()
        return decode_state_ready;
    }

    decode_state_t
    decode_t::read_header(teleport_t *RESTRICT in)
    {
        location_t *read_location;
        if (!(read_location = in->read_reserved(sizeof(header), DENSITY_DECODE_END_DATA_OVERHEAD)))
            return decode_state_stall_on_input;
        total_read += header.read(read_location);
        return decode_state_ready;
    }
    decode_state_t
    decode_t::read_footer(teleport_t *RESTRICT in)
    {
        location_t *read_location;
        if (!(read_location = in->read(sizeof(footer))))
            return decode_state_stall_on_input;
        total_read += footer.read(read_location);
        return decode_state_ready;
    }
}