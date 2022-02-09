//
// Created by pekopeko on 25.07.2021.
//

#include "vdbClouds.h"

#include "vdbOfflineIntegrator.h"

#include <OpenVDB.h>

vdbClouds::vdbClouds(std::string path)
        : _vdbPath(std::move(path)) {
    _pushShader("Debug AABB", std::initializer_list<const char *>{
            filePaths::GLSL_SCREEN_QUAD_VERT,
            filePaths::GLSL_VDB_DEBUG_AABB_FRAG
    });
    _pushShader("Color Additive", std::initializer_list<const char *>{
            filePaths::GLSL_SCREEN_QUAD_VERT,
            filePaths::GLSL_VDB_COLOR_ADD_FRAG
    });
    _pushShader("Secondary Sub", std::initializer_list<const char *>{
            filePaths::GLSL_SCREEN_QUAD_VERT,
            filePaths::GLSL_VDB_SECONDARY_SUB_FRAG
    });
    _pushShader("Secondary Sub V2", std::initializer_list<const char *>{
            filePaths::GLSL_SCREEN_QUAD_VERT,
            filePaths::GLSL_VDB_SECONDARY_SUB_V2_FRAG
    });

    _availableVdbFiles = {filePaths::VDB_CLOUD_HD, filePaths::VDB_CLOUD_MD, filePaths::VDB_CLOUD_LD};

//    _initCloudStorage();
}

vdbClouds::~vdbClouds() {
    _destroyDataset();
}

void vdbClouds::_initDataset() {
    if (_dataset) return;

    _dataset = std::make_unique<vdbDatasetType>();
    _cachedDataset = std::make_unique<cachedDatasetType>();

    {
        openvdb::initialize();

        std::cout << "Loading OpenVDB file: " << _vdbPath << "\n";

        openvdb::io::File file(_vdbPath);

        file.open(false);

        auto baseGrid = file.readGrid("density");

        openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);

        auto valueIter = grid->beginValueOn();

        std::cout << "Reading OpenVDB file into our custom dataset\n";

        valueIter = grid->beginValueOn();

        while (valueIter) {
            dimType pos;
            float value = valueIter.getValue();
            valueIter.getCoord().asXYZ(pos.x, pos.y, pos.z);

            // convert between blender/opengl coordinate system
            _dataset->setValue({-pos.x, pos.z, pos.y}, value);
            _cachedDataset->setValue({-pos.x, pos.z, pos.y}, {value, 0});

            ++valueIter;
        }

        std::cout << "Dataset size: " << _dataset->countMemorySize() << '\n';
        std::cout << "Cached size: " << _cachedDataset->countMemorySize() << '\n';

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
//    if (_cloudStorage) return;

    _initDataset();

    std::cout << "Extracting GPU usable data from dataset\n";

    cachedGlType extract(*_cachedDataset);
    vdbGlType originalExtract(*_dataset);

    std::cout << "Creating data structures on GPU\n";

    _memorySize =
            extract.getNodesSize() +
            extract.getLeavesSize() +
            extract.getRootsSize() +
            extract.getDescriptionSize();

    _rawMemorySize =
            originalExtract.getNodesSize() +
            originalExtract.getLeavesSize() +
            originalExtract.getRootsSize() +
            originalExtract.getDescriptionSize();

    _cachedCloudStorage = std::make_unique<cachedCloudStorageType>(extract);
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
    recompileShaders(_availableShaders.front().first);
}

void vdbClouds::_destroyShaders() {
    _cloudShader.reset();
}

void vdbClouds::_resetShaders() {
    if (lastShader.empty()) {
        _destroyShaders();
        _initShaders();
    } else {
        recompileShaders(lastShader);
    }
}

void vdbClouds::render() {
    if (!_cloudShader) return;

    _cloudShader->use();
    _cloudStorage->bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void vdbClouds::recompileShaders() {
    _resetShaders();
}

void vdbClouds::_pushShader(const std::string &name, std::initializer_list<const char *> sources) {
    _availableShaders.emplace_back(name, std::vector<std::string>(sources.begin(), sources.end()));
}

const std::vector<std::pair<std::string, std::vector<std::string>>> &vdbClouds::getAvailableShaders() {
    return _availableShaders;
}

void vdbClouds::recompileShaders(std::string_view name) {
    std::cout << "Recompiling program <<< " << name << " >>>\n";
    lastShader = name;
    auto iter = std::find_if(_availableShaders.begin(), _availableShaders.end(), [&name](auto pair) {
        return pair.first == name;
    });

    _cloudShader = std::make_unique<shader>(iter->second);
}

const std::vector<std::string> &vdbClouds::getAvailableVdbFiles() {
    return _availableVdbFiles;
}

void vdbClouds::changeDataset(std::string_view path) {
    _vdbPath = path;
    _destroyDataset();
    _resetCloudStorage();
}

void vdbClouds::bind() {
    _cachedCloudStorage->bind();
}

void vdbClouds::launchProcessing(sceneData sceneData) {
    _integrator = std::make_unique<vdbOfflineIntegrator>(std::move(_cachedDataset));
    _integrator->setData(sceneData);
    _integrator->dispatch();
}

integrationStatus vdbClouds::getProcessingStatus() {
    return _integrator ? _integrator->getStatus() : integrationStatus{};
}

void vdbClouds::finalizeProcessing() {
    _cachedDataset = _integrator->returnData();
    _integrator.reset();
    _initCloudStorage();
}
