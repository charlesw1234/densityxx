#include "densityxx/lion.hpp"

namespace density {
    // encode.
    lion_encode_t::lion_encode_t(void): kernel_encode_t()
    {
        // FIXME: NOT IMPLEMENTED YET.
    }
    lion_encode_t::~lion_encode_t()
    {
        // FIXME: NOT IMPLEMENTED YET.
    }
    kernel_encode_state_t
    lion_encode_t::init(void)
    {
        // FIXME: NOT IMPLEMENTED YET.
        return kernel_encode_state_ready;
    }
    kernel_encode_state_t
    lion_encode_t::continue_(teleport_t *RESTRICT in, location_t *RESTRICT out)
    {
        // FIXME: NOT IMPLEMENTED YET.
        return kernel_encode_state_ready;
    }
    kernel_encode_state_t
    lion_encode_t::finish(teleport_t *RESTRICT in, location_t *RESTRICT out)
    {
        // FIXME: NOT IMPLEMENTED YET.
        return kernel_encode_state_ready;
    }

    // decode.
    lion_decode_t::lion_decode_t(const main_header_parameters_t parameters,
                                 const uint_fast8_t end_data_overhead):
        kernel_decode_t(parameters, end_data_overhead)
    {
        // FIXME: NOT IMPLEMENTED YET.
    }
    lion_decode_t::~lion_decode_t()
    {
        // FIXME: NOT IMPLEMENTED YET.
    }
    kernel_decode_state_t
    lion_decode_t::continue_(teleport_t *RESTRICT in, location_t *RESTRICT out)
    {
        // FIXME: NOT IMPLEMENTED YET.
        return kernel_decode_state_ready;
    }
    kernel_decode_state_t
    lion_decode_t::finish(teleport_t *RESTRICT in, location_t *RESTRICT out)
    {
        // FIXME: NOT IMPLEMENTED YET.
        return kernel_decode_state_ready;
    }
}
