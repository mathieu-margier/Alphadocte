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
 * File: DictionaryTests.cpp
 */

#include <Alphadocte/Dictionary.h>
#include <Alphadocte/FixedSizeDictionary.h>
#include <Alphadocte/TxtDictionary.h>
#include <cmath>
#include <map>

#include <boost/random/mersenne_twister.hpp>
#include <catch2/catch.hpp>

#include "stubs/DictionaryStub.h"
#include "TestDefinitions.h"

using namespace Alphadocte;

// dictionary to test
static const std::vector<std::string> SIMPLE_WORDLIST = {
        "a", "b", "c", "d", "e", "f", "g"
};
static const std::vector<std::string> COMPOSITE_WORDLIST = {
        "a", "aa", "aaa", "ab", "abc",
        "b", "baa", "bb",
        "c", "cc", "ccc",
        "ddddd"
};
static const std::vector<std::string> COMPOSITE_WORDLIST_1 = {
        "a", "b", "c"
};
static const std::vector<std::string> COMPOSITE_WORDLIST_2 = {
        "aa", "ab", "bb", "cc"
};
static const std::vector<std::string> COMPOSITE_WORDLIST_3 = {
        "aaa", "abc", "baa", "ccc"
};
static const std::vector<std::string> COMPOSITE_WORDLIST_5 = {
        "ddddd"
};

// file paths
static const std::filesystem::path SIMPLE_WORDLIST_FILE = TEST_DATA_DIR / "simple_wordlist.txt";
static const std::filesystem::path INVALID1_WORDLIST_FILE = TEST_DATA_DIR / "invalid1_wordlist.txt";
static const std::filesystem::path INVALID2_WORDLIST_FILE = TEST_DATA_DIR / "invalid2_wordlist.txt";
static const std::filesystem::path INVALID3_WORDLIST_FILE = TEST_DATA_DIR / "invalid3_wordlist.txt";
static const std::filesystem::path NOT_A_FILE = TEST_DATA_DIR / "not_a_file";

// parameters for statistical tests
static constexpr unsigned int TEST_RANDOM_WORDS_N = 20;
/*
 * chi 2 test to check if random words do follow an uniform distribution
 * TODO Use boost's quantile function if more tests are needed, without computing by hand every upperbound value...
 */
constexpr size_t CHI2_TEST_SAMPLES = 100;
constexpr size_t CHI2_TEST_DEGREE_FREEDOM = 6;
constexpr double CHI2_TEST_UPPERBOUND = 12.59; // extracted from a table, depend on the degree of freedom and the chosen p-value (0.05 here)

// static vars
static boost::mt19937 randomGenerator(std::time(nullptr));

TEST_CASE("Checking dictionary base class", "[dictionary][Lib]") {
    DictionaryStub dict{SIMPLE_WORDLIST};

    CHECK(dict.isLoaded());
    CHECK_FALSE(dict.load());
    CHECK(dict.isLoaded());
    REQUIRE(std::is_sorted(std::cbegin(dict.getAllWords()), std::cend(dict.getAllWords())));
    REQUIRE(dict.getAllWords() == SIMPLE_WORDLIST);

    SECTION("Check if dictionary contains words") {
        REQUIRE(dict.contains("a"));
        REQUIRE(dict.contains("d"));
        REQUIRE(dict.contains("g"));
        REQUIRE_FALSE(dict.contains(""));
        REQUIRE_FALSE(dict.contains("z"));
        REQUIRE_FALSE(dict.contains("h"));
        REQUIRE_FALSE(dict.contains("o"));
        REQUIRE_FALSE(dict.contains("aa"));
        REQUIRE_FALSE(dict.contains("ab"));
    }


    SECTION("Random draw of word from dictionary") {
        std::map<std::string_view, size_t> occurences;
        for (size_t i = 0; i < CHI2_TEST_SAMPLES; i++) {
            auto word = dict.getRandomWord(randomGenerator);
            REQUIRE(dict.contains(word));

            if (auto it = occurences.find(word); it != occurences.end()) {
                it->second++;
            } else {
                occurences.emplace(word, 1);
            }

        }

        REQUIRE(CHI2_TEST_DEGREE_FREEDOM == std::size(SIMPLE_WORDLIST) - 1); // degree of freedom is correct
        const double expectedNo = static_cast<double>(CHI2_TEST_SAMPLES) / std::size(SIMPLE_WORDLIST);
        double d2 = 0;
        for (const auto& pair : occurences) {
            d2 += pow(pair.second - expectedNo, 2) / expectedNo;
        }

        // chi2 test has been passed (with p-value < 0.05)
        // thus it can fail about 1/20th of the times
        REQUIRE(d2 < CHI2_TEST_UPPERBOUND);
    }
}

TEST_CASE("Checking dictionary loading text file", "[dictionary][Lib]") {
    TxtDictionary dict{SIMPLE_WORDLIST_FILE};

    // correct getters
    REQUIRE(dict.getFilepath() == SIMPLE_WORDLIST_FILE);

    // dictionary is not loaded and empty at first
    REQUIRE_FALSE(dict.isLoaded());
    REQUIRE(dict.getAllWords().empty());

    // successfully load dictionary
    REQUIRE(dict.load());
    REQUIRE(dict.isLoaded());
    REQUIRE(std::is_sorted(std::cbegin(dict.getAllWords()), std::cend(dict.getAllWords())));
    REQUIRE(dict.getAllWords() == SIMPLE_WORDLIST);
    REQUIRE(dict.contains(SIMPLE_WORDLIST.front()));
    REQUIRE(dict.contains(SIMPLE_WORDLIST.back()));

    SECTION("Cannot load twice a dictionary") {
        REQUIRE_FALSE(dict.load());
        REQUIRE(dict.isLoaded());
    }

    SECTION("Draw random words of dictionary") {
        // only check if random words can be drawn, and belong to the dictionary
        // distribution of draws is checked with the dictionary stub
        for (size_t i = 0; i < TEST_RANDOM_WORDS_N; i++) {
            REQUIRE(dict.contains(dict.getRandomWord(randomGenerator)));
        }
    }
}

TEST_CASE("Check txt dictionary loading an invalid file", "[dictionary][Lib]") {

    SECTION("Loading a file with invalid syntax (1) : non-alpha character") {
        TxtDictionary dict{INVALID1_WORDLIST_FILE};

        // correct getters
        REQUIRE(dict.getFilepath() == INVALID1_WORDLIST_FILE);

        // dictionary is not loaded and empty at first
        REQUIRE_FALSE(dict.isLoaded());
        REQUIRE(dict.getAllWords().empty());

        // assert load dictionary fails
        REQUIRE_FALSE(dict.load());
        REQUIRE_FALSE(dict.isLoaded());
    }

    SECTION("Loading a file with invalid syntax (2) : uppercase character") {
        TxtDictionary dict{INVALID2_WORDLIST_FILE};

        // correct getters
        REQUIRE(dict.getFilepath() == INVALID2_WORDLIST_FILE);

        // dictionary is not loaded and empty at first
        REQUIRE_FALSE(dict.isLoaded());
        REQUIRE(dict.getAllWords().empty());

        // assert load dictionary fails
        REQUIRE_FALSE(dict.load());
        REQUIRE_FALSE(dict.isLoaded());
    }

    SECTION("Loading a file with invalid syntax (3) : duplicated word") {
        TxtDictionary dict{INVALID3_WORDLIST_FILE};

        // correct getters
        REQUIRE(dict.getFilepath() == INVALID3_WORDLIST_FILE);

        // dictionary is not loaded and empty at first
        REQUIRE_FALSE(dict.isLoaded());
        REQUIRE(dict.getAllWords().empty());

        // assert load dictionary fails
        REQUIRE_FALSE(dict.load());
        REQUIRE_FALSE(dict.isLoaded());
    }

    SECTION("Loading an non-existing file") {
        TxtDictionary dict{NOT_A_FILE};

        // correct getters
        REQUIRE(dict.getFilepath() == NOT_A_FILE);

        // dictionary is not loaded and empty at first
        REQUIRE_FALSE(dict.isLoaded());
        REQUIRE(dict.getAllWords().empty());

        // assert load dictionary fails
        REQUIRE_FALSE(dict.load());
        REQUIRE_FALSE(dict.isLoaded());
    }

}

TEST_CASE("Checking fixed size dictionary", "[dictionary][lib]") {
    std::shared_ptr<Dictionary> internalDict = std::make_shared<DictionaryStub>(COMPOSITE_WORDLIST);

    //
    // Check size 1
    //

    FixedSizeDictionary fixed1{internalDict, 1};

    // correct getters
    REQUIRE(fixed1.getWordSize() == 1);

    // dictionary is not loaded and empty at first
    REQUIRE_FALSE(fixed1.isLoaded());
    REQUIRE(fixed1.getAllWords().empty());

    // successfully load dictionary
    REQUIRE(fixed1.load());
    REQUIRE(fixed1.isLoaded());
    REQUIRE(std::is_sorted(std::cbegin(fixed1.getAllWords()), std::cend(fixed1.getAllWords())));
    REQUIRE(fixed1.getAllWords() == COMPOSITE_WORDLIST_1);
    REQUIRE(fixed1.contains(COMPOSITE_WORDLIST_1.front()));
    REQUIRE(fixed1.contains(COMPOSITE_WORDLIST_1.back()));

    // cannot load twice the dictionary (only check once)
    REQUIRE_FALSE(fixed1.load());
    REQUIRE(fixed1.isLoaded());
    REQUIRE(fixed1.getAllWords() == COMPOSITE_WORDLIST_1);

    // Check random words
    // only check if random words can be drawn, and belong to the dictionary
    // distribution of draws is checked with the dictionary stub
    for (size_t i = 0; i < TEST_RANDOM_WORDS_N; i++) {
        REQUIRE(fixed1.contains(fixed1.getRandomWord(randomGenerator)));
    }

    //
    // Check size 2
    //

    FixedSizeDictionary fixed2{internalDict, 2};

    // correct getters
    REQUIRE(fixed2.getWordSize() == 2);

    // dictionary is not loaded and empty at first
    REQUIRE_FALSE(fixed2.isLoaded());
    REQUIRE(fixed2.getAllWords().empty());

    // successfully load dictionary
    REQUIRE(fixed2.load());
    REQUIRE(fixed2.isLoaded());
    REQUIRE(std::is_sorted(std::cbegin(fixed2.getAllWords()), std::cend(fixed2.getAllWords())));
    REQUIRE(fixed2.getAllWords() == COMPOSITE_WORDLIST_2);
    REQUIRE(fixed2.contains(COMPOSITE_WORDLIST_2.front()));
    REQUIRE(fixed2.contains(COMPOSITE_WORDLIST_2.back()));

    //
    // Check size 3
    //

    FixedSizeDictionary fixed3{internalDict, 3};

    // correct getters
    REQUIRE(fixed3.getWordSize() == 3);

    // dictionary is not loaded and empty at first
    REQUIRE_FALSE(fixed3.isLoaded());
    REQUIRE(fixed3.getAllWords().empty());

    // successfully load dictionary
    REQUIRE(fixed3.load());
    REQUIRE(fixed3.isLoaded());
    REQUIRE(std::is_sorted(std::cbegin(fixed3.getAllWords()), std::cend(fixed3.getAllWords())));
    REQUIRE(fixed3.getAllWords() == COMPOSITE_WORDLIST_3);
    REQUIRE(fixed3.contains(COMPOSITE_WORDLIST_3.front()));
    REQUIRE(fixed3.contains(COMPOSITE_WORDLIST_3.back()));

    //
    // Check size 5
    //

    FixedSizeDictionary fixed5{internalDict, 5};

    // correct getters
    REQUIRE(fixed5.getWordSize() == 5);

    // dictionary is not loaded and empty at first
    REQUIRE_FALSE(fixed5.isLoaded());
    REQUIRE(fixed5.getAllWords().empty());

    // successfully load dictionary
    REQUIRE(fixed5.load());
    REQUIRE(fixed5.isLoaded());
    REQUIRE(std::is_sorted(std::cbegin(fixed5.getAllWords()), std::cend(fixed5.getAllWords())));
    REQUIRE(fixed5.getAllWords() == COMPOSITE_WORDLIST_5);
    REQUIRE(fixed5.contains(COMPOSITE_WORDLIST_5.front()));
    REQUIRE(fixed5.contains(COMPOSITE_WORDLIST_5.back()));

    //
    // Check invalid size (no word has this size)
    //
    FixedSizeDictionary fixed4{internalDict, 4};
    REQUIRE(fixed4.getWordSize() == 4);
    REQUIRE_FALSE(fixed4.isLoaded());
    REQUIRE(fixed4.getAllWords().empty());
    REQUIRE_FALSE(fixed4.load());
}

