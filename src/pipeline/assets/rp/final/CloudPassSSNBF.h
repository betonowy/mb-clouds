//
// Created by pekopeko on 29.11.2021.
//

#ifndef MB_CLOUDS_CLOUDPASSSSNBF_H
#define MB_CLOUDS_CLOUDPASSSSNBF_H

#include <pipeline/RenderPass.h>
#include <shaders/texture.h>

class CloudPassSSNBF : public RenderPass {
public:
    explicit CloudPassSSNBF(const std::string &output);

    void execute() override;
private:
    texture _blueNoiseTex;
};


#endif //MB_CLOUDS_CLOUDPASSSSNBF_H
