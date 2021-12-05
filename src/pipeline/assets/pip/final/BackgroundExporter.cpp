//
// Created by pekopeko on 02.12.2021.
//

#include "BackgroundExporter.h"

#include <pipeline/assets/rp/tests/BackgroundPassExport.h>
#include <pipeline/assets/rp/tests/PostPassExport.h>

BackgroundExporter::BackgroundExporter()
        : Pipeline({std::make_shared<BackgroundPassExport>("fbBackgroundColor"),
                    std::make_shared<PostPassExport>("fbBackgroundColor")}) {}
