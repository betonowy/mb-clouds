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

//    static void StoreDropUnused() {
//        auto &storage = GetStorage();
//
//        auto iterator = storage.begin();
//        while (iterator != storage.end()) {
//            if (iterator->second.unique()) {
//                storage.erase(iterator);
//                iterator = storage.begin();
//            } else {
//                ++iterator;
//            }
//        }
//    }

        inline static const std::unordered_map<std::string, std::shared_ptr<T>> &
        GetConstStorage() { return GetStorage(); }

    private:
        static std::unordered_map<std::string, std::shared_ptr<T>> &GetStorage() {
            static std::unordered_map<std::string, std::shared_ptr<T>> storage;
            return storage;
        }
    };

//    template<class T>
//    class staticStorageLoadable {
//    public:
//
//        static std::shared_ptr<T> Get(const std::string &path, bool preload = true) {
//            auto &storage = GetStorage();
//            auto &object = storage[path];
//            if (!object) object = std::make_shared<T>(path);
//            if (preload) object->Load();
//            return object;
//        }
//
//        static void StoreUnloadUnused() {
//            for (auto &object : GetStorage()) {
//                if (object.second.use_count() == 1)
//                    object.second->Unload();
//            }
//        }
//
//        static void StoreDropUnused() {
//            auto &storage = GetStorage();
//
//            auto iterator = storage.begin();
//            while (iterator != storage.end()) {
//                if (iterator->second.unique()) {
//                    storage.erase(iterator);
//                    iterator = storage.begin();
//                } else {
//                    ++iterator;
//                }
//            }
//        }
//
//    private:
//        static std::unordered_map<std::string, std::shared_ptr<T>> &GetStorage() {
//            static std::unordered_map<std::string, std::shared_ptr<T>> storage;
//            return storage;
//        }
//    };
}
#endif //BASSSDLGL_STATICSTORAGE_H
