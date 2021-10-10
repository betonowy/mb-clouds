//
// Created by pekopeko on 08.09.2021.
//

#include "BlurPipeline.h"

#include <pipeline/assets/rp/BackgroundPass.h>
#include <pipeline/assets/rp/CloudPass.h>
#include <pipeline/assets/rp/PostPassForBlur.h>
#include <pipeline/assets/rp/BlurHorizontalPass.h>
#include <pipeline/assets/rp/BlurVerticalPass.h>

BlurPipeline::BlurPipeline()
        : Pipeline({
                           std::make_shared<BackgroundPass>("fbBackgroundColor"),
                           std::make_shared<CloudPass>("fbCloudColor"),
                           std::make_shared<PostPassForBlur>("fbBackgroundColor", "fbCloudColor", "fbCombined"),
                           std::make_shared<BlurHorizontalPass>("fbCombined", "fbCloudBlurX"),
                           std::make_shared<BlurVerticalPass>("fbCloudBlurX"),
                   }) {}
