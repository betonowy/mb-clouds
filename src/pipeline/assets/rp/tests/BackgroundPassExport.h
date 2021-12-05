//
// Created by pekopeko on 04.12.2021.
//

#ifndef MB_CLOUDS_BACKGROUNDPASSEXPORT_H
#define MB_CLOUDS_BACKGROUNDPASSEXPORT_H

#include <pipeline/RenderPass.h>

class BackgroundPassExport : public RenderPass {
public:
    BackgroundPassExport(const std::string &output);

    void execute() override;

private:
    std::string _input;
};

#endif //MB_CLOUDS_BACKGROUNDPASSEXPORT_H
