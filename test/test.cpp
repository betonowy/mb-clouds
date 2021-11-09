//
// Created by pekopeko on 20.07.2021.
//

#include <gtest/gtest.h>

#include "tests/vdbProcessingTest.h"
#include "tests/vdbDataTest.h"
#include "tests/vdbDataExtractTest.h"

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
