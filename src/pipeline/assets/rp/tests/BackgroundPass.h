//
// Created by pekopeko on 08.09.2021.
//

#ifndef MB_CLOUDS_BACKGROUNDPASS_H
#define MB_CLOUDS_BACKGROUNDPASS_H

#include <pipeline/RenderPass.h>

class BackgroundPass : public RenderPass {
public:
    BackgroundPass(const std::string &output);

    void execute() override;
};


#endif //MB_CLOUDS_BACKGROUNDPASS_H
