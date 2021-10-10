//
// Created by pekopeko on 06.09.2021.
//

#ifndef MB_CLOUDS_POSTPASS_H
#define MB_CLOUDS_POSTPASS_H

#include <pipeline/RenderPass.h>

class PostPass : public RenderPass {
public:
    PostPass(std::string backgroundTargetName, std::string cloudTargetName);

    PostPass(std::string backgroundTargetName, std::string cloudTargetName, const std::string &output);

    void execute() override;

private:
    std::string _backgroundTargetName;
    std::string _cloudTargetName;
    bool _hasCustomFb;
};


#endif //MB_CLOUDS_POSTPASS_H
