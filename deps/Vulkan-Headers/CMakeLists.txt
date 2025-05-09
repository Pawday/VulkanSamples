list(APPEND HEADERS
    vulkan_directfb.h
    vulkan_xlib.h
    vulkan_handles.hpp
    vulkan_enums.hpp
    vulkan_ios.h
    vulkan_ggp.h
    vulkan_win32.h
    vulkan.h
    vulkan_metal.h
    vulkan_screen.h
    vulkan_xcb.h
    vulkan_video.hpp
    vulkan_funcs.hpp
    vulkan_hpp_macros.hpp
    vulkan.hpp
    vulkan_structs.hpp
    vulkan_shared.hpp
    vulkan_format_traits.hpp
    vulkan_to_string.hpp
    vulkan_core.h
    vk_icd.h
    vulkan_extension_inspection.hpp
    vulkan_xlib_xrandr.h
    vk_platform.h
    vulkan_android.h
    vulkan_static_assertions.hpp
    vulkan_beta.h
    vulkan_wayland.h
    vk_layer.h
    vulkan_vi.h
    vulkan_fuchsia.h
    vulkan_raii.hpp
    vulkan_hash.hpp
    vulkan_macos.h
)

list(APPEND VK_VIDEO_HEADERS
    vulkan_video_codec_av1std.h
    vulkan_video_codec_av1std_decode.h
    vulkan_video_codec_av1std_encode.h
    vulkan_video_codec_h264std.h
    vulkan_video_codec_h264std_decode.h
    vulkan_video_codec_h264std_encode.h
    vulkan_video_codec_h265std.h
    vulkan_video_codec_h265std_decode.h
    vulkan_video_codec_h265std_encode.h
    vulkan_video_codecs_common.h
)

list(TRANSFORM HEADERS PREPEND "src/include/vulkan/")
list(TRANSFORM VK_VIDEO_HEADERS PREPEND "src/include/vk_video/")

target_sources(${PREF}vulkan.headers INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "src/include"
    FILES ${HEADERS} ${VK_VIDEO_HEADERS}
)
