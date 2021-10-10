//
// Created by pekopeko on 08.09.2021.
//

#ifndef MB_CLOUDS_BLURHORIZONTALPASS_H
#define MB_CLOUDS_BLURHORIZONTALPASS_H

#include <pipeline/RenderPass.h>

class BlurHorizontalPass : public RenderPass {
public:
    BlurHorizontalPass(std::string input, const std::string &output);

    void execute() override;

private:
    std::string _input;
};


#endif //MB_CLOUDS_BLURHORIZONTALPASS_H
