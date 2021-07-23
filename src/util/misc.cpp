//
// Created by pekopeko on 29.05.2021.
//

#include "misc.h"

namespace mb::misc {
    static constexpr const char *_exceptionMessage = "mb::misc::exception: ";

    [[noreturn]] void exception(const char *reason) {
        throw exceptionType(reason);
    }

    misc::exceptionType::exceptionType(const char *reason)
            : reason(_exceptionMessage + std::string(reason)) {}

    const char *misc::exceptionType::what() const noexcept { return reason.c_str(); }
}
