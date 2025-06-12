#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdint>

#include "Handles.hh"

struct wl_interface;

struct wl_message
{
    /** Message name */
    const char *name;
    /** Message signature */
    const char *signature;
    /** Object argument interfaces */
    const wl_interface **types;
};

struct wl_interface
{
    /** Interface name */
    const char *name;
    /** Interface version */
    int version;
    /** Number of methods (requests) */
    int method_count;
    /** Method (request) signatures */
    const wl_message *methods;
    /** Number of events */
    int event_count;
    /** Event signatures */
    const wl_message *events;
};

struct wl_array
{
    /** Array size */
    size_t size;
    /** Allocated space */
    size_t alloc;
    /** Array data */
    void *data;
};

using wl_fixed_t = int32_t;

union wl_argument {
    int32_t i;     /**< `int`    */
    uint32_t u;    /**< `uint`   */
    wl_fixed_t f;  /**< `fixed`  */
    const char *s; /**< `string` */
    wl_object *o;  /**< `object` */
    uint32_t n;    /**< `new_id` */
    wl_array *a;   /**< `array`  */
    int32_t h;     /**< `fd`     */
};

using wl_dispatcher_func_t = int (*)(
    const void *user_data,
    void *target,
    uint32_t opcode,
    const wl_message *msg,
    union wl_argument *args);

using wl_log_func_t = void (*)(const char *fmt, va_list args);

