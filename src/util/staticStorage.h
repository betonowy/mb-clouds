//
// Created by Mikołaj Bajkowski (ara ara this yo mama) on 07.03.2021.
//

#ifndef BASSSDLGL_STATICSTORAGE_H
#define BASSSDLGL_STATICSTORAGE_H

#include <memory>
#include <unordered_map>
#include <string>

namespace mb {
    template<class T>
    class staticStorage {
    public:
        static const std::shared_ptr<T> &Get(const std::string &path) {
            auto &storage = GetStorage();
            auto &object = storage[path];
            if (!object) object = std::make_shared<T>(path);
            return object;
        }

        inline static const std::unordered_map<std::string, std::shared_ptr<T>> &
        GetConstStorage() { return GetStorage(); }

    private:
        static std::unordered_map<std::string, std::shared_ptr<T>> &GetStorage() {
            static std::unordered_map<std::string, std::shared_ptr<T>> storage;
            return storage;
        }
    };
}
#endif //BASSSDLGL_STATICSTORAGE_H
