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
    // NodeAllocator* nodeAllocator;
    // fStar::FStar* star;

public:
    const char* STAR_FILE_NAME = "Star.txt";
    const char* PROBLEM_FILE_NAME = "Problem.txt";

    static void save(std::filesystem::path path, fStar::FStar* star);
    static void load(std::string path, fStar::FStar* star, NodeAllocator* nodeAllocator);
};


#endif //IOA_SEMESTRALKA_LOADER_H
