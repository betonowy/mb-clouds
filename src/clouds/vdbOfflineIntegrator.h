//
// Created by pekopeko on 01.11.2021.
//

#ifndef MB_CLOUDS_VDBOFFLINEINTEGRATOR_H
#define MB_CLOUDS_VDBOFFLINEINTEGRATOR_H

#include "vdbClouds.h"
#include "shaders/sceneData.h"

#include <thread>
#include <mutex>
#include <condition_variable>

#include "vdbIntegrationStatus.h"

class vdbOfflineIntegrator {
public:
    explicit vdbOfflineIntegrator(std::unique_ptr<vdbClouds::cachedDatasetType> cachedCloudStorage)
            : _cachedCloudStorage(std::move(cachedCloudStorage)) {}

    integrationStatus getStatus() {
        return {
                .processed = processed,
                .total = totalSize,
                .active = finished - started,
                .processedPercentage = 100.f * (float(processed) / float(totalSize)),
        };
    }

    void dispatch();

    void wait();

    std::unique_ptr<vdbClouds::cachedDatasetType> returnData();

    void setData(sceneData sceneData) { _sceneData = sceneData; }

private:

    void _processVoxel(vdbValueAccessor<5, 4, 3, cachedVoxelData> &voxel);

    void _processingFunction(std::vector<vdbValueAccessor<5, 4, 3, cachedVoxelData>> dataToProcess);

    std::unique_ptr<vdbClouds::cachedDatasetType> _cachedCloudStorage;

    std::vector<std::thread> workers;
    std::vector<decltype(_cachedCloudStorage->listVoxels())> _workSubsets;

    glm::vec3 _lowDim, _highDim;
    float _ratio;

    std::size_t totalSize{};
    std::atomic<std::size_t> started{}, finished{}, processed{};

    sceneData _sceneData;
};


#endif //MB_CLOUDS_VDBOFFLINEINTEGRATOR_H
