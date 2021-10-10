//
// Created by pekopeko on 08.09.2021.
//

#ifndef MB_CLOUDS_POSTPASSFORBLUR_H
#define MB_CLOUDS_POSTPASSFORBLUR_H

#include <pipeline/RenderPass.h>

class PostPassForBlur : public RenderPass {
public:
    PostPassForBlur(std::string backgroundTargetName, std::string cloudTargetName, const std::string& outputName);

    void execute() override;

private:
    std::string _backgroundTargetName;
    std::string _cloudTargetName;
};


#endif //MB_CLOUDS_POSTPASSFORBLUR_H
