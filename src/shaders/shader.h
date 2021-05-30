//
// Created by pekopeko on 29.05.2021.
//

#ifndef MB_CLOUDS_SHADER_H
#define MB_CLOUDS_SHADER_H

#include <util/staticStorage.h>
#include <vulkan/vulkan_core.h>

namespace mb {
    class shader : public staticStorage<shader> {
    public:
        explicit shader(const std::string &path);

        virtual ~shader();

        VkShaderModule &GetShaderModule();

    private:
        VkShaderModule _vulcanShaderModule{};
    };

}


#endif //MB_CLOUDS_SHADER_H
