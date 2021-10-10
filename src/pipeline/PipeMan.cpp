//
// Created by pekopeko on 10.10.2021.
//

#include "PipeMan.h"
#include <pipeline/assets/pip/DefaultPipeline.h>
#include <pipeline/assets/pip/BlurPipeline.h>
#include <pipeline/assets/pip/DiffShaderTest.h>

PipeMan::PipeMan()
        : _pipelineMakers({
                                  {std::string("Default Pipeline"), std::make_shared<DefaultPipeline>},
                                  {std::string("Blur Pipeline"), std::make_shared<BlurPipeline>},
                                  {std::string("Diff shader test"), std::make_shared<DiffShaderTest>},
                          }) {}
