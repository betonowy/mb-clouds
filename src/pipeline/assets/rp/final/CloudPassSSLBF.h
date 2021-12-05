//
// Created by pekopeko on 29.11.2021.
//

#ifndef MB_CLOUDS_CLOUDPASSSSLBF_H
#define MB_CLOUDS_CLOUDPASSSSLBF_H

#include <pipeline/RenderPass.h>
#include <shaders/texture.h>

class CloudPassSSLBF : public RenderPass {
public:
    explicit CloudPassSSLBF(const std::string &output);

    void execute() override;

private:
    texture _blueNoiseTex;
};


#endif //MB_CLOUDS_CLOUDPASSSSLBF_H
