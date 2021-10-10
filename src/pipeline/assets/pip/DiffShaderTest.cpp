//
// Created by pekopeko on 02.10.2021.
//

#include "DiffShaderTest.h"

#include <pipeline/assets/rp/BackgroundPass.h>
#include <pipeline/assets/rp/CloudPass.h>
#include <pipeline/assets/rp/PostPass.h>
#include <pipeline/assets/rp/PersistencePass.h>
#include <pipeline/assets/rp/AbsDiffPass.h>
#include <pipeline/assets/rp/BlurHorizontalPass.h>
#include <pipeline/assets/rp/BlurVerticalPass.h>

DiffShaderTest::DiffShaderTest()
        : Pipeline({
                           std::make_shared<BackgroundPass>("fbBackgroundColor"),
                           std::make_shared<CloudPass>("fbCloudColor"),
                           std::make_shared<AbsDiffPass>("fbCloudColor", "fbCloudCopy", "absDiffColor"),
                           std::make_shared<PersistencePass>("fbCloudColor", "fbCloudCopy"),
                           std::make_shared<BlurHorizontalPass>("absDiffColor", "fbBlurX"),
                           std::make_shared<BlurVerticalPass>("fbBlurX", "fbBlurY"),
                           std::make_shared<PostPass>("fbBackgroundColor", "fbBlurY"),
                   }) {}
