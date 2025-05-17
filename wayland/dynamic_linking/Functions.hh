#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <ctime>

#include "Handles.hh"
#include "Types.hh"

namespace Wayland {

using FN_wl_event_queue_destroy = void(EventQueue *queue);

using FN_wl_proxy_marshal_flags = Proxy *(
    Proxy *proxy,
    uint32_t opcode,
    const Interface *interface,
    uint32_t version,
    uint32_t flags,
    ...);

using FN_wl_proxy_marshal_array_flags = Proxy *(
    Proxy *proxy,
    uint32_t opcode,
    const Interface *interface,
    uint32_t version,
    uint32_t flags,
    union Argument *args);

using FN_wl_proxy_marshal = void(Proxy *p, uint32_t opcode, ...);

using FN_wl_proxy_marshal_array =
    void(Proxy *p, uint32_t opcode, union Argument *args);

using FN_wl_proxy_create = Proxy *(Proxy *factory, const Interface *interface);
using FN_wl_proxy_create_wrapper = void *(void *proxy);
using FN_wl_proxy_wrapper_destroy = void(void *proxy_wrapper);

using FN_wl_proxy_marshal_constructor =
    Proxy *(Proxy *proxy, uint32_t opcode, const Interface *interface, ...);

using FN_wl_proxy_marshal_constructor_versioned = Proxy *(
    Proxy *proxy,
    uint32_t opcode,
    const Interface *interface,
    uint32_t version,
    ...);

using FN_wl_proxy_marshal_array_constructor = Proxy *(
    Proxy *proxy,
    uint32_t opcode,
    union Argument *args,
    const Interface *interface);

using FN_wl_proxy_marshal_array_constructor_versioned = Proxy *(
    Proxy *proxy,
    uint32_t opcode,
    union Argument *args,
    const Interface *interface,
    uint32_t version);

using FN_wl_proxy_destroy = void(Proxy *proxy);

using FN_wl_proxy_add_listener =
    int(Proxy *proxy, void (**implementation)(void), void *data);

using FN_wl_proxy_get_listener = const void *(Proxy *proxy);

using FN_wl_proxy_add_dispatcher =
    int(Proxy *proxy,
        wl_dispatcher_func_t dispatcher_func,
        const void *dispatcher_data,
        void *data);

using FN_wl_proxy_set_user_data = void(Proxy *proxy, void *user_data);
using FN_wl_proxy_get_user_data = void *(Proxy *proxy);
using FN_wl_proxy_get_version = uint32_t(Proxy *proxy);
using FN_wl_proxy_get_id = uint32_t(Proxy *proxy);
using FN_wl_proxy_set_tag = void(Proxy *proxy, const char *const *tag);
using FN_wl_proxy_get_tag = const char *const *(Proxy *proxy);
using FN_wl_proxy_get_class = const char *(Proxy *proxy);
using FN_wl_proxy_get_display = Display *(Proxy *proxy);
using FN_wl_proxy_set_queue = void(Proxy *proxy, EventQueue *queue);
using FN_wl_proxy_get_queue = EventQueue *(const Proxy *proxy);
using FN_wl_event_queue_get_name = const char *(const EventQueue *queue);
using FN_wl_display_connect = Display *(const char *name);
using FN_wl_display_connect_to_fd = Display *(int fd);
using FN_wl_display_disconnect = void(Display *display);
using FN_wl_display_get_fd = int(Display *display);
using FN_wl_display_dispatch = int(Display *display);
using FN_wl_display_dispatch_queue = int(Display *display, EventQueue *queue);

using FN_wl_display_dispatch_timeout =
    int(Display *display, const timespec *timeout);
using FN_wl_display_dispatch_queue_timeout =
    int(Display *display, EventQueue *queue, const timespec *timeout);
using FN_wl_display_dispatch_queue_pending =
    int(Display *display, EventQueue *queue);

using FN_wl_display_dispatch_pending = int(Display *display);
using FN_wl_display_get_error = int(Display *display);

using FN_wl_display_get_protocol_error =
    uint32_t(Display *display, const Interface **interface, uint32_t *id);

using FN_wl_display_flush = int(Display *display);
using FN_wl_display_roundtrip_queue = int(Display *display, EventQueue *queue);
using FN_wl_display_roundtrip = int(Display *display);
using FN_wl_display_create_queue = EventQueue *(Display *display);

using FN_wl_display_create_queue_with_name =
    EventQueue *(Display *display, const char *name);

using FN_wl_display_prepare_read_queue =
    int(Display *display, EventQueue *queue);

using FN_wl_display_prepare_read = int(Display *display);
using FN_wl_display_cancel_read = void(Display *display);
using FN_wl_display_read_events = int(Display *display);
using FN_wl_log_set_handler_client = void(LogFunc handler);

using FN_wl_display_set_max_buffer_size =
    void(Display *display, size_t max_buffer_size);

} // namespace Wayland
