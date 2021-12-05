//
// Created by pekopeko on 26.11.2021.
//

#include "SingleScatteringNearestCached.h"

#include <pipeline/assets/rp/tests/BackgroundPass.h>
#include <pipeline/assets/rp/final/CloudPassSSNC.h>
#include <pipeline/assets/rp/tests/PostPass.h>
#include <pipeline/assets/rp/tests/BlurHorizontalPass.h>
#include <pipeline/assets/rp/tests/BlurVerticalPass.h>

SingleScatteringNearestCached::SingleScatteringNearestCached()
        : Pipeline({std::make_shared<BackgroundPass>("fbBackgroundColor"),
                    std::make_shared<CloudPassSSNC>("fbCloudColor"),
                    std::make_shared<BlurHorizontalPass>("fbCloudColor", "fbBlurX"),
                    std::make_shared<BlurVerticalPass>("fbBlurX", "fbBlurY"),
                    std::make_shared<PostPass>("fbBackgroundColor", "fbBlurY")}) {}
