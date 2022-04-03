/*
 * Copyright (C) 2022  Mathieu Margier
 *
 *  This file is part of Alphadocte.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>. 
 * 
 * File: TestDefinitions.h
 */

#ifndef TESTDEFINITIONS_H_
#define TESTDEFINITIONS_H_

/*
 * Contains common definitions for tests
 */

#include <Alphadocte/FixedSizeDictionary.h>
#include <Alphadocte/TxtDictionary.h>
#include <memory>
#include <filesystem>
#include <fstream>

#include <catch2/catch.hpp>


// file paths
inline const std::filesystem::path TEST_OUT_DIR = std::filesystem::path{"out"};

inline const std::filesystem::path TEST_DATA_DIR = std::filesystem::path{"data"};
inline const std::filesystem::path TEST_MOTUS_WORDS = TEST_DATA_DIR / "motus_test_wordlist.txt";
inline const std::filesystem::path TEST_WORDLE_WORDS = TEST_DATA_DIR / "wordle_test_wordlist.txt";

inline const std::filesystem::path TEST_CONFIG_DIR = TEST_DATA_DIR / "config";
inline const std::filesystem::path TEST_CONFIG_EXAMPLE1 = TEST_CONFIG_DIR / "example_config.txt";
inline const std::filesystem::path TEST_CONFIG_EXAMPLE1_COPY = TEST_CONFIG_DIR / "copy_example_config.txt";
inline const std::filesystem::path TEST_CONFIG_BAD_ENTRY = TEST_CONFIG_DIR / "example_config_bad_entry.txt";
inline const std::filesystem::path TEST_CONFIG_BAD_SECTION1 = TEST_CONFIG_DIR / "example_config_bad_section1.txt";
inline const std::filesystem::path TEST_CONFIG_BAD_SECTION2 = TEST_CONFIG_DIR / "example_config_bad_section2.txt";
inline const std::filesystem::path TEST_CONFIG_BAD_ENTRY_COMMENT = TEST_CONFIG_DIR / "example_config_bad_entry_comment.txt";
inline const std::filesystem::path TEST_CONFIG_CACHE = TEST_CONFIG_DIR / "cache_config.txt";

// common dictionary
inline const std::shared_ptr<Alphadocte::Dictionary> motusDict = std::make_shared<Alphadocte::TxtDictionary>(TEST_MOTUS_WORDS);
inline const std::shared_ptr<Alphadocte::FixedSizeDictionary> wordleDict = std::make_shared<Alphadocte::FixedSizeDictionary>(
        std::make_shared<Alphadocte::TxtDictionary>(TEST_WORDLE_WORDS), 5
);

inline std::shared_ptr<Alphadocte::Dictionary> getMotusDict() {
    if (!motusDict->isLoaded()) {
        REQUIRE(motusDict->load());
    }
    return motusDict;
}

inline std::shared_ptr<Alphadocte::FixedSizeDictionary> getWordleDict() {
    if (!wordleDict->isLoaded()) {
        REQUIRE(wordleDict->load());
    }
    return wordleDict;
}

// helpful functions
// from https://stackoverflow.com/a/37575457
inline bool files_identical(std::filesystem::path path1, std::filesystem::path path2) {
    std::ifstream f1(path1, std::ifstream::binary|std::ifstream::ate);
    std::ifstream f2(path2, std::ifstream::binary|std::ifstream::ate);

    if (f1.fail() || f2.fail()) {
        throw std::runtime_error("Could not open either " + path1.string() + " or " + path2.string() + ", or both.");
    }

    if (f1.tellg() != f2.tellg()) {
        return false; //size mismatch
    }

    //seek back to beginning and use std::equal to compare contents
    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);

    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                   std::istreambuf_iterator<char>(),
                   std::istreambuf_iterator<char>(f2.rdbuf()));
}

#endif /* TESTDEFINITIONS_H_ */
