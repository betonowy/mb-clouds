//
// Created by pekopeko on 18.10.2021.
//

#include "AdaptivePipeline.h"

#include <pipeline/assets/rp/tests/BackgroundPass.h>
#include <pipeline/assets/rp/tests/CloudPass.h>
#include <pipeline/assets/rp/tests/PostPass.h>
#include <pipeline/assets/rp/tests/PersistencePass.h>
#include <pipeline/assets/rp/tests/AbsDiffPass.h>
#include <pipeline/assets/rp/tests/BlurHorizontalPass.h>
#include <pipeline/assets/rp/tests/BlurVerticalPass.h>

AdaptivePipeline::AdaptivePipeline()
        : Pipeline({
                           std::make_shared<BackgroundPass>("fbBackgroundColor"),
                           std::make_shared<CloudPass>("fbCloudColor", "fbBlurY"),
                           std::make_shared<AbsDiffPass>("fbCloudColor", "fbCloudCopy", "absDiffColor"),
                           std::make_shared<PersistencePass>("fbCloudColor", "fbCloudCopy"),
                           std::make_shared<BlurHorizontalPass>("absDiffColor", "fbBlurX"),
                           std::make_shared<BlurVerticalPass>("fbBlurX", "fbBlurY"),
                           std::make_shared<PostPass>("fbBackgroundColor", "fbCloudColor"),
                   }) {}
