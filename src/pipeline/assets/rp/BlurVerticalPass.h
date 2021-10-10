//
// Created by pekopeko on 08.09.2021.
//

#ifndef MB_CLOUDS_BLURVERTICALPASS_H
#define MB_CLOUDS_BLURVERTICALPASS_H

#include <pipeline/RenderPass.h>

class BlurVerticalPass : public RenderPass{
public:
    explicit BlurVerticalPass(std::string input);

    BlurVerticalPass(std::string input, const std::string &output);

    void execute() override;

private:
    std::string _input;
    bool _hasCustomFb;
};


#endif //MB_CLOUDS_BLURVERTICALPASS_H
