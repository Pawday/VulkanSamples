#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <format>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <cstdlib>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>
#include <vulkan/vulkan_to_string.hpp>

#include "shaders/spans.hh"

#include "wayland/WaylandContext.hh"

#include "FormatTools.hh"
#include "Messenger.hh"
#include "SimpleVulkanObjects.hh"

constexpr size_t device_idx = 0;
constexpr auto prefered_transparency_mod =
    vk::CompositeAlphaFlagBitsKHR::ePreMultiplied;

int main()
try {
    vk::raii::Context ctx;

    Messenger VKI_msgr{"VKI"};
    auto msgr_ci = SimpleVulkanObjects::make_verbose_messenger_ci(
        &VKI_msgr,
        [](vk::DebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
           vk::DebugUtilsMessageTypeFlagsEXT /*messageTypes*/,
           const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
           void *pUserData) -> vk::Bool32 {
            Messenger &m = *reinterpret_cast<Messenger *>(pUserData);

            m.message(pCallbackData->pMessage);

            return false;
        });

    std::shared_ptr<vk::raii::Instance> VKI = [&ctx, &VKI_msgr, &msgr_ci]() {
        vk::InstanceCreateInfo in_ci{};

        do {
            auto exts = ctx.enumerateInstanceExtensionProperties();
            std::vector<std::string> exten_strings;
            for (auto &ext : exts) {
                exten_strings.push_back(ext.extensionName.data());
            }

            format_table(
                "Extentions", exten_strings, [&VKI_msgr](std::string_view msg) {
                    VKI_msgr.message(msg);
                });

        } while (false);

        in_ci.setPNext(&msgr_ci);

        std::vector<const char *> VKI_exts;
        VKI_exts.push_back("VK_EXT_debug_utils");

        VKI_exts.push_back("VK_KHR_surface");
        VKI_exts.push_back("VK_KHR_wayland_surface");

        std::vector<const char *> layers;
        layers.push_back("VK_LAYER_KHRONOS_validation");

        in_ci.setPEnabledExtensionNames(VKI_exts);
        in_ci.setPEnabledLayerNames(layers);

        vk::ApplicationInfo app_info{};
        app_info.apiVersion = vk::ApiVersion13;
        in_ci.setPApplicationInfo(&app_info);

        return std::make_shared<vk::raii::Instance>(ctx.createInstance(in_ci));
    }();

    vk::raii::DebugUtilsMessengerEXT msgr = [&]() {
        return VKI->createDebugUtilsMessengerEXT(msgr_ci);
    }();

    WaylandContext wl_ctx{VKI};

    auto D_phy = [&VKI]() {
        auto phys = VKI->enumeratePhysicalDevices();
        return phys.at(device_idx);
    }();

    struct DeviceWithQueues
    {
        vk::raii::Device dev;
        std::vector<vk::raii::Queue> g_qs;
    };

    DeviceWithQueues Q_dev = [&D_phy]() {
        auto q_fams = D_phy.getQueueFamilyProperties();

        std::optional<size_t> g_fam_idx{};

        for (size_t idx = 0; idx != q_fams.size(); ++idx) {
            auto &q = q_fams[idx];

            auto q_fam_graphics = q.queueFlags & vk::QueueFlagBits::eGraphics;
            if (q_fam_graphics != vk::QueueFlagBits::eGraphics) {
                continue;
            }

            g_fam_idx = idx;
            break;
        }

        std::array<vk::DeviceQueueCreateInfo, 1> q_cis{};
        vk::DeviceQueueCreateInfo &q_ci = q_cis[0];
        q_ci.queueFamilyIndex = g_fam_idx.value();
        q_ci.queueCount = q_fams[q_ci.queueFamilyIndex].queueCount;
        std::vector<float> priors;
        priors.resize(q_ci.queueCount, 1.0);
        q_ci.setQueuePriorities(priors);

        vk::DeviceCreateInfo dev_ci{};

        const char *dev_exts[] = {
            "VK_KHR_swapchain", "VK_KHR_shader_non_semantic_info"};

        vk::PhysicalDeviceVulkan13Features vk13_features{};
        vk13_features.synchronization2 = true;

        dev_ci.setQueueCreateInfos(q_cis);
        dev_ci.setPEnabledExtensionNames(dev_exts);
        dev_ci.setPNext(&vk13_features);

        auto D = D_phy.createDevice(dev_ci);

        std::vector<vk::raii::Queue> qs;

        for (size_t q_idx = 0; q_idx != q_ci.queueCount; ++q_idx) {
            qs.emplace_back(D.getQueue(q_ci.queueFamilyIndex, q_idx));
        }

        return DeviceWithQueues{std::move(D), std::move(qs)};
    }();

    auto &D = Q_dev.dev;

    auto win = wl_ctx.create_window();
    auto &surface = win.surface();

    vk::SurfaceFormatKHR surface_format = [&D_phy, &surface]() {
        auto surface_formants = D_phy.getSurfaceFormatsKHR(surface);

        std::optional<vk::SurfaceFormatKHR> o{};

        for (auto &fmt : surface_formants) {
            std::cout << std::format(
                "Fmt:{} colorspace:{}\n",
                vk::to_string(fmt.format),
                vk::to_string(fmt.colorSpace));

            bool supported_format = false;
            switch (fmt.format) {
            case vk::Format::eR8G8B8A8Unorm:
            case vk::Format::eB8G8R8A8Unorm:
                supported_format = true;
                break;
            default:
                break;
            }

            if (supported_format) {
                o = fmt.format;
                break;
            }
        }

        return o.value();
    }();

    vk::Extent2D swapchain_resolution{};
    swapchain_resolution.width = 640;
    swapchain_resolution.height = 480;

    auto swapchain = [&]() {
        auto present_mode = [&D_phy, &surface]() {
            std::optional<vk::PresentModeKHR> o{};
            std::vector<vk::PresentModeKHR> modes =
                D_phy.getSurfacePresentModesKHR(surface);

            for (auto &mode : modes) {

                bool supported_mode = false;

                switch (mode) {
                case vk::PresentModeKHR::eImmediate:
                case vk::PresentModeKHR::eFifo:
                    supported_mode = true;
                    break;
                default:
                    break;
                }

                if (supported_mode) {
                    o = mode;
                    break;
                }
            }

            return o.value();
        }();

        vk::SurfaceCapabilitiesKHR capabilities =
            D_phy.getSurfaceCapabilitiesKHR(surface);

        bool asignable_extend =
            capabilities.currentExtent.width ==
            std::numeric_limits<
                decltype(capabilities.currentExtent.width)>::max();
        if (asignable_extend) {
            auto min = capabilities.minImageExtent;
            swapchain_resolution.width =
                std::max(min.width, swapchain_resolution.width);
            swapchain_resolution.height =
                std::max(min.height, swapchain_resolution.height);
        }

        vk::CompositeAlphaFlagBitsKHR alpha_flags =
            vk::CompositeAlphaFlagBitsKHR::eOpaque;
        auto check_flag = [&capabilities](vk::CompositeAlphaFlagBitsKHR flag) {
            auto filtered = capabilities.supportedCompositeAlpha & flag;
            return filtered == flag;
        };

        if (check_flag(prefered_transparency_mod)) {
            alpha_flags = prefered_transparency_mod;
        }

        vk::SwapchainCreateInfoKHR swapchain_ci{};
        swapchain_ci.surface = surface;
        swapchain_ci.minImageCount = std::max(capabilities.maxImageCount, 1u);
        swapchain_ci.imageFormat = surface_format.format;
        swapchain_ci.imageColorSpace = surface_format.colorSpace;
        swapchain_ci.imageExtent = swapchain_resolution;
        swapchain_ci.imageArrayLayers = 1;
        swapchain_ci.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment;
        swapchain_ci.imageUsage |= vk::ImageUsageFlagBits::eTransferDst;

        swapchain_ci.presentMode = present_mode;
        swapchain_ci.preTransform = capabilities.currentTransform;
        swapchain_ci.compositeAlpha = alpha_flags;

        std::vector<uint32_t> all_q_fams_idxs = [&D_phy]() {
            std::vector<uint32_t> o;
            auto size = D_phy.getQueueFamilyProperties().size();
            for (size_t idx = 0; idx != size; ++idx) {
                o.push_back(idx);
            }
            return o;
        }();

        swapchain_ci.imageSharingMode = vk::SharingMode::eExclusive;
        if (all_q_fams_idxs.size() > 1) {
            swapchain_ci.imageSharingMode = vk::SharingMode::eConcurrent;
            swapchain_ci.setQueueFamilyIndices(all_q_fams_idxs);
        }

        return D.createSwapchainKHR(swapchain_ci);
    }();

    vk::ImageSubresourceRange images_subresource{};
    images_subresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    images_subresource.baseMipLevel = 0;
    images_subresource.levelCount = 1;
    images_subresource.baseArrayLayer = 0;
    images_subresource.layerCount = 1;

    std::vector<vk::Image> images = swapchain.getImages();
    std::vector<vk::raii::ImageView> image_views =
        [&images, &D, &surface_format, &images_subresource]() {
            std::vector<vk::raii::ImageView> o;
            for (auto &image : images) {
                vk::ImageViewCreateInfo ci{};
                ci.image = image;
                ci.format = surface_format.format;
                ci.viewType = vk::ImageViewType::e2D;

                vk::ComponentSwizzle id_swizzle =
                    vk::ComponentSwizzle::eIdentity;
                ci.components.r = id_swizzle;
                ci.components.g = id_swizzle;
                ci.components.b = id_swizzle;
                ci.components.a = id_swizzle;

                ci.subresourceRange = images_subresource;
                o.emplace_back(D.createImageView(ci));
            }
            return o;
        }();

    auto render_pass =
        SimpleVulkanObjects::make_render_pass(D, surface_format.format);

    vk::raii::PipelineLayout pipeline_layout = [&D]() {
        vk::PipelineLayoutCreateInfo p_layout_ci{};
        return D.createPipelineLayout(p_layout_ci);
    }();

    auto vertex_shader_code = get_shader_vert_trig();
    auto fragment_shader_code = get_shader_frag_trig();

    vk::raii::Pipeline pipeline = SimpleVulkanObjects::make_pipeline(
        D,
        render_pass,
        pipeline_layout,
        vertex_shader_code,
        fragment_shader_code);

    std::vector<vk::raii::Framebuffer> fbms =
        [&D, &render_pass, &image_views, &swapchain_resolution]() {
            std::vector<vk::raii::Framebuffer> o;

            for (auto &image : image_views) {
                std::array<vk::ImageView, 1> attach{image};

                vk::FramebufferCreateInfo fbm_ci{};
                fbm_ci.renderPass = render_pass;
                fbm_ci.setAttachments(attach);
                fbm_ci.width = swapchain_resolution.width;
                fbm_ci.height = swapchain_resolution.height;
                fbm_ci.layers = 1;
                o.emplace_back(D.createFramebuffer(fbm_ci));
            }
            return o;
        }();

    std::vector<vk::RenderPassBeginInfo> render_pass_begin_info =
        [&render_pass, &swapchain_resolution, &fbms]() {
            std::vector<vk::RenderPassBeginInfo> o;

            for (auto &fbm : fbms) {
                vk::RenderPassBeginInfo rp_i{};
                rp_i.renderPass = render_pass;
                rp_i.framebuffer = fbm;
                std::array<vk::ClearValue, 1> cls{};
                cls[0].color = {0, 0, 0, 0};
                rp_i.renderArea.extent = swapchain_resolution;
                rp_i.setClearValues(cls);
                o.push_back(rp_i);
            }

            return o;
        }();

    vk::CommandPoolCreateInfo cmdp_ci{};
    auto cmdp = D.createCommandPool(cmdp_ci);

    auto cmd_buffers = [&D, &cmdp, &fbms]() {
        vk::CommandBufferAllocateInfo cmdb_ci{};
        cmdb_ci.commandPool = cmdp;
        cmdb_ci.commandBufferCount = fbms.size();
        return D.allocateCommandBuffers(cmdb_ci);
    }();

    vk::DependencyInfoKHR dep_info{};
    std::array<vk::ImageMemoryBarrier2, 1> image_bars{};
    auto &image_bar = image_bars[0];

    std::array<vk::Viewport, 1> viewports;
    std::array<vk::Rect2D, 1> scizors;
    scizors[0].extent = swapchain_resolution;
    viewports[0].height = swapchain_resolution.height;
    viewports[0].width = swapchain_resolution.width;

    for (size_t idx = 0; idx != fbms.size(); ++idx) {
        auto &cmd_buffer = cmd_buffers[idx];
        auto &rp_i = render_pass_begin_info[idx];

        image_bar.image = images[idx];
        image_bar.subresourceRange = images_subresource;

        cmd_buffer.begin({});
        cmd_buffer.setViewport(0, viewports);
        cmd_buffer.setScissor(0, scizors);

        image_bar.newLayout = vk::ImageLayout::eGeneral;
        dep_info.setImageMemoryBarriers(image_bars);
        cmd_buffer.pipelineBarrier2(dep_info);
        cmd_buffer.beginRenderPass(rp_i, vk::SubpassContents::eInline);
        cmd_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
        cmd_buffer.draw(3 * 1, 1, 0, 0);
        cmd_buffer.endRenderPass();

        image_bar.newLayout = vk::ImageLayout::ePresentSrcKHR;
        dep_info.setImageMemoryBarriers(image_bars);
        cmd_buffer.pipelineBarrier2(dep_info);

        cmd_buffer.end();
    }

    auto draw_done_fence = D.createFence({});

    std::array<vk::SubmitInfo, 1> submit_infos{};

    auto image_acuired_sem = D.createSemaphore({});
    auto draw_fence = D.createFence({});

    auto &main_q = Q_dev.g_qs.at(0);

    while (true) {
        auto image_idx = swapchain.acquireNextImage(100, image_acuired_sem);

        if (image_idx.first == vk::Result::eTimeout) {
            std::this_thread::yield();
            continue;
        }

        if (image_idx.first != vk::Result::eSuccess) {
            std::cout << "Breaking loop: acquireNextImage() == "
                      << vk::to_string(image_idx.first);
            break;
        }

        auto &cmd_buffer = cmd_buffers[image_idx.second];
        auto &submit_info = submit_infos[0];
        submit_info = vk::SubmitInfo{};
        std::array<vk::PipelineStageFlags, 1> stages{};
        stages[0] = vk::PipelineStageFlagBits::eAllGraphics;
        submit_info.setWaitDstStageMask(stages);
        std::array<vk::Semaphore, 1> semas{image_acuired_sem};
        submit_info.setWaitSemaphores(semas);
        std::array<vk::CommandBuffer, 1> cmds{*cmd_buffer};
        submit_info.setCommandBuffers(cmds);

        main_q.submit(submit_infos, draw_fence);

        size_t wait_iterations = 0;

        auto fence_wait_status = vk::Result::eTimeout;

        while (fence_wait_status == vk::Result::eTimeout) {
            wait_iterations++;
            fence_wait_status = D.waitForFences({draw_fence}, true, 0);
        }

        if (fence_wait_status != vk::Result::eSuccess) {
            std::cout << "Wait status error: "
                      << vk::to_string(fence_wait_status) << '\n';
            break;
        }
        D.resetFences({draw_fence});

        std::cout << "Draw taken " << wait_iterations << " cycles\n";

        vk::PresentInfoKHR present_i{};
        std::array<uint32_t, 1> indexes{image_idx.second};
        std::array<vk::SwapchainKHR, 1> swchs{*swapchain};
        present_i.setSwapchains(swchs);
        present_i.setImageIndices(indexes);
        auto present_res = main_q.presentKHR(present_i);

        if (present_res != vk::Result::eSuccess) {
            std::cout << vk::to_string(present_res) << '\n';
            break;
        }

        wl_ctx.update();
    }

} catch (std::exception &e) {
    std::cout << e.what() << '\n';
    return EXIT_FAILURE;
}
