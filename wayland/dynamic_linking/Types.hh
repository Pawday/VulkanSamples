#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdint>

#include "Handles.hh"

namespace Wayland {

struct Message
{
    /** Message name */
    const char *name;
    /** Message signature */
    const char *signature;
    /** Object argument interfaces */
    const Interface **types;
};

struct Array
{
    /** Array size */
    size_t size;
    /** Allocated space */
    size_t alloc;
    /** Array data */
    void *data;
};

using Fixed = int32_t;

union Argument {
    int32_t i;     /**< `int`    */
    uint32_t u;    /**< `uint`   */
    Fixed f;       /**< `fixed`  */
    const char *s; /**< `string` */
    Object *o;     /**< `object` */
    uint32_t n;    /**< `new_id` */
    Array *a;      /**< `array`  */
    int32_t h;     /**< `fd`     */
};

using wl_dispatcher_func_t = int (*)(
    const void *user_data,
    void *target,
    uint32_t opcode,
    const Message *msg,
    union Argument *args);

using LogFunc = void (*)(const char *fmt, va_list args);

} // namespace Wayland
