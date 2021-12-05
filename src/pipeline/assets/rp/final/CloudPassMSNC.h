//
// Created by pekopeko on 29.11.2021.
//

#ifndef MB_CLOUDS_CLOUDPASSMSNC_H
#define MB_CLOUDS_CLOUDPASSMSNC_H

#include <pipeline/RenderPass.h>
#include <shaders/texture.h>

class CloudPassMSNC : public RenderPass {
public:
    explicit CloudPassMSNC(const std::string &output);

    void execute() override;
private:
    texture _blueNoiseTex;
};


#endif //MB_CLOUDS_CLOUDPASSMSNC_H
