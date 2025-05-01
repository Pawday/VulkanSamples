#pragma once

#include <span>

#include <cstdint>

#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>
#include <vulkan/vulkan_to_string.hpp>

struct SimpleVulkanObjects
{
    static vk::raii::RenderPass
        make_render_pass(vk::raii::Device &D, vk::Format format);

    static vk::raii::Pipeline make_pipeline(
        vk::raii::Device &D,
        vk::raii::RenderPass &render_pass,
        vk::raii::PipelineLayout &pipeline_layout,
        std::span<uint32_t> v_shader_code,
        std::span<uint32_t> f_shader_code);

    static vk::DebugUtilsMessengerCreateInfoEXT make_verbose_messenger_ci(
        void *user_data, vk::PFN_DebugUtilsMessengerCallbackEXT callback);
};
