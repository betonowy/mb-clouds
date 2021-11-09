//
// Created by pekopeko on 10.10.2021.
//

#include "PipeMan.h"
#include <pipeline/assets/pip/DefaultPipeline.h>
#include <pipeline/assets/pip/BlurPipeline.h>
#include <pipeline/assets/pip/DiffShaderTest.h>
#include <pipeline/assets/pip/AdaptivePipeline.h>

PipeMan::PipeMan()
        : _pipelineMakers({
                                  {std::string("Default Pipeline"),     []() { return DefaultPipeline(); }},
                                  {std::string("Blur Pipeline"),        []() { return BlurPipeline(); }},
                                  {std::string("Diff shader test"),     []() { return DiffShaderTest(); }},
                                  {std::string("Adaptive shader test"), []() { return AdaptivePipeline(); }},
                          }) {}
