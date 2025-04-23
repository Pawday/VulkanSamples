#include <array>

#include <cstdint>
#include <span>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>
#include <vulkan/vulkan_to_string.hpp>

#include "SimpleVulkanObjects.hh"


vk::raii::RenderPass SimpleVulkanObjects::make_render_pass(
    vk::raii::Device &D, vk::Format format)
{
    std::array<vk::AttachmentDescription, 1> attache_infos{};
    attache_infos[0].format = format;
    attache_infos[0].samples = vk::SampleCountFlagBits::e1;
    attache_infos[0].loadOp = vk::AttachmentLoadOp::eClear;
    attache_infos[0].storeOp = vk::AttachmentStoreOp::eStore;
    attache_infos[0].finalLayout = vk::ImageLayout::eGeneral;

    std::array<vk::AttachmentReference, 1> attach_refs{};
    attach_refs[0].layout = vk::ImageLayout::eColorAttachmentOptimal;

    std::array<vk::SubpassDescription, 1> subpasses{};
    subpasses[0].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpasses[0].setColorAttachments(attach_refs);

    vk::RenderPassCreateInfo render_pass_ci{};
    render_pass_ci.setSubpasses(subpasses);
    render_pass_ci.setAttachments(attache_infos);
    return D.createRenderPass(render_pass_ci);
};

vk::raii::Pipeline SimpleVulkanObjects::make_pipeline(
    vk::raii::Device &D,
    vk::raii::RenderPass &render_pass,
    vk::raii::PipelineLayout &pipeline_layout,
    std::span<uint32_t> v_shader_code,
    std::span<uint32_t> f_shader_code)
{
    vk::ShaderModuleCreateInfo v_shader_ci;
    v_shader_ci.setCode(v_shader_code);
    auto v_shader = D.createShaderModule(v_shader_ci);
    vk::ShaderModuleCreateInfo f_shader_ci;
    f_shader_ci.setCode(f_shader_code);
    auto f_shader = D.createShaderModule(f_shader_ci);

    std::array<vk::PipelineShaderStageCreateInfo, 2> stages;
    stages[0].stage = vk::ShaderStageFlagBits::eVertex;
    stages[0].module = v_shader;
    stages[0].pName = "main";

    stages[1].stage = vk::ShaderStageFlagBits::eFragment;
    stages[1].module = f_shader;
    stages[1].pName = "main";

    vk::GraphicsPipelineCreateInfo pipeline_ci{};
    pipeline_ci.layout = pipeline_layout;
    pipeline_ci.renderPass = render_pass;
    pipeline_ci.setStages(stages);

    vk::PipelineVertexInputStateCreateInfo vertext_input_state{};
    pipeline_ci.pVertexInputState = &vertext_input_state;

    vk::PipelineInputAssemblyStateCreateInfo in_asm_state{};
    in_asm_state.topology = vk::PrimitiveTopology::eTriangleList;
    pipeline_ci.pInputAssemblyState = &in_asm_state;

    vk::PipelineMultisampleStateCreateInfo m_state{};
    pipeline_ci.pMultisampleState = &m_state;

    vk::PipelineRasterizationStateCreateInfo raster_state{};
    raster_state.polygonMode = vk::PolygonMode::eFill;
    raster_state.lineWidth = 1.0;
    pipeline_ci.pRasterizationState = &raster_state;

    std::array<vk::PipelineColorBlendAttachmentState, 1> blend_attachments{};
    blend_attachments[0].blendEnable = false;
    {
        using C = vk::ColorComponentFlagBits;
        blend_attachments[0].colorWriteMask |= C::eR;
        blend_attachments[0].colorWriteMask |= C::eG;
        blend_attachments[0].colorWriteMask |= C::eB;
        blend_attachments[0].colorWriteMask |= C::eA;
    }
    vk::PipelineColorBlendStateCreateInfo blend_state{};
    blend_state.setAttachments(blend_attachments);
    pipeline_ci.pColorBlendState = &blend_state;

    std::array<vk::DynamicState, 2> dyn_state_types{
        vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dyn_state{};
    dyn_state.setDynamicStates(dyn_state_types);
    pipeline_ci.pDynamicState = &dyn_state;

    vk::PipelineViewportStateCreateInfo viewport_state{};
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;
    pipeline_ci.pViewportState = &viewport_state;

    return D.createGraphicsPipeline(nullptr, pipeline_ci);
};
