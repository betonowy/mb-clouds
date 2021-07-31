//
// Created by pekopeko on 29.05.2021.
//

#ifndef MB_CLOUDS_BINARYFILE_H
#define MB_CLOUDS_BINARYFILE_H

#include <vector>

namespace mb {
    class binaryFile {
    public:
        explicit binaryFile(const char *path);

        [[nodiscard]] const std::vector<unsigned char>& GetBuffer() const;

        [[nodiscard]] const unsigned char* data() const;

        [[nodiscard]] size_t size() const;

        [[nodiscard]] const std::vector<unsigned char>& vector() const;

    private:
        std::vector<unsigned char> buffer;
    };
}


#endif //MB_CLOUDS_BINARYFILE_H
