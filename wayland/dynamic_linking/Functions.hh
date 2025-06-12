#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <ctime>

#include "Handles.hh"
#include "Types.hh"

using FN_wl_event_queue_destroy = void(wl_event_queue *queue);

using FN_wl_proxy_marshal_flags = wl_proxy *(
    wl_proxy *proxy,
    uint32_t opcode,
    const wl_interface *interface,
    uint32_t version,
    uint32_t flags,
    ...);

using FN_wl_proxy_marshal_array_flags = wl_proxy *(
    wl_proxy *proxy,
    uint32_t opcode,
    const wl_interface *interface,
    uint32_t version,
    uint32_t flags,
    union wl_argument *args);

using FN_wl_proxy_marshal = void(wl_proxy *p, uint32_t opcode, ...);

using FN_wl_proxy_marshal_array =
    void(wl_proxy *p, uint32_t opcode, union wl_argument *args);

using FN_wl_proxy_create =
    wl_proxy *(wl_proxy *factory, const wl_interface *interface);
using FN_wl_proxy_create_wrapper = void *(void *proxy);
using FN_wl_proxy_wrapper_destroy = void(void *proxy_wrapper);

using FN_wl_proxy_marshal_constructor = wl_proxy *(
    wl_proxy *proxy, uint32_t opcode, const wl_interface *interface, ...);

using FN_wl_proxy_marshal_constructor_versioned = wl_proxy *(
    wl_proxy *proxy,
    uint32_t opcode,
    const wl_interface *interface,
    uint32_t version,
    ...);

using FN_wl_proxy_marshal_array_constructor = wl_proxy *(
    wl_proxy *proxy,
    uint32_t opcode,
    union wl_argument *args,
    const wl_interface *interface);

using FN_wl_proxy_marshal_array_constructor_versioned = wl_proxy *(
    wl_proxy *proxy,
    uint32_t opcode,
    union wl_argument *args,
    const wl_interface *interface,
    uint32_t version);

using FN_wl_proxy_destroy = void(wl_proxy *proxy);

using FN_wl_proxy_add_listener =
    int(wl_proxy *proxy, void (**implementation)(void), void *data);

using FN_wl_proxy_get_listener = const void *(wl_proxy *proxy);

using FN_wl_proxy_add_dispatcher =
    int(wl_proxy *proxy,
        wl_dispatcher_func_t dispatcher_func,
        const void *dispatcher_data,
        void *data);

using FN_wl_proxy_set_user_data = void(wl_proxy *proxy, void *user_data);
using FN_wl_proxy_get_user_data = void *(wl_proxy *proxy);
using FN_wl_proxy_get_version = uint32_t(wl_proxy *proxy);
using FN_wl_proxy_get_id = uint32_t(wl_proxy *proxy);
using FN_wl_proxy_set_tag = void(wl_proxy *proxy, const char *const *tag);
using FN_wl_proxy_get_tag = const char *const *(wl_proxy *proxy);
using FN_wl_proxy_get_class = const char *(wl_proxy *proxy);
using FN_wl_proxy_get_display = wl_display *(wl_proxy *proxy);
using FN_wl_proxy_set_queue = void(wl_proxy *proxy, wl_event_queue *queue);
using FN_wl_proxy_get_queue = wl_event_queue *(const wl_proxy *proxy);
using FN_wl_event_queue_get_name = const char *(const wl_event_queue *queue);
using FN_wl_display_connect = wl_display *(const char *name);
using FN_wl_display_connect_to_fd = wl_display *(int fd);
using FN_wl_display_disconnect = void(wl_display *display);
using FN_wl_display_get_fd = int(wl_display *display);
using FN_wl_display_dispatch = int(wl_display *display);
using FN_wl_display_dispatch_queue =
    int(wl_display *display, wl_event_queue *queue);

using FN_wl_display_dispatch_timeout =
    int(wl_display *display, const timespec *timeout);
using FN_wl_display_dispatch_queue_timeout =
    int(wl_display *display, wl_event_queue *queue, const timespec *timeout);
using FN_wl_display_dispatch_queue_pending =
    int(wl_display *display, wl_event_queue *queue);

using FN_wl_display_dispatch_pending = int(wl_display *display);
using FN_wl_display_get_error = int(wl_display *display);

using FN_wl_display_get_protocol_error =
    uint32_t(wl_display *display, const wl_interface **interface, uint32_t *id);

using FN_wl_display_flush = int(wl_display *display);
using FN_wl_display_roundtrip_queue =
    int(wl_display *display, wl_event_queue *queue);
using FN_wl_display_roundtrip = int(wl_display *display);
using FN_wl_display_create_queue = wl_event_queue *(wl_display *display);

using FN_wl_display_create_queue_with_name =
    wl_event_queue *(wl_display *display, const char *name);

using FN_wl_display_prepare_read_queue =
    int(wl_display *display, wl_event_queue *queue);

using FN_wl_display_prepare_read = int(wl_display *display);
using FN_wl_display_cancel_read = void(wl_display *display);
using FN_wl_display_read_events = int(wl_display *display);
using FN_wl_log_set_handler_client = void(wl_log_func_t handler);

using FN_wl_display_set_max_buffer_size =
    void(wl_display *display, size_t max_buffer_size);

