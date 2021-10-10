//
// Created by pekopeko on 02.10.2021.
//

#ifndef MB_CLOUDS_PERSISTENCEPASS_H
#define MB_CLOUDS_PERSISTENCEPASS_H

#include <pipeline/RenderPass.h>

class PersistencePass : public RenderPass {
public:
    PersistencePass(std::string input, const std::string &output);

    void execute() override;
private:
    std::string _input;
};


#endif //MB_CLOUDS_PERSISTENCEPASS_H
