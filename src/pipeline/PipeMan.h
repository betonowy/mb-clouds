//
// Created by pekopeko on 10.10.2021.
//

#ifndef MB_CLOUDS_PIPEMAN_H
#define MB_CLOUDS_PIPEMAN_H

#include <pipeline/pipeline.h>
#include <functional>
#include <memory>
#include <string>

class PipeMan {
public:
    PipeMan();

    [[nodiscard]] const auto &getMakers() const& { return _pipelineMakers; }

private:
    std::vector<std::pair<std::string, std::function<Pipeline()>>> _pipelineMakers;
};


#endif //MB_CLOUDS_PIPEMAN_H
