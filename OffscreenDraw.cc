#include <algorithm>
#include <array>
#include <cstdlib>
#include <format>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cstddef>
#include <cstdint>

#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>
#include <vulkan/vulkan_to_string.hpp>

#include "shaders/spans.hh"

#include "FormatTools.hh"
#include "Messenger.hh"
#include "SimpleVulkanObjects.hh"

#include "Application.hh"

constexpr size_t device_idx = 0;

struct DeviceWrap
{
    DeviceWrap(
        vk::raii::Device dev,
        std::vector<vk::raii::Queue> graphic_queues,
        vk::raii::PhysicalDevice phy)
        : m_dev(std::move(dev)), m_graphic_queues(std::move(graphic_queues)),
          m_phy(phy)
    {
    }

    vk::raii::Device *operator->()
    {
        return &m_dev;
    }

    vk::raii::PhysicalDevice &phy()
    {
        return m_phy;
    };

    vk::raii::Device &get()
    {
        return m_dev;
    }

    vk::raii::Queue main_gq()
    {
        return m_graphic_queues.at(0);
    }

  private:
    vk::raii::Device m_dev;
    std::vector<vk::raii::Queue> m_graphic_queues;
    vk::raii::PhysicalDevice m_phy;
};

struct MemTypeIndexMaping
{
    std::unordered_set<uint32_t> device_local_indexes;
    std::unordered_set<uint32_t> host_visible_indexes;
};

namespace {
MemTypeIndexMaping
    retreave_mem_type_idx_mapping(vk::PhysicalDeviceMemoryProperties &mem_props)
{
    MemTypeIndexMaping output;

    std::span<vk::MemoryType> mem_types{
        mem_props.memoryTypes.data(), mem_props.memoryTypeCount};

    for (uint8_t mem_type_idx = 0; mem_type_idx < mem_types.size();
         mem_type_idx++) {

        auto mem_type_flags = mem_types[mem_type_idx].propertyFlags;

        if (mem_type_flags & vk::MemoryPropertyFlagBits::eDeviceLocal) {
            output.device_local_indexes.emplace(mem_type_idx);
        }

        if (mem_type_flags & vk::MemoryPropertyFlagBits::eHostVisible) {
            output.host_visible_indexes.emplace(mem_type_idx);
        }
    }

    return output;
}
} // namespace

struct DeviceLocalImage
{
    DeviceLocalImage(
        vk::raii::PhysicalDevice &phy,
        vk::raii::Device &D,
        vk::Extent2D resolution)
        : m_fmt(vk::Format::eR8G8B8A8Unorm),

          m_image([&D, &resolution, this]() {
              vk::ImageCreateInfo image_ci;
              image_ci.format = m_fmt;
              image_ci.extent =
                  vk::Extent3D{resolution.width, resolution.height, 1};
              image_ci.imageType = vk::ImageType::e2D;
              image_ci.usage |= vk::ImageUsageFlagBits::eColorAttachment;
              image_ci.usage |= vk::ImageUsageFlagBits::eTransferSrc;
              image_ci.mipLevels = 1;
              image_ci.arrayLayers = 1;

              return D.createImage(image_ci);
          }()),
          m_mem_req(m_image.getMemoryRequirements()),
          m_image_mem([this, &D, &phy]() {
              vk::MemoryAllocateInfo mem_ci{};

              auto mem_props = phy.getMemoryProperties();

              auto mem_t_mapings = retreave_mem_type_idx_mapping(mem_props);

              std::span<vk::MemoryType> mem_types{
                  mem_props.memoryTypes.data(), mem_props.memoryTypeCount};

              std::optional<uint32_t> capable_device_local_mem_type_idx;
              for (uint8_t mem_type_idx = 0; mem_type_idx < mem_types.size();
                   mem_type_idx++) {

                  uint32_t memt_mask = 1;
                  memt_mask <<= mem_type_idx;
                  bool image_req_bit_set = m_mem_req.memoryTypeBits & memt_mask;

                  bool is_device_local =
                      mem_t_mapings.device_local_indexes.contains(mem_type_idx);

                  if (image_req_bit_set && is_device_local) {
                      capable_device_local_mem_type_idx = mem_type_idx;
                  }
              }

              mem_ci.allocationSize = m_mem_req.size;
              mem_ci.memoryTypeIndex =
                  capable_device_local_mem_type_idx.value();
              return D.allocateMemory(mem_ci);
          }()),
          m_sizes(resolution), m_buffer([&D, this]() {
              vk::BufferCreateInfo b_ci{};
              b_ci.usage = vk::BufferUsageFlagBits::eTransferSrc;
              b_ci.size = m_mem_req.size;
              return D.createBuffer(b_ci);
          }())
    {
        m_image.bindMemory(m_image_mem, 0);
        m_buffer.bindMemory(m_image_mem, 0);

        m_view = [this, &D]() {
            vk::ImageViewCreateInfo imv_ci{};

            vk::ImageSubresourceRange r{};
            r.aspectMask = vk::ImageAspectFlagBits::eColor;
            r.levelCount = 1;
            r.layerCount = 1;

            imv_ci.format = m_fmt;
            imv_ci.image = m_image;
            imv_ci.viewType = vk::ImageViewType::e2D;
            imv_ci.subresourceRange = r;
            return D.createImageView(imv_ci);
        }();
    }

    vk::Image image()
    {
        return m_image;
    }

    auto width() const
    {
        return m_sizes.width;
    }

    auto height() const
    {
        return m_sizes.height;
    }

    vk::Format format() const
    {
        return m_fmt;
    }

    vk::ImageView view() const
    {
        return m_view.value();
    }

    uint64_t buffer_size() const
    {
        return m_mem_req.size;
    }

  private:
    vk::Format m_fmt;
    vk::raii::Image m_image;
    vk::MemoryRequirements m_mem_req;
    vk::raii::DeviceMemory m_image_mem;
    vk::Extent2D m_sizes;
    std::optional<vk::raii::ImageView> m_view;

    vk::raii::Buffer m_buffer;
};

struct HostVisMemBuffer
{
    HostVisMemBuffer(
        vk::raii::PhysicalDevice &phy, vk::raii::Device &D, uint64_t size)
        : m_mem([&]() {
              auto mem_props = phy.getMemoryProperties();
              auto mem_types = retreave_mem_type_idx_mapping(mem_props);

              if (mem_types.host_visible_indexes.empty()) {
                  throw std::runtime_error(
                      "This device has no single mem idx with "
                      "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT");
              }
              uint32_t host_vis_idx = *mem_types.host_visible_indexes.begin();

              vk::MemoryAllocateInfo allocate_info;
              allocate_info.memoryTypeIndex = host_vis_idx;
              allocate_info.allocationSize = size;

              return D.allocateMemory(allocate_info);
          }()),
          m_buffer([&]() {
              vk::BufferCreateInfo create_info;
              create_info.usage = vk::BufferUsageFlagBits::eTransferDst;
              create_info.size = size;
              return D.createBuffer(create_info);
          }()),
          m_size(size)
    {
        m_buffer.bindMemory(m_mem, 0);
        m_data_map = reinterpret_cast<std::byte *>(m_mem.mapMemory(0, m_size));
    }

    vk::raii::Buffer &buffer()
    {
        return m_buffer;
    }

    std::span<const std::byte> data()
    {
        return std::span<const std::byte>{m_data_map, m_size};
    }

  private:
    vk::raii::DeviceMemory m_mem;
    vk::raii::Buffer m_buffer;
    uint64_t m_size;
    std::byte *m_data_map = nullptr;
};

namespace {
const char *grayscale_pixel(uint8_t val)
{
    constexpr const char *lut[] = {" ", "▁", "▂", "▃", "▄", "▅", "▇", "█"};
    constexpr size_t lut_size = sizeof(lut) / sizeof(lut[0]);

    size_t index = val;
    index *= lut_size;
    index /= std::numeric_limits<decltype(val)>::max();
    index = std::clamp(index, 0ul, lut_size - 1);
    return lut[index];
}
} // namespace

int Application::main()
{
    auto ctx_p = std::make_shared<vk::raii::Context>();
    app_share_lifetime(ctx_p);
    vk::raii::Context &ctx = *ctx_p;

    auto VKI_msgr_p = std::make_shared<Messenger>("VKI");
    app_share_lifetime(VKI_msgr_p);
    auto &VKI_msgr = *VKI_msgr_p;
    auto msgr_ci = SimpleVulkanObjects::make_verbose_messenger_ci(
        &VKI_msgr,
        [](vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
           vk::DebugUtilsMessageTypeFlagsEXT /*messageTypes*/,
           const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
           void *pUserData) -> vk::Bool32 {
            Messenger &m = *reinterpret_cast<Messenger *>(pUserData);

            if (messageSeverity == decltype(messageSeverity)::eError) {
                std::string message = pCallbackData->pMessage;
                throw std::runtime_error(std::move(message));
            }

            m.message(pCallbackData->pMessage);
            return false;
        });

    std::shared_ptr<vk::raii::Instance> VKI = [&ctx, &VKI_msgr, &msgr_ci]() {
        vk::InstanceCreateInfo in_ci{};
        auto exts_avail = [&ctx]() {
            std::unordered_set<std::string> output;
            auto exts_arr = ctx.enumerateInstanceExtensionProperties();
            for (auto &ext : exts_arr) {
                output.emplace(ext.extensionName);
            }
            return output;
        }();
        auto layers_avail = [&ctx] {
            std::unordered_set<std::string> output;
            auto layers_arr = ctx.enumerateInstanceLayerProperties();
            for (auto &layer : layers_arr) {
                output.emplace(layer.layerName);
            }
            return output;
        }();

        {
            std::vector<std::string> exten_strings;
            for (auto &ext : exts_avail) {
                exten_strings.push_back(ext);
            }
            std::ranges::sort(exten_strings);

            format_table(
                "Extentions", exten_strings, [&VKI_msgr](std::string_view msg) {
                    VKI_msgr.message(msg);
                });
        }

        in_ci.setPNext(&msgr_ci);

        std::vector<const char *> exts;
        auto add_ext_if_avail = [&exts_avail, &exts](const char *ext_cstr) {
            if (!exts_avail.contains(ext_cstr)) {
                std::cout << std::format(
                    "[WARN] missing extension \"{}\"\n", ext_cstr);
                return;
            }
            exts.push_back(ext_cstr);
        };
        add_ext_if_avail("VK_EXT_debug_utils");

        std::vector<const char *> layers;
        auto add_layer_if_avail = [&layers_avail,
                                   &layers](const char *layer_cstr) {
            if (!layers_avail.contains(layer_cstr)) {
                std::cout << std::format(
                    "[WARN] missing layer \"{}\"\n", layer_cstr);
                return;
            }
            layers.push_back(layer_cstr);
        };
        add_layer_if_avail("VK_LAYER_KHRONOS_validation");

        in_ci.setPEnabledExtensionNames(exts);
        in_ci.setPEnabledLayerNames(layers);

        return std::make_shared<vk::raii::Instance>(ctx.createInstance(in_ci));
    }();
    app_share_lifetime(VKI);

    vk::raii::DebugUtilsMessengerEXT msgr = [&]() {
        return VKI->createDebugUtilsMessengerEXT(msgr_ci);
    }();

    DeviceWrap D = [&VKI]() {
        std::optional<vk::raii::PhysicalDevice> output_phys;
        std::vector<vk::raii::PhysicalDevice> devs =
            VKI->enumeratePhysicalDevices();
        for (size_t idx = 0; idx < devs.size(); idx++) {
            auto &dev = devs[idx];

            auto props = dev.getProperties();

            if (idx == device_idx) {
                std::cout << std::format(
                    "Selected phys dev #{} - \"{}\"\n",
                    idx,
                    props.deviceName.data());
                output_phys = std::move(dev);
                break;
            }
        }

        vk::raii::PhysicalDevice &phys_dev = output_phys.value();

        auto q_fams = phys_dev.getQueueFamilyProperties();
        std::optional<size_t> graph_q_fam_idx_op;
        for (size_t idx = 0; idx < q_fams.size(); idx++) {
            auto &q = q_fams[idx];
            auto q_fam_graphics = q.queueFlags & vk::QueueFlagBits::eGraphics;
            if (q_fam_graphics != vk::QueueFlagBits::eGraphics) {
                continue;
            }

            std::cout << std::format(
                "Found graph queue family: idx: {}, flags: {} nb_queues: {}\n",
                idx,
                vk::to_string(q.queueFlags),
                q.queueCount);
            graph_q_fam_idx_op = idx;
            break;
        }

        size_t graph_q_fam_idx = graph_q_fam_idx_op.value();

        std::array<vk::DeviceQueueCreateInfo, 1> q_cis{};
        q_cis[0].queueFamilyIndex = graph_q_fam_idx;
        q_cis[0].queueCount = q_fams[graph_q_fam_idx].queueCount;
        std::vector<float> priors;
        priors.resize(q_cis[0].queueCount, 1.0);
        q_cis[0].setQueuePriorities(priors);

        std::vector<const char *> dev_extst;

        vk::DeviceCreateInfo dev_ci{};
        dev_ci.setQueueCreateInfos(q_cis);
        dev_ci.setPEnabledExtensionNames(dev_extst);
        auto device = phys_dev.createDevice(dev_ci);

        std::vector<vk::raii::Queue> g_queues;
        g_queues.reserve(q_cis[0].queueCount);
        for (size_t greph_q_dix = 0; greph_q_dix < q_cis[0].queueCount;
             greph_q_dix++) {
            g_queues.push_back(
                device.getQueue(q_cis[0].queueFamilyIndex, greph_q_dix));
        }

        do {
            auto exts = phys_dev.enumerateDeviceExtensionProperties();
            std::vector<std::string> exten_strings;
            for (auto &ext : exts) {
                exten_strings.push_back(ext.extensionName.data());
            }

            std::string dev_name{phys_dev.getProperties().deviceName.data()};

            format_table(
                dev_name + " extentions",
                exten_strings,
                [](std::string_view msg) { std::cout << msg << '\n'; });

        } while (false);

        return DeviceWrap{
            std::move(device), std::move(g_queues), std::move(phys_dev)};
    }();

    DeviceLocalImage image{D.phy(), D.get(), {10 * 10, 4 * 10}};
    HostVisMemBuffer image_dst_buffer{
        D.phy(), D.get(), image.width() * image.height() * 4};

    auto render_pass =
        SimpleVulkanObjects::make_render_pass(D.get(), image.format());

    vk::raii::PipelineLayout pipeline_layout = [&D]() {
        vk::PipelineLayoutCreateInfo p_layout_ci{};

        return D->createPipelineLayout(p_layout_ci);
    }();

    vk::raii::Pipeline pipeline = SimpleVulkanObjects::make_pipeline(
        D.get(),
        render_pass,
        pipeline_layout,
        get_shader_vert_trig(),
        get_shader_frag_trig());

    vk::raii::Framebuffer fbm = [&D, &render_pass, &image]() {
        vk::FramebufferCreateInfo fbm_ci{};
        fbm_ci.renderPass = render_pass;
        std::array<vk::ImageView, 1> attachments{image.view()};
        fbm_ci.setAttachments(attachments);
        fbm_ci.width = image.width();
        fbm_ci.height = image.height();
        fbm_ci.layers = 1;
        return D->createFramebuffer(fbm_ci);
    }();

    vk::CommandPoolCreateInfo cmdp_ci{};
    auto cmdp = D->createCommandPool(cmdp_ci);

    vk::CommandBufferAllocateInfo cmdb_ci{};
    cmdb_ci.commandPool = cmdp;
    cmdb_ci.commandBufferCount = 2;
    auto cmd_buffers = D->allocateCommandBuffers(cmdb_ci);

    auto &main_cmdb = cmd_buffers.at(0);
    vk::RenderPassBeginInfo rp_i{};
    rp_i.renderPass = render_pass;
    rp_i.framebuffer = fbm;
    std::array<vk::ClearValue, 1> cls{};
    cls[0].color = {0, 0, 0, 0};
    rp_i.renderArea.extent = vk::Extent2D{image.width(), image.height()};
    rp_i.setClearValues(cls);

    auto fence = [&D]() {
        vk::FenceCreateInfo f_ci{};
        return D->createFence(f_ci);
    }();

    main_cmdb.begin({});
    std::array<vk::Viewport, 1> viewports;
    std::array<vk::Rect2D, 1> scizors;
    scizors[0].extent.height = image.height();
    scizors[0].extent.width = image.width();
    viewports[0].height = image.height();
    viewports[0].width = image.width();
    main_cmdb.setViewport(0, viewports);
    main_cmdb.setScissor(0, scizors);
    main_cmdb.beginRenderPass(rp_i, vk::SubpassContents::eInline);
    main_cmdb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    main_cmdb.draw(3 * 1, 2, 0, 0);
    main_cmdb.endRenderPass();
    main_cmdb.end();

    vk::BufferImageCopy reg{};
    reg.imageExtent = vk::Extent3D{image.width(), image.height(), 1};
    reg.bufferRowLength = image.width();
    reg.bufferImageHeight = image.height();

    vk::ImageSubresourceRange r{};
    r.aspectMask = vk::ImageAspectFlagBits::eColor;
    r.levelCount = 1;
    r.layerCount = 1;

    auto &srs = reg.imageSubresource;
    srs.baseArrayLayer = r.baseArrayLayer;
    srs.aspectMask = r.aspectMask;
    srs.layerCount = r.layerCount;
    srs.baseArrayLayer = r.baseArrayLayer;
    srs.mipLevel = r.baseMipLevel;

    auto &copy_cmdb = cmd_buffers.at(1);
    copy_cmdb.begin({});
    copy_cmdb.copyImageToBuffer(
        image.image(),
        vk::ImageLayout::eGeneral,
        image_dst_buffer.buffer(),
        reg);
    copy_cmdb.end();

    std::vector<vk::CommandBuffer> bufs;

    while (!terminate_requested()) {
        vk::Result wait_status = vk::Result::eTimeout;
        uint64_t fence_wait_counter = 0;

        bufs.clear();
        for (auto &b : cmd_buffers) {
            bufs.emplace_back(*b);
        }

        std::array<vk::SubmitInfo, 2> submits{};
        submits[0].setCommandBuffers(bufs.at(0));
        D.main_gq().submit(submits[0], fence);

        fence_wait_counter = 0;
        wait_status = vk::Result::eTimeout;
        while (wait_status == vk::Result::eTimeout) {
            wait_status = D->waitForFences({fence}, true, 0);
            fence_wait_counter++;
        }
        D->resetFences({fence});
        std::cout << "Draw taken " << fence_wait_counter << " cycles\n";

        submits[1].setCommandBuffers(bufs.at(1));
        vk::PipelineStageFlags m{};
        m |= vk::PipelineStageFlagBits::eAllCommands;
        submits[1].pWaitDstStageMask = &m;
        D.main_gq().submit(submits[1], fence);

        fence_wait_counter = 0;
        wait_status = vk::Result::eTimeout;
        while (wait_status == vk::Result::eTimeout) {
            wait_status = D->waitForFences({fence}, true, 0);
            fence_wait_counter++;
        }
        D->resetFences({fence});
        std::cout << "Copy taken " << fence_wait_counter << " cycles\n";

        auto data = image_dst_buffer.data();
        auto h = image.height();
        auto w = image.width();

        auto print_fbm_ascii = [&](uint8_t color_component_idx) {
            std::cout << "+-";
            for (size_t x = 0; x < w; x++) {
                std::cout << '-';
            }
            std::cout << "-+";
            std::cout << '\n';

            for (size_t y = 0; y < h; y++) {
                std::cout << "| ";
                for (size_t x = 0; x < w; x++) {
                    size_t pxl_idx = (y * w + x) * 4;
                    uint32_t val = std::to_integer<uint8_t>(
                        data[pxl_idx + color_component_idx]);
                    std::cout << grayscale_pixel(val);
                }
                std::cout << " |";
                std::cout << '\n';
            }

            std::cout << "+-";
            for (size_t x = 0; x < w; x++) {
                std::cout << '-';
            }
            std::cout << "-+";
            std::cout << '\n';
        };

        std::cout << "[RED]\n";
        print_fbm_ascii(0);
        std::cout << "[GREEN]\n";
        print_fbm_ascii(1);
        std::cout << "[BLUE]\n";
        print_fbm_ascii(2);
        std::cout << "[ALPHA]\n";
        print_fbm_ascii(3);
        break;
    }

    return EXIT_SUCCESS;
}
