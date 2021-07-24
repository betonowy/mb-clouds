//
// Created by pekopeko on 24.07.2021.
//

#ifndef MB_CLOUDS_VDBDATAEXTRACTTEST_H
#define MB_CLOUDS_VDBDATAEXTRACTTEST_H

#include <tree/vdbDataExtract.h>

TEST(vdbDataExtract, vdbDataExtractInitTest) {
    vdbDataset<float, 5, 4, 3> dataset;

    // prepare data begin

    std::vector<std::pair<dimType, float>> testData;

    constexpr dimType lowDim{-64, -64, -64};
    constexpr dimType highDim{256, 64, 64};

    std::random_device rd;

    std::ranlux24_base re(rd());

    std::uniform_real_distribution<float> ud(0, 1);

    for (int x = lowDim.x; x < highDim.x; x++) {
        for (int y = lowDim.y; y < highDim.y; y++) {
            for (int z = lowDim.z; z < highDim.z; z++) {
                testData.emplace_back(dimType(x, y, z), ud(re));
            }
        }
    }

    std::shuffle(testData.begin(), testData.end(), re);

    for (auto &[pos, val] : testData) {
        dataset.setValue(pos, val);
    }

    // prepare data end

    vdbGl<float, 5, 4, 3> extract(dataset);

    std::cout << "description:\n" << extract.getDescriptionPtr()->toString();
}

#endif //MB_CLOUDS_VDBDATAEXTRACTTEST_H
