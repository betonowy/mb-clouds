//
// Created by pekopeko on 26.11.2021.
//

#ifndef MB_CLOUDS_CLOUDPASSSSNC_H
#define MB_CLOUDS_CLOUDPASSSSNC_H

#include <pipeline/RenderPass.h>
#include <shaders/texture.h>

class CloudPassSSNC : public RenderPass {
public:
    explicit CloudPassSSNC(const std::string &output);

    void execute() override;
private:
    texture _blueNoiseTex;
};


#endif //MB_CLOUDS_CLOUDPASSSSNC_H
