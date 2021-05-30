//
// Created by pekopeko on 29.05.2021.
//

#include "shader.h"
#include <util/file/binaryFile.h>
#include <init.h>
#include <util/misc.h>

mb::shader::shader(const std::string &path) {
    auto file = binaryFile(path.c_str());

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = file.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(file.data());

    auto init = init::GetInstance();

    if (VK_SUCCESS != vkCreateShaderModule(init->GetLogicalDevice(), &createInfo, nullptr, &_vulcanShaderModule))
        misc::exception("failed to create shader module");
}

mb::shader::~shader() {
    auto init = init::GetInstance();
    vkDestroyShaderModule(init->GetLogicalDevice(), _vulcanShaderModule, nullptr);
}

VkShaderModule & mb::shader::GetShaderModule() { return _vulcanShaderModule; }
