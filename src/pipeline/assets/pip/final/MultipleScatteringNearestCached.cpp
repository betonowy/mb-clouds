//
// Created by pekopeko on 29.11.2021.
//

#include "MultipleScatteringNearestCached.h"

#include <pipeline/assets/rp/tests/BackgroundPass.h>
#include <pipeline/assets/rp/final/CloudPassMSNC.h>
#include <pipeline/assets/rp/tests/PostPass.h>
#include <pipeline/assets/rp/tests/BlurHorizontalPass.h>
#include <pipeline/assets/rp/tests/BlurVerticalPass.h>

MultipleScatteringNearestCached::MultipleScatteringNearestCached()
        : Pipeline({std::make_shared<BackgroundPass>("fbBackgroundColor"),
                    std::make_shared<CloudPassMSNC>("fbCloudColor"),
                    std::make_shared<PostPass>("fbBackgroundColor", "fbCloudColor", "fbPostColor"),
                    std::make_shared<BlurHorizontalPass>("fbPostColor", "fbBlurX"),
                    std::make_shared<BlurVerticalPass>("fbBlurX")}) {}
