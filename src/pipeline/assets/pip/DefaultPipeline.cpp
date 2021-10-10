//
// Created by pekopeko on 06.09.2021.
//

#include "DefaultPipeline.h"

#include <pipeline/assets/rp/BackgroundPass.h>
#include <pipeline/assets/rp/CloudPass.h>
#include <pipeline/assets/rp/PostPass.h>

DefaultPipeline::DefaultPipeline()
        : Pipeline({
                           std::make_shared<BackgroundPass>("fbBackgroundColor"),
                           std::make_shared<CloudPass>("fbCloudColor"),
                           std::make_shared<PostPass>("fbBackgroundColor", "fbCloudColor"),
                   }) {}
