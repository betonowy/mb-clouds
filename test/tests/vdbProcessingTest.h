//
// Created by pekopeko on 01.11.2021.
//

#ifndef MB_CLOUDS_VDBPROCESSINGTEST_H
#define MB_CLOUDS_VDBPROCESSINGTEST_H

#include <clouds/vdbOfflineIntegrator.h>
#include <tree/vdbData.h>
#include <config.h>
#include <shaders/sceneData.h>

#include <openvdb/openvdb.h>

TEST(vdbProcessing, working) {
    using clock = std::chrono::steady_clock;

    auto toMillis = [](const auto &timePoint) -> float {
        return std::chrono::duration_cast<std::chrono::milliseconds>(timePoint).count();
    };

    auto dataset{std::make_unique<vdbDataset<cachedVoxelData, 5, 4, 3>>()};

    openvdb::initialize();

    openvdb::io::File testFile("../" + std::string(filePaths::VDB_CLOUD_HD));

    testFile.open(false);

    auto baseGrid = testFile.readGrid("density");

    openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);

    auto valueIter = grid->beginValueOn();

    while (valueIter) {
        dimType pos;
        float value;

        valueIter.getCoord().asXYZ(pos.x, pos.y, pos.z);
        value = valueIter.getValue();

        dataset->setValue(pos, {.density = value, .integral = 0});

        ++valueIter;
    }

    testFile.close();

    auto integrator{vdbOfflineIntegrator(std::move(dataset))};

    {
        sceneData data{};

        data.sunDir = glm::normalize(glm::vec3{0.3, 0.5, 0.7});
        data.vdbDensityMultiplier = 10;

        integrator.setData(data);
    }

    auto process_t1 = std::chrono::steady_clock::now();

    integrator.dispatch();

    auto process_tMid = std::chrono::steady_clock::now();

    while (true) {
        const auto status = integrator.getStatus();

        std::cout << "Status: " << status.processed << "/" << status.total << ", active: " << status.active
                  << ", percent: " << status.processedPercentage << std::endl;

        if (status.processed == status.total) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    auto processedData = integrator.returnData();

    auto process_t2 = std::chrono::steady_clock::now();

    ASSERT_TRUE(processedData);

    auto check_t1 = std::chrono::steady_clock::now();

    for (auto &voxel : processedData->listVoxels()) {
        ASSERT_EQ(voxel.value->integral, processedData->getValue(voxel.decodePosition()).integral);
        ASSERT_EQ(voxel.value->density, processedData->getValue(voxel.decodePosition()).density);
    }

    auto check_t2 = std::chrono::steady_clock::now();

    std::cout << "dispatch took: " << toMillis(process_tMid - process_t1) << " ms\n"
              << "processing took: " << toMillis(process_t2 - process_tMid) << " ms\n"
              << "checking took  : " << toMillis(check_t2 - check_t1) << " ms\n";
}

#endif //MB_CLOUDS_VDBPROCESSINGTEST_H
