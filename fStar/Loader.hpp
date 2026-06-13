//
// Created by ja on 03/06/2026.
//

#ifndef IOA_SEMESTRALKA_LOADER_H
#define IOA_SEMESTRALKA_LOADER_H
#include <filesystem>
#include "./NodeAllocator.hpp"

// TODO: Save load fStar + whole problem
template <typename Tnode>
class Loader {
private:
    // NodeAllocator* nodeAllocator;
    // fStar::FStar* star;

public:
    static inline const char* STAR_FILE_NAME = "Star.txt";
    static inline const char* PROBLEM_FILE_NAME = "Problem.txt";

    static void save(std::filesystem::path path, fStar::FStar<Tnode>* star);
    static void load(std::filesystem::path path, fStar::FStar<Tnode>* star, NodeAllocator* nodeAllocator,  bool ignoreId);
};

#include "Loader.tpp"

#endif //IOA_SEMESTRALKA_LOADER_H
