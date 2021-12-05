//
// Created by pekopeko on 01.11.2021.
//

#include "vdbOfflineIntegrator.h"

void vdbOfflineIntegrator::wait() {
    for (auto &worker : workers) {
        worker.join();
    }
    workers.clear();
}

void vdbOfflineIntegrator::_processVoxel(vdbValueAccessor<5, 4, 3, cachedVoxelData> &voxel) {
    glm::vec3 pos = glm::vec3{voxel.decodePosition()} + glm::vec3{0.5, 0.5, 0.5};

    constexpr float stepSize = 0.3141593;

    float integral{};
    dimType lastPos = pos;
    auto lastVoxel = _cachedCloudStorage->getValue(pos);

    while (pos.x > _lowDim.x && pos.y > _lowDim.y && pos.z > _lowDim.z &&
           pos.x <= _highDim.x && pos.y <= _highDim.y && pos.z <= _highDim.z) {
//        auto lastVoxel = _cachedCloudStorage->getValue(pos);
        pos += _sceneData.sunDir * stepSize;
        integral += lastVoxel.density * stepSize;

        if (dimType{pos} != lastPos) {
            lastVoxel = _cachedCloudStorage->getValue(pos);
        } else {
            (void)0;
        }
    }

    assert(_ratio > 0);

    voxel.value->integral = integral * _ratio;
}

void vdbOfflineIntegrator::_processingFunction(std::vector<vdbValueAccessor<5, 4, 3, cachedVoxelData>> dataToProcess) {
    auto _dataToProcess = std::move(dataToProcess);
    --started;

    for (auto &voxel : _dataToProcess) {
        _processVoxel(voxel);
        ++processed;
    }

    --finished;
}

void vdbOfflineIntegrator::dispatch() {
    const auto maxThreads = std::thread::hardware_concurrency();
    auto voxelsToProcess = _cachedCloudStorage->listVoxels();
    assert(maxThreads < voxelsToProcess.size());

    finished = maxThreads;
    started = maxThreads;
    processed = 0;
    totalSize = voxelsToProcess.size();

    _workSubsets = std::vector<decltype(voxelsToProcess)>(maxThreads);

    for (auto i{std::size_t{}}; i < maxThreads; i++) {
        const auto indexLo{(i + 0) * voxelsToProcess.size() / maxThreads};
        const auto indexHi{(i + 1) * voxelsToProcess.size() / maxThreads};

        decltype(voxelsToProcess) subset;

        auto begin{voxelsToProcess.begin() + int(indexLo)};
        auto end{voxelsToProcess.begin() + int(indexHi)};

        std::move(begin, end, std::back_inserter(_workSubsets[i]));
    }

    _lowDim = _cachedCloudStorage->getLowDim();
    _highDim = _cachedCloudStorage->getHighDim();

    _ratio = 1.0 / std::max(std::max(_highDim.x - _lowDim.x, _highDim.y - _lowDim.y), _highDim.z - _lowDim.z);

    for (auto &workSubset : _workSubsets) {
        workers.emplace_back([this, &workSubset]() {
            _processingFunction(std::move(workSubset));
        });
    }
}

std::unique_ptr<vdbClouds::cachedDatasetType> vdbOfflineIntegrator::returnData() {
    wait();
    return std::move(_cachedCloudStorage);
}
