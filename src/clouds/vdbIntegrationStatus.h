//
// Created by pekopeko on 09.11.2021.
//

#ifndef MB_CLOUDS_VDBINTEGRATIONSTATUS_H
#define MB_CLOUDS_VDBINTEGRATIONSTATUS_H

#include <cstdint>

struct integrationStatus {
    std::size_t processed;
    std::size_t total;
    std::size_t active;
    float processedPercentage;
};

#endif //MB_CLOUDS_VDBINTEGRATIONSTATUS_H
