//
// Created by pekopeko on 29.11.2021.
//

#include "MultipleScatteringLinearCached.h"

#include <pipeline/assets/rp/tests/BackgroundPass.h>
#include <pipeline/assets/rp/final/CloudPassMSLC.h>
#include <pipeline/assets/rp/tests/PostPass.h>
#include <pipeline/assets/rp/tests/BlurHorizontalPass.h>
#include <pipeline/assets/rp/tests/BlurVerticalPass.h>

MultipleScatteringLinearCached::MultipleScatteringLinearCached()
        : Pipeline({std::make_shared<BackgroundPass>("fbBackgroundColor"),
                    std::make_shared<CloudPassMSLC>("fbCloudColor"),
                    std::make_shared<PostPass>("fbBackgroundColor", "fbCloudColor", "fbPostColor"),
                    std::make_shared<BlurHorizontalPass>("fbPostColor", "fbBlurX"),
                    std::make_shared<BlurVerticalPass>("fbBlurX")}) {}
