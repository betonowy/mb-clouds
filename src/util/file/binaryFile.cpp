//
// Created by pekopeko on 29.05.2021.
//

#include "binaryFile.h"

#include <fstream>
#include <util/misc.h>
#include <iterator>

namespace mb {
    binaryFile::binaryFile(const char *path) {
        std::ifstream file(path, std::ios::binary);

        if (!file) misc::exception(("Opening file at: " + std::string(path) + " failed").c_str());
        // don't eat newlines
        file.unsetf(std::ios::skipws);
        // read file size
        file.seekg(0, std::ios::end);
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);
        // reserve vector so we allocate memory only once
        buffer.reserve(size);
        // load file into buffer
        buffer.insert(buffer.begin(),
                      std::istream_iterator<uint8_t>(file),
                      std::istream_iterator<uint8_t>());
    }

    const std::vector<unsigned char> &binaryFile::GetBuffer() const { return buffer; }

    const unsigned char *binaryFile::data() const { return buffer.data(); }

    size_t binaryFile::size() const { return buffer.size(); }
}
