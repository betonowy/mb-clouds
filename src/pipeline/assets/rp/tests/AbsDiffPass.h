//
// Created by pekopeko on 07.10.2021.
//

#ifndef MB_CLOUDS_ABSDIFFPASS_H
#define MB_CLOUDS_ABSDIFFPASS_H

#include <pipeline/RenderPass.h>

class AbsDiffPass : public RenderPass {
public:
    AbsDiffPass(std::string input_1, std::string input_2);

    AbsDiffPass(std::string input_1, std::string input_2, const std::string& output);

    void execute() override;

private:
    std::string _input_1, _input_2;
    bool _hasCustomFb;
};


#endif //MB_CLOUDS_ABSDIFFPASS_H
