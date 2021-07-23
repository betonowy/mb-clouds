//
// Created by pekopeko on 29.05.2021.
//

#ifndef MB_CLOUDS_MISC_H
#define MB_CLOUDS_MISC_H

#include <exception>
#include <string>

namespace mb::misc {
    [[noreturn]] void exception(const char *reason);

    class exceptionType : std::exception {
    public:
        explicit exceptionType(const char *reason);

    private:
        [[nodiscard]] const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override;

        std::string reason;
    };
}

#endif //MB_CLOUDS_MISC_H
