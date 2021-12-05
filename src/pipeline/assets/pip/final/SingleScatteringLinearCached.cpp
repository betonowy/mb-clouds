//
// Created by pekopeko on 26.11.2021.
//

#include "SingleScatteringLinearCached.h"

#include <pipeline/assets/rp/tests/BackgroundPass.h>
#include <pipeline/assets/rp/final/CloudPassSSLC.h>
#include <pipeline/assets/rp/tests/PostPass.h>
#include <pipeline/assets/rp/tests/BlurHorizontalPass.h>
#include <pipeline/assets/rp/tests/BlurVerticalPass.h>

SingleScatteringLinearCached::SingleScatteringLinearCached()
        : Pipeline({std::make_shared<BackgroundPass>("fbBackgroundColor"),
                    std::make_shared<CloudPassSSLC>("fbCloudColor"),
                    std::make_shared<PostPass>("fbBackgroundColor", "fbCloudColor", "fbPostColor"),
                    std::make_shared<BlurHorizontalPass>("fbPostColor", "fbBlurX"),
                    std::make_shared<BlurVerticalPass>("fbBlurX")}) {}
