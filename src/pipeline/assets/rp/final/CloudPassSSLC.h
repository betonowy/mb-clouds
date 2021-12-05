//
// Created by pekopeko on 26.11.2021.
//

#ifndef MB_CLOUDS_CLOUDPASSSSLC_H
#define MB_CLOUDS_CLOUDPASSSSLC_H

#include <pipeline/RenderPass.h>
#include <shaders/texture.h>

class CloudPassSSLC : public RenderPass {
public:
    explicit CloudPassSSLC(const std::string &output);

    void execute() override;
private:
    texture _blueNoiseTex;
};


#endif //MB_CLOUDS_CLOUDPASSSSLC_H
