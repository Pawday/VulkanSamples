#pragma once

#include <cstdarg>
#include <cstddef>

#include "Functions.hh"

namespace Wayland {

struct ClientCore
{
    FN_wl_event_queue_destroy *wl_event_queue_destroy = nullptr;
    FN_wl_proxy_marshal_flags *wl_proxy_marshal_flags = nullptr;
    FN_wl_proxy_marshal_array_flags *wl_proxy_marshal_array_flags = nullptr;
    FN_wl_proxy_marshal *wl_proxy_marshal = nullptr;
    FN_wl_proxy_marshal_array *wl_proxy_marshal_array = nullptr;
    FN_wl_proxy_create *wl_proxy_create = nullptr;
    FN_wl_proxy_create_wrapper *wl_proxy_create_wrapper = nullptr;
    FN_wl_proxy_wrapper_destroy *wl_proxy_wrapper_destroy = nullptr;
    FN_wl_proxy_marshal_constructor *wl_proxy_marshal_constructor = nullptr;
    FN_wl_proxy_marshal_constructor_versioned
        *wl_proxy_marshal_constructor_versioned = nullptr;
    FN_wl_proxy_marshal_array_constructor *wl_proxy_marshal_array_constructor =
        nullptr;
    FN_wl_proxy_marshal_array_constructor_versioned
        *wl_proxy_marshal_array_constructor_versioned = nullptr;
    FN_wl_proxy_destroy *wl_proxy_destroy = nullptr;
    FN_wl_proxy_add_listener *wl_proxy_add_listener = nullptr;
    FN_wl_proxy_get_listener *wl_proxy_get_listener = nullptr;
    FN_wl_proxy_add_dispatcher *wl_proxy_add_dispatcher = nullptr;
    FN_wl_proxy_set_user_data *wl_proxy_set_user_data = nullptr;
    FN_wl_proxy_get_user_data *wl_proxy_get_user_data = nullptr;
    FN_wl_proxy_get_version *wl_proxy_get_version = nullptr;
    FN_wl_proxy_get_id *wl_proxy_get_id = nullptr;
    FN_wl_proxy_set_tag *wl_proxy_set_tag = nullptr;
    FN_wl_proxy_get_tag *wl_proxy_get_tag = nullptr;
    FN_wl_proxy_get_class *wl_proxy_get_class = nullptr;
    FN_wl_proxy_get_display *wl_proxy_get_display = nullptr;
    FN_wl_proxy_set_queue *wl_proxy_set_queue = nullptr;
    FN_wl_proxy_get_queue *wl_proxy_get_queue = nullptr;
    FN_wl_event_queue_get_name *wl_event_queue_get_name = nullptr;
    FN_wl_display_connect *wl_display_connect = nullptr;
    FN_wl_display_connect_to_fd *wl_display_connect_to_fd = nullptr;
    FN_wl_display_disconnect *wl_display_disconnect = nullptr;
    FN_wl_display_get_fd *wl_display_get_fd = nullptr;
    FN_wl_display_dispatch *wl_display_dispatch = nullptr;
    FN_wl_display_dispatch_queue *wl_display_dispatch_queue = nullptr;
    FN_wl_display_dispatch_timeout *wl_display_dispatch_timeout = nullptr;
    FN_wl_display_dispatch_queue_timeout *wl_display_dispatch_queue_timeout =
        nullptr;
    FN_wl_display_dispatch_queue_pending *wl_display_dispatch_queue_pending =
        nullptr;
    FN_wl_display_dispatch_pending *wl_display_dispatch_pending = nullptr;
    FN_wl_display_get_error *wl_display_get_error = nullptr;
    FN_wl_display_get_protocol_error *wl_display_get_protocol_error = nullptr;
    FN_wl_display_flush *wl_display_flush = nullptr;
    FN_wl_display_roundtrip_queue *wl_display_roundtrip_queue = nullptr;
    FN_wl_display_roundtrip *wl_display_roundtrip = nullptr;
    FN_wl_display_create_queue *wl_display_create_queue = nullptr;
    FN_wl_display_create_queue_with_name *wl_display_create_queue_with_name =
        nullptr;
    FN_wl_display_prepare_read_queue *wl_display_prepare_read_queue = nullptr;
    FN_wl_display_prepare_read *wl_display_prepare_read = nullptr;
    FN_wl_display_cancel_read *wl_display_cancel_read = nullptr;
    FN_wl_display_read_events *wl_display_read_events = nullptr;
    FN_wl_log_set_handler_client *wl_log_set_handler_client = nullptr;
    FN_wl_display_set_max_buffer_size *wl_display_set_max_buffer_size = nullptr;
};

} // namespace Wayland

