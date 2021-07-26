//
// Created by pekopeko on 25.07.2021.
//

#include "vdbClouds.h"

#include <openvdb/openvdb.h>

vdbClouds::vdbClouds(std::string path)
        : _vdbPath(std::move(path)) {
    _initCloudStorage();
    _initShaders();
}

vdbClouds::~vdbClouds() {
    _destroyDataset();
}

void vdbClouds::_initDataset() {
    if (_dataset) return;

    _dataset = std::make_unique<vdbDatasetType>();

    {
        openvdb::initialize();

        std::cout << "Loading OpenVDB file: " << _vdbPath << "\n";

        openvdb::io::File file(_vdbPath);

        file.open(false);

        auto baseGrid = file.readGrid("density");

        openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);

        auto valueIter = grid->beginValueOn();

        std::cout << "Reading OpenVDB file into our custom dataset\n";

        while (valueIter) {
            dimType pos;
            float value = valueIter.getValue();
            valueIter.getCoord().asXYZ(pos.x, pos.y, pos.z);

            _dataset->setValue(pos, value);

            ++valueIter;
        }

        file.close();

        openvdb::uninitialize();
    }
}

void vdbClouds::_destroyDataset() {
    _dataset.reset();
}

void vdbClouds::_resetDataset() {
    _destroyDataset();
    _initDataset();
}

void vdbClouds::_initCloudStorage() {
    if (_cloudStorage) return;

    _initDataset();

    std::cout << "Extracting GPU usable data from dataset\n";

    vdbGlType extract(*_dataset);

    std::cout << "Creating data structures on GPU\n";

    _cloudStorage = std::make_unique<glCloudStorageType>(extract);
}

void vdbClouds::_destroyCloudStorage() {
    _cloudStorage.reset();
}

void vdbClouds::_resetCloudStorage() {
    _destroyCloudStorage();
    _initCloudStorage();
}

void vdbClouds::_initShaders() {
    if (_cloudShader) return;
    _cloudShader = std::make_unique<shader>(std::initializer_list<const char *>{
            filePaths::GLSL_CLOUD_VERT,
            filePaths::GLSL_CLOUD_FRAG
    });
}

void vdbClouds::_destroyShaders() {
    _cloudShader.reset();
}

void vdbClouds::_resetShaders() {
    _destroyShaders();
    _initShaders();
}

void vdbClouds::render() {
    _cloudShader->use();
    _cloudStorage->bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void vdbClouds::recompileShaders() {
    _resetShaders();
}
