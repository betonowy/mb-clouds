//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_VDBCLOUDS_H
#define MB_CLOUDS_VDBCLOUDS_H

#include <shaders/cloudStorage.h>
#include <tree/vdbDataExtract.h>
#include <shaders/shader.h>

class vdbClouds {
    using glCloudStorageType = cloudStorage<float, 5, 4, 3>;
    using vdbGlType = vdbGl<float, 5, 4, 3>;
    using vdbDatasetType = vdbDataset<float, 5, 4, 3>;
public:
    explicit vdbClouds(std::string path);

    ~vdbClouds();

    void render();

    void recompileShaders();

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

    std::unique_ptr<vdbDatasetType> _dataset;
    std::unique_ptr<glCloudStorageType> _cloudStorage;

    std::unique_ptr<shader> _cloudShader;
};


#endif //MB_CLOUDS_VDBCLOUDS_H
