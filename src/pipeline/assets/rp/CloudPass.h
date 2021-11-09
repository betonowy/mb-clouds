//
// Created by pekopeko on 05.09.2021.
//

#ifndef MB_CLOUDS_CLOUDPASS_H
#define MB_CLOUDS_CLOUDPASS_H

#include <pipeline/RenderPass.h>
#include <shaders/texture.h>

class CloudPass : public RenderPass {
public:
    void execute() override;

    explicit CloudPass(const std::string &output);

    CloudPass(const std::string &output, const std::string &noiseLayer);
private:
    texture _blueNoiseTex;

    std::optional<std::string> _noiseLayerInput;
};


#endif //MB_CLOUDS_CLOUDPASS_H
