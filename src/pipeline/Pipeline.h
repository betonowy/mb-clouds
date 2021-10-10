//
// Created by pekopeko on 05.09.2021.
//

#ifndef MB_CLOUDS_PIPELINE_H
#define MB_CLOUDS_PIPELINE_H

#include <memory>
#include <vector>
#include "RenderPass.h"

class Pipeline {
public:
    Pipeline(std::initializer_list<std::shared_ptr<RenderPass>> passes);

    void execute();

    void resize(glm::ivec2 resolution);

private:
    std::vector<std::shared_ptr<RenderPass>> _passes;

    glm::ivec2 _lastResolution{800, 600};
};


#endif //MB_CLOUDS_PIPELINE_H
