//
// Created by pekopeko on 29.05.2021.
//

#include "shader.h"
#include <util/file/binaryFile.h>
#include <init.h>
#include <util/misc.h>

mb::shader::shader(const std::string &path) {
    auto file = binaryFile(path.c_str());

    vk::ShaderModuleCreateInfo createInfo{
            .sType = vk::StructureType::eShaderModuleCreateInfo,
            .codeSize = file.size(),
            .pCode = reinterpret_cast<const uint32_t *>(file.data())
    };

    auto init = init::GetInstance();

    if (vk::Result::eSuccess != init->GetLogicalDevice().createShaderModule(&createInfo, nullptr, &_vulcanShaderModule))
        misc::exception("failed to create shader module");
}

mb::shader::~shader() {
    auto init = init::GetInstance();
    init->GetLogicalDevice().destroyShaderModule(_vulcanShaderModule, nullptr);
}

vk::ShaderModule &mb::shader::GetShaderModule() { return _vulcanShaderModule; }
