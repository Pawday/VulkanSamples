#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <cstdlib>

#include <vulkan/vulkan_raii.hpp>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "wayland/WaylandContext.hh"

#include "FormatTools.hh"
#include "Messenger.hh"

int main()
try {
    vk::raii::Context ctx;

    Messenger VKI_msgr{"VKI"};

    vk::DebugUtilsMessengerCreateInfoEXT msgr_ci = [&VKI_msgr]() {
        vk::DebugUtilsMessengerCreateInfoEXT output{};

        vk::PFN_DebugUtilsMessengerCallbackEXT callback_t = []

            (vk::DebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
             vk::DebugUtilsMessageTypeFlagsEXT /*messageTypes*/,
             const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
             void *pUserData) -> vk::Bool32 {
            Messenger &m = *reinterpret_cast<Messenger *>(pUserData);

            m.message(pCallbackData->pMessage);

            return false;
        };

        vk::DebugUtilsMessageSeverityFlagsEXT sever;
        using SevT = vk::DebugUtilsMessageSeverityFlagBitsEXT;
        sever |= SevT::eError;
        sever |= SevT::eWarning;
        sever |= SevT::eInfo;
        sever |= SevT::eVerbose;
        output.setMessageSeverity(sever);

        vk::DebugUtilsMessageTypeFlagsEXT types;
        using TypeF = vk::DebugUtilsMessageTypeFlagBitsEXT;
        types |= TypeF::eValidation;
        types |= TypeF::eDeviceAddressBinding;
        types |= TypeF::eGeneral;
        types |= TypeF::ePerformance;
        output.setMessageType(types);

        output.setPUserData(&VKI_msgr);
        output.setPfnUserCallback(callback_t);

        return output;
    }();

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

        return std::make_shared<vk::raii::Instance>(ctx.createInstance(in_ci));
    }();

    WaylandContext c{VKI};

    auto win = c.create_window();

} catch (std::exception &e) {
    std::cout << e.what() << '\n';
    return EXIT_FAILURE;
}
