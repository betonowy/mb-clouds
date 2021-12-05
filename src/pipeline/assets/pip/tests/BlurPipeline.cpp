//
// Created by pekopeko on 08.09.2021.
//

#include "BlurPipeline.h"

#include <pipeline/assets/rp/tests/BackgroundPass.h>
#include <pipeline/assets/rp/tests/CloudPass.h>
#include <pipeline/assets/rp/tests/PostPass.h>
#include <pipeline/assets/rp/tests/BlurHorizontalPass.h>
#include <pipeline/assets/rp/tests/BlurVerticalPass.h>

BlurPipeline::BlurPipeline()
        : Pipeline({
                           std::make_shared<BackgroundPass>("fbBackgroundColor"),
                           std::make_shared<CloudPass>("fbCloudColor"),
                           std::make_shared<PostPass>("fbBackgroundColor", "fbCloudColor", "fbCombined"),
                           std::make_shared<BlurHorizontalPass>("fbCombined", "fbCloudBlurX"),
                           std::make_shared<BlurVerticalPass>("fbCloudBlurX"),
                   }) {}
