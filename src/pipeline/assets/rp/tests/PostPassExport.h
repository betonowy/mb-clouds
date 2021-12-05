//
// Created by pekopeko on 02.12.2021.
//

#ifndef MB_CLOUDS_POSTPASSEXPORT_H
#define MB_CLOUDS_POSTPASSEXPORT_H

#include <pipeline/Pipeline.h>
#include <shaders/texture.h>

class PostPassExport : public RenderPass {
public:
    PostPassExport(std::string backgroundTargetName);

    void execute() override;

private:
    std::string _backgroundTargetName;
    texture _blueNoiseTex;
};


#endif //MB_CLOUDS_POSTPASSEXPORT_H
