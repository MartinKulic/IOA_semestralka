//
// Created by ja on 03/06/2026.
//

#ifndef IOA_SEMESTRALKA_LOADER_H
#define IOA_SEMESTRALKA_LOADER_H
#include <filesystem>
#include "./NodeAllocator.hpp"

// TODO: Save load fStar + whole problem

class Loader {
private:
    NodeAllocator* nodeAllocator;

public:
    void save(std::filesystem::path path);
    void load(std::string path);
};


#endif //IOA_SEMESTRALKA_LOADER_H
