//
// Created by pekopeko on 29.05.2021.
//

#ifndef MB_CLOUDS_SHADER_H
#define MB_CLOUDS_SHADER_H

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <util/staticStorage.h>
#include <vulkan/vulkan.hpp>

namespace mb {
    class shader : public staticStorage<shader> {
    public:
        explicit shader(const std::string &path);

        virtual ~shader();

        vk::ShaderModule & GetShaderModule();

    private:
        vk::ShaderModule _vulcanShaderModule{};
    };

}


#endif //MB_CLOUDS_SHADER_H
