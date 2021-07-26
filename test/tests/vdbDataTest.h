//
// Created by pekopeko on 20.07.2021.
//

#ifndef MB_CLOUDS_VDBDATATEST_H
#define MB_CLOUDS_VDBDATATEST_H

#include <tree/vdbData.h>
#include <config.h>

#include <openvdb/openvdb.h>

#include <random>
#include <chrono>

TEST(vdbData, vdbLeaf) {
    using vdbLeafType = vdbLeaf<float, 3>;

    vdbLeafType leaf(dimType{2}, dimType{10});

    ASSERT_EQ(leaf.countOnValues(), 0);

    ASSERT_EQ(leaf.getSize(), dimType{8});
    ASSERT_EQ(leaf.getLowDim(), dimType{2});
    ASSERT_EQ(leaf.getHighDim(), dimType{10});

    leaf.setValue({2, 3, 4}, 0.5f);
    ASSERT_EQ(leaf.countOnValues(), 1);
    ASSERT_FLOAT_EQ(leaf.getValue({2, 3, 4}), 0.5f);

    leaf.setValue({3, 3, 4}, 0.4f);
    ASSERT_EQ(leaf.countOnValues(), 2);
    ASSERT_FLOAT_EQ(leaf.getValue({3, 3, 4}), 0.4f);

    leaf.unsetValue({2, 3, 4});
    ASSERT_EQ(leaf.countOnValues(), 1);

    auto vdbData = leaf.getData();

    ASSERT_TRUE(vdbData.bitset.any());
}

TEST(vdbData, vdbNode) {
    using vdbNodeType = vdbNode<float, 4>;

    vdbNodeType node({10, 10, 10}, {26, 26, 26});

    ASSERT_EQ(node.calculateIndex({0, 0, 0}), 0);
    ASSERT_EQ(node.calculateIndex({2, 3, 5}), 2 + 16 * 3 + 16 * 16 * 5);

    ASSERT_EQ(node.calculateIndex({-1, 0, 0}), vdbBadIndex);
    ASSERT_EQ(node.calculateIndex({0, -1, 0}), vdbBadIndex);
    ASSERT_EQ(node.calculateIndex({0, 0, -1}), vdbBadIndex);

    ASSERT_EQ(node.calculateIndex({16, 0, 0}), vdbBadIndex);
    ASSERT_EQ(node.calculateIndex({0, 16, 0}), vdbBadIndex);
    ASSERT_EQ(node.calculateIndex({0, 0, 16}), vdbBadIndex);

    node.setIndex({1, 2, 3}, 13);
    node.setIndex({2, 3, 4}, 12);
    node.setIndex({0, 0, 0}, 11);

    ASSERT_TRUE(node.getIndex({1, 2, 3}));
    ASSERT_TRUE(node.getIndex({2, 3, 4}));
    ASSERT_TRUE(node.getIndex({0, 0, 0}));

    ASSERT_FALSE(node.getIndex({3, 4, 5}));
}

TEST(vdbData, vdbAccessorTest) {
    using accType = vdbAccessor<5, 4, 3>;

    {
        dimType pos = {0, 0, 0};

        auto acc = accType(pos);
        ASSERT_EQ(acc.voxelPos, dimType(0, 0, 0));
        ASSERT_EQ(acc.leafPos, dimType(0, 0, 0));
        ASSERT_EQ(acc.nodePos, dimType(0, 0, 0));
        ASSERT_EQ(acc.rootPos, dimType(0, 0, 0));

        const auto returnPos = acc.decodePosition();

        ASSERT_EQ(pos, returnPos);
    }

    {
        dimType pos = {129, 9, 1};

        auto acc = accType(pos);
        ASSERT_EQ(acc.voxelPos, dimType(1, 1, 1));
        ASSERT_EQ(acc.leafPos, dimType(0, 1, 0));
        ASSERT_EQ(acc.nodePos, dimType(1, 0, 0));
        ASSERT_EQ(acc.rootPos, dimType(0, 0, 0));

        const auto returnPos = acc.decodePosition();

        ASSERT_EQ(pos, returnPos);
    }

    {
        dimType pos = {-3967, -4087, -4095};

        auto acc = accType(pos);
        ASSERT_EQ(acc.voxelPos, dimType(1, 1, 1));
        ASSERT_EQ(acc.leafPos, dimType(0, 1, 0));
        ASSERT_EQ(acc.nodePos, dimType(1, 0, 0));
        ASSERT_EQ(acc.rootPos, dimType(-1, -1, -1));

        const auto returnPos = acc.decodePosition();

        ASSERT_EQ(pos, returnPos);
    }

    {
        dimType pos = {997, 66, 3};

        auto acc = accType(pos);
        ASSERT_EQ(acc.voxelPos, dimType(5, 2, 3));
        ASSERT_EQ(acc.leafPos, dimType(12, 8, 0));
        ASSERT_EQ(acc.nodePos, dimType(7, 0, 0));
        ASSERT_EQ(acc.rootPos, dimType(0, 0, 0));

        const auto returnPos = acc.decodePosition();

        ASSERT_EQ(pos, returnPos);
    }

    {
        dimType pos = {-281627, -282558, -282621};

        const auto acc = accType(pos);

        ASSERT_EQ(acc.voxelPos, dimType(5, 2, 3));
        ASSERT_EQ(acc.leafPos, dimType(12, 8, 0));
        ASSERT_EQ(acc.nodePos, dimType(7, 0, 0));
        ASSERT_EQ(acc.rootPos, dimType(-69, -69, -69));

        const auto returnPos = acc.decodePosition();

        ASSERT_EQ(pos, returnPos);
    }
}

TEST(vdbData, vdbDatasetHierarchyTest) {
    vdbDataset<float, 5, 4, 3> dataset;

    constexpr uint32_t ARRAY_SIZE = 8;

    glm::ivec3 positions[ARRAY_SIZE]{
            {-6144, -4096, -2049},
            {-2048, 0,     2047},
            {2048,  4096,  6143},
            {6144,  8192,  10239},
            {1,     0,     1},
            {0,     2,     3},
            {4,     8,     3},
            {0,     9,     4},
    };

    float values[ARRAY_SIZE]{1, 2, 3, 4, 5, 6, 7, 8};

    for (uint32_t i = 0; i < ARRAY_SIZE; i++) {
        dataset.setValue(positions[i], values[i]);
    }

    for (uint32_t i = 0; i < ARRAY_SIZE; i++) {
        ASSERT_EQ(dataset.getValue(positions[i]), values[i]);
    }
}

TEST(vdbData, vdbDatasetFillTest) {
    using clock = std::chrono::steady_clock;

    auto toMillis = [](const auto &timePoint) -> float {
        return std::chrono::duration_cast<std::chrono::milliseconds>(timePoint).count();
    };

    vdbDataset<float, 5, 4, 3> dataset;

    std::vector<std::pair<dimType, float>> testData;

    constexpr dimType lowDim{-64, -64, -64};
    constexpr dimType highDim{256, 64, 64};
    constexpr uint32_t finalRoots = 8;
    constexpr uint32_t finalNodes = 12;
    constexpr uint32_t finalLeaves = 10240;
    constexpr uint32_t finalVoxels = 5242880;

    testData.reserve(finalVoxels);

    std::random_device rd;

    std::ranlux24_base re(rd());

    std::uniform_real_distribution<float> ud(0, 1);

    auto genT1 = clock::now();

    for (int x = lowDim.x; x < highDim.x; x++) {
        for (int y = lowDim.y; y < highDim.y; y++) {
            for (int z = lowDim.z; z < highDim.z; z++) {
                testData.emplace_back(dimType(x, y, z), ud(re));
            }
        }
    }

    auto readData = testData;

    std::shuffle(testData.begin(), testData.end(), re);

    auto genT2 = clock::now();
    auto fillT1 = genT2;

    for (auto &[pos, val] : testData) {
        dataset.setValue(pos, val);
    }

    auto fillT2 = clock::now();
    auto reorgT1 = fillT2;

    dataset.reorganizeData();

    auto reorgT2 = clock::now();
    auto getT1 = reorgT2;

    for (auto &[pos, val] : readData) {
        ASSERT_EQ(dataset.getValue(pos), val);
    }

    auto getT2 = clock::now();

    ASSERT_EQ(dataset.countRoots(), finalRoots);
    ASSERT_EQ(dataset.countNodes(), finalNodes);
    ASSERT_EQ(dataset.countLeaves(), finalLeaves);
    ASSERT_EQ(dataset.countVoxels(), finalVoxels);

    constexpr uint32_t kiB = 1 << 10;
    constexpr uint32_t miB = 1 << 20;

    const auto memSize = dataset.countMemorySize();

    constexpr uint32_t w = 9;

    std::cout << "vdb fill stress test:\n"
              << "gen time: " << std::setw(w) << toMillis(genT2 - genT1) << " ms\n"
              << "fill time:" << std::setw(w) << toMillis(fillT2 - fillT1) << " ms\n"
              << "re time:  " << std::setw(w) << toMillis(reorgT2 - reorgT1) << " ms\n"
              << "get time: " << std::setw(w) << toMillis(getT2 - getT1) << " ms\n"
              << "roots:    " << std::setw(w) << dataset.countRoots() << "\n"
              << "nodes:    " << std::setw(w) << dataset.countNodes() << "\n"
              << "leaves:   " << std::setw(w) << dataset.countLeaves() << "\n"
              << "voxels:   " << std::setw(w) << dataset.countVoxels() << "\n"
              << "memory footprint:\n"
              << std::setw(w) << memSize << " B\n"
              << std::setw(w) << memSize / kiB << " kiB\n"
              << std::setw(w) << memSize / miB << " miB\n";
}

TEST(vdbData, vdbDatasetRealDataStressTest) {
    using clock = std::chrono::steady_clock;

    auto toMillis = [](const auto &timePoint) -> float {
        return std::chrono::duration_cast<std::chrono::milliseconds>(timePoint).count();
    };

    vdbDataset<float, 5, 4, 3> dataset;

    openvdb::initialize();

    openvdb::io::File testFile("../" + std::string(filePaths::VDB_CLOUD_LD));

    testFile.open(false);

    auto baseGrid = testFile.readGrid("density");

    openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);

    auto valueIter = grid->beginValueOn();

    std::vector<std::pair<dimType, float>> cloudVals;

    while (valueIter) {
        dimType pos;
        float value;

        valueIter.getCoord().asXYZ(pos.x, pos.y, pos.z);
        value = valueIter.getValue();

        dataset.setValue(pos, value);
        cloudVals.emplace_back(pos, value);

        ++valueIter;
    }

    dataset.reorganizeData();

    std::cout << "openvdb mem usage: " << grid->memUsage() << "\n"
              << "my data mem usage: " << dataset.countMemorySize() << "\n";

    auto time = clock::now();

    for (auto &[pos, value] : cloudVals) {
        grid->getAccessor().getValue({pos.x, pos.y, pos.z});
    }

    auto vdbIterTime = clock::now() - time;

    time = clock::now();

    for (auto &[pos, value] : cloudVals) {
        dataset.getValue({pos.x, pos.y, pos.z});
    }

    auto datasetIterTime = clock::now() - time;

    auto memSize = dataset.countMemorySize();

    constexpr uint32_t w = 9;
    constexpr uint32_t kiB = 1 << 10;
    constexpr uint32_t miB = 1 << 20;

    std::cout << "vdb iteration time:\n"
              << "openvdb time :" << std::setw(w) << toMillis(vdbIterTime) << " ms\n"
              << "roots:    " << std::setw(w) << grid->tree().nodeCount()[2] << "\n"
              << "nodes:    " << std::setw(w) << grid->tree().nodeCount()[1] << "\n"
              << "leaves:   " << std::setw(w) << grid->tree().nodeCount()[0] << "\n"
              << "voxels:   " << std::setw(w) << grid->tree().activeVoxelCount() << "\n"
              << "my data time :" << std::setw(w) << toMillis(datasetIterTime) << " ms\n"
              << "roots:    " << std::setw(w) << dataset.countRoots() << "\n"
              << "nodes:    " << std::setw(w) << dataset.countNodes() << "\n"
              << "leaves:   " << std::setw(w) << dataset.countLeaves() << "\n"
              << "voxels:   " << std::setw(w) << dataset.countActiveVoxels() << "\n"
              << "memory footprint:\n"
              << std::setw(w) << memSize << " B\n"
              << std::setw(w) << memSize / kiB << " KiB\n"
              << std::setw(w) << memSize / miB << " MiB\n";

    testFile.close();
}

#endif //MB_CLOUDS_VDBDATATEST_H
