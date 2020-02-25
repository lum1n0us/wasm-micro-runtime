#include "lib_export.h"
#include "sensor_native_api.h"
#include "connection_native_api.h"
#include "display_indev.h"

static NativeSymbol extended_native_symbol_defs[] = {
    #include "runtime_sensor.inl"
    #include "connection.inl"
    EXPORT_WASM_API_WITH_SIG(display_init, "()"),
    EXPORT_WASM_API_WITH_SIG(display_input_read, "(*)i"),
    EXPORT_WASM_API_WITH_SIG(display_flush, "(iiii*)"),
    EXPORT_WASM_API_WITH_SIG(display_fill, "(iiii*)"),
    EXPORT_WASM_API_WITH_SIG(display_vdb_write, "(*iii*i)"),
    EXPORT_WASM_API_WITH_SIG(display_map, "(iiii*)"),
    EXPORT_WASM_API_WITH_SIG(time_get_ms, "()i")
};

#include "ext_lib_export.h"
