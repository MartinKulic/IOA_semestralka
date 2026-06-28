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
    static inline const char* STAR_FILE_NAME = "Star.txt";
    static inline const char* SOLUTION_FILE_NAME = "Solution.txt";

    static void save(std::filesystem::path path, fStar::FStar* star, std::string* sol_big=nullptr, std::string* sol_small=nullptr);
    static void load(std::filesystem::path path, fStar::FStar* star, NodeAllocator* nodeAllocator,  bool ignoreId, std::string* sol_big=nullptr, std::string* sol_small=nullptr);
};


#endif //IOA_SEMESTRALKA_LOADER_H
