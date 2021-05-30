//
// Created by pekopeko on 29.05.2021.
//

#include "misc.h"
#include <string>

#ifndef DEBUG_BUILD
#include <iostream>
#endif

namespace mb::misc {
    static constexpr const char *_exceptionMessage = "mb::misc::exception: ";

    void exception(const char *reason) {
#ifdef DEBUG_BUILD
        throw exceptionType(reason);
#else
        std::cerr << _exceptionMessage << reason << std::endl;
        std::terminate();
#endif
    }

#ifdef DEBUG_BUILD

    misc::exceptionType::exceptionType(const char *reason)
            : reason(_exceptionMessage + std::string(reason)) {}

    const char *misc::exceptionType::what() const noexcept { return reason.c_str(); }

#endif
}
