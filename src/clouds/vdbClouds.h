//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_VDBCLOUDS_H
#define MB_CLOUDS_VDBCLOUDS_H

#include <shaders/cloudStorage.h>

#include <tree/vdbDataExtract.h>
#include <shaders/shader.h>
#include "vdbIntegrationStatus.h"
#include <shaders/sceneData.h>

#include <memory>

struct cachedVoxelData {
    float density;
    float integral;
};

class vdbOfflineIntegrator;

class vdbClouds {
public:
    using glCloudStorageType = cloudStorage<float, 5, 4, 3>;
    using vdbGlType = vdbGl<float, 5, 4, 3>;
    using vdbDatasetType = vdbDataset<float, 5, 4, 3>;
    using cachedCloudStorageType = cloudStorage<cachedVoxelData, 5, 4, 3>;
    using cachedGlType = vdbGl<cachedVoxelData, 5, 4, 3>;
    using cachedDatasetType = vdbDataset<cachedVoxelData, 5, 4, 3>;
    
    explicit vdbClouds(std::string path);

    ~vdbClouds();

    void render();

    void recompileShaders();

    void recompileShaders(std::string_view name);

    void changeDataset(std::string_view path);

    const std::vector<std::string> &getAvailableVdbFiles();

    const std::vector<std::pair<std::string, std::vector<std::string>>> &getAvailableShaders();

    [[nodiscard]] inline dimType getSize() const { return _dataset->getHighDim() - _dataset->getLowDim(); }

    inline std::size_t getMemorySize() { return _memorySize; }

    void bind();

    integrationStatus getProcessingStatus();

    void launchProcessing(sceneData sceneData);

    void finalizeProcessing();

private:
    std::string _vdbPath;

    void _initDataset();

    void _destroyDataset();

    void _resetDataset();

    void _initCloudStorage();

    void _destroyCloudStorage();

    void _resetCloudStorage();

    void _initShaders();

    void _destroyShaders();

    void _resetShaders();

    void _pushShader(const std::string &name, std::initializer_list<const char *> sources);

    std::unique_ptr<vdbDatasetType> _dataset;
    std::unique_ptr<glCloudStorageType> _cloudStorage;

    std::unique_ptr<cachedDatasetType> _cachedDataset;
    std::unique_ptr<cachedCloudStorageType> _cachedCloudStorage;

    std::unique_ptr<vdbOfflineIntegrator> _integrator;

    std::string lastShader;
    std::unique_ptr<shader> _cloudShader;

    std::vector<std::pair<std::string, std::vector<std::string>>> _availableShaders;
    std::vector<std::string> _availableVdbFiles;

    std::size_t _memorySize = 0;
};


#endif //MB_CLOUDS_VDBCLOUDS_H
