//
// Created by pekopeko on 29.05.2021.
//

#ifndef MB_CLOUDS_MISC_H
#define MB_CLOUDS_MISC_H

#ifdef DEBUG_BUILD

#include <exception>
#include <string>

#endif

namespace mb::misc {
    void exception(const char *reason);

#ifdef DEBUG_BUILD

    class exceptionType : std::exception {
    public:
        explicit exceptionType(const char *reason);

    private:
        [[nodiscard]] const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override;

        std::string reason;
    };

#endif

}


#endif //MB_CLOUDS_MISC_H
