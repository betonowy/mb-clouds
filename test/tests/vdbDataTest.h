//
// Created by pekopeko on 20.07.2021.
//

#ifndef MB_CLOUDS_VDBDATATEST_H
#define MB_CLOUDS_VDBDATATEST_H

#include <tree/vdbData.h>

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

    ASSERT_TRUE(vdbData.flag);
}

TEST(vdbData, vdbNode) {
    using vdbNodeType = vdbNode<4>;

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

TEST(vdbData, vdbDatasetRootTest) {
    vdbDataset<float, 5, 4, 3> dataset;

    constexpr size_t ARRAY_SIZE = 8;

    glm::ivec3 positions[ARRAY_SIZE] {
            {-6144, -4096, -2049},
            {-2048, 0, 2047},
            {2048, 4096, 6143},
            {6144, 8192, 10239},
            {1, 0, 1},
            {0, 2, 3},
            {4, 8, 3},
            {0, 9, 4},
    };

    float values[ARRAY_SIZE] {1, 2, 3, 4, 5, 6, 7, 8};

    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        dataset.setValue(positions[i], values[i]);
    }

    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        ASSERT_EQ(dataset.getValue(positions[i]), values[i]);
    }
}


#endif //MB_CLOUDS_VDBDATATEST_H
