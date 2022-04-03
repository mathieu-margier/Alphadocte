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
 * File: CacheConfigTests.cpp
 */

#include <Alphadocte/Exceptions.h>
#include <catch2/catch.hpp>

#include "../TestDefinitions.h"
#include "../../apps/CacheConfig.h"

using namespace std::string_literals;
using Catch::Message;
using namespace Alphadocte;
using namespace Alphadocte::CLI;

constexpr char SOLVER_NAME[] = "entropy_maximizer";
constexpr unsigned int SOLVER_VERSION = 1;

static const Section CACHE1_SECTION{"root", {
        Entry("file_path", "/dataPath/fr_wordlist.txt"),
        Entry("file_timestamp", "-4793217213896459979")
    }, {
        Section("solver_entry", {
            Entry("solver_name", SOLVER_NAME),
            Entry("solver_version", std::to_string(SOLVER_VERSION))
        }, {
            Section("guess_entry", {
                Entry("template", "....."),
                Entry("requested_number", "3"),
                Entry("guess", "raies 6.342236"),
                Entry("guess", "taies 6.299622"),
                Entry("guess", "tarie 6.299398")
            }, {}),
            Section("guess_entry", {
                Entry("template", "s......"),
                Entry("requested_number", "3"),
                Entry("guess", "sauries 7.654206"),
                Entry("guess", "sorties 7.544587"),
                Entry("guess", "surites 7.533810")
            }, {}),
            Section("guess_entry", {
                Entry("template", "f........"),
                Entry("requested_number", "3"),
                Entry("guess", "fruitiees 9.514804"),
                Entry("guess", "feutrines 9.460037"),
                Entry("guess", "fourniees 9.453263")
            }, {})
        })
    }
};

TEST_CASE("Intialising an empty CacheConfig", "[config][CLI]") {
    CacheConfig cache{TEST_WORDLE_WORDS};
    auto timestamp = std::filesystem::last_write_time(TEST_WORDLE_WORDS);

    REQUIRE(cache.getConfig().getRootSection() == Section("root", {
            Entry("file_path", std::filesystem::absolute(TEST_WORDLE_WORDS).string()),
            Entry("file_timestamp", std::to_string(timestamp.time_since_epoch().count()))
    }, {}));
    REQUIRE(cache.isCacheValid());
    REQUIRE(cache.getDictionaryPath().is_absolute());
    REQUIRE(cache.getDictionaryPath() == std::filesystem::absolute(TEST_WORDLE_WORDS));
    REQUIRE(cache.getDictionaryTimestamp() == timestamp);

    // calling constructor on a non existing file
    REQUIRE_THROWS_MATCHES(CacheConfig(TEST_OUT_DIR / "invalid_file"), Exception, Message("Dictionary at " + (TEST_OUT_DIR / "invalid_file").string() + " is not a file."));
}

TEST_CASE("Checking CacheConfig getters/setters", "[config][CLI]") {
    // Make cache entry valid
    Section section = CACHE1_SECTION;
    section.entries.at(0).value = std::filesystem::absolute(TEST_WORDLE_WORDS).string();
    section.entries.at(1).value = std::to_string(std::filesystem::last_write_time(TEST_WORDLE_WORDS).time_since_epoch().count());

    Config config;
    config.setRootSection(section);

    CacheConfig cache{config};
    REQUIRE(cache.isCacheValid());
    REQUIRE(cache.getConfig().getRootSection() == section);

    SECTION("Retrieve and modify top guesses") {
        // check existing guess
        REQUIRE(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 3, ".....") == std::vector{
            std::pair("raies"s, 6.342236),
            std::pair("taies"s, 6.299622),
            std::pair("tarie"s, 6.299398)
        });
        REQUIRE(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, ".....") == std::vector{
            std::pair("raies"s, 6.342236)
        });

        REQUIRE(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 3, "s......") == std::vector{
            std::pair("sauries"s, 7.654206),
            std::pair("sorties"s, 7.544587),
            std::pair("surites"s, 7.533810)
        });
        REQUIRE(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "s......") == std::vector{
            std::pair("sauries"s, 7.654206)
        });

        REQUIRE(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 3, "f........") == std::vector{
            std::pair("fruitiees"s, 9.514804),
            std::pair("feutrines"s, 9.460037),
            std::pair("fourniees"s, 9.453263)
        });
        REQUIRE(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "f........") == std::vector{
            std::pair("fruitiees"s, 9.514804)
        });

        // add section
        REQUIRE_NOTHROW(cache.setTopGuesses(SOLVER_NAME, SOLVER_VERSION, "a....", 1, std::vector{
            std::pair("antre"s, 5.1)
        }));
        REQUIRE(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "a....") == std::vector{
            std::pair("antre"s, 5.1)
        });

        // replace section
        REQUIRE_NOTHROW(cache.setTopGuesses(SOLVER_NAME, SOLVER_VERSION, "a....", 3, std::vector{
            std::pair("antre"s, 5.1),
            std::pair("autre"s, 5.0),
            std::pair("arbre"s, 4.5)
        }));
        REQUIRE(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 3, "a....") == std::vector{
            std::pair("antre"s, 5.1),
            std::pair("autre"s, 5.0),
            std::pair("arbre"s, 4.5)
        });
    }

    SECTION("Checking getter from invalid/incomplete cache.") {
        Config localConfig;
        Section& localSection = localConfig.getRootSection();
        localSection = cache.getConfig().getRootSection();

        Section& solverSection = localSection.sections.at(0);
        CHECK(solverSection.name == "solver_entry");
        CHECK(solverSection.entries.at(0).name == "solver_name");
        CHECK(solverSection.entries.at(0).value == std::string(SOLVER_NAME));
        CHECK(solverSection.entries.at(1).name == "solver_version");
        CHECK(solverSection.entries.at(1).value == std::to_string(SOLVER_VERSION));

        Section& guessSection = solverSection.sections.at(0);
        CHECK(guessSection.name == "guess_entry");
        CHECK(guessSection.entries.at(0).name == "template");
        CHECK(guessSection.entries.at(0).value == ".....");

        // not enough guesses
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 10, "....."),
                Exception, Message("Not enough guesses in cache."));

        // non existing guess entry with this template
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "......"),
                Exception, Message("Guess section with template \"......\" not found."));

        // invalid guess in guess entry

        CHECK(guessSection.entries.at(2).name == "guess");
        guessSection.entries.at(2).value = "abcde"; // no space
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "....."),
                Exception, Message("Values of entry guess must be separated by a space."));

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        CHECK(guessSection.entries.at(2).name == "guess");
        guessSection.entries.at(2).value = "abcde az"; // second value is not a double
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "....."),
                Exception, Message("Guess trust value (az) cannot be parsed as a number."));

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        CHECK(guessSection.entries.at(2).name == "guess");
        guessSection.entries.at(2).value = "abcdef 4.2"; // invalid guess (no. of letters)
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "....."),
                Exception, Message("Guess abcdef does not have the same number of letters as template \".....\"."));

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        CHECK(guessSection.entries.at(0).name == "template");
        guessSection.entries.at(0).value = "aa...";
        CHECK(guessSection.entries.at(2).name == "guess");
        guessSection.entries.at(2).value = "abcde 4.2"; // invalid guess (does not match template)
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "aa..."),
                Exception, Message("Guess abcde does not match the template \"aa...\"."));

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        CHECK(guessSection.entries.at(2).name == "guess");
        guessSection.entries.at(2).value = "ab4de 4.2"; // invalid guess (forbidden chars)
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "....."),
                Exception, Message("Guess ab4de contains invalid characters."));

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        // invalid request number entry

        CHECK(guessSection.entries.at(1).name == "requested_number");
        guessSection.entries.at(1).value = "abc"; // not a number
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "....."),
                Exception, Message("Invalid value for entry requested_number : abc is not a positive integer."));

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        CHECK(guessSection.entries.at(1).name == "requested_number");
        guessSection.entries.at(1).name = "not_requested_number"; // missing request number entry
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "....."),
                Exception, Message("Missing entry requested_number in section guess with template ....."));

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        // invalid solver section

        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, 2, 1, "....."), // different version requested
                Exception, Message("Actual solver version is different from entry's solver_version : got 1, expected 2."));

        CHECK(solverSection.entries.at(1).name == "solver_version");
        solverSection.entries.at(1).value = "abc"; // not a number
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "....."),
                Exception, Message("Invalid value for entry solver_version : abc is not a positive integer."));

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        CHECK(solverSection.entries.at(1).name == "solver_version");
        solverSection.entries.at(1).name = "not_solver_version"; // missing solver version entry
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "....."),
                Exception, Message("Missing entry solver_version in section solver " + std::string(SOLVER_NAME) + '.'));

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        // non existing solver entry with this solver name

        CHECK(solverSection.entries.at(0).name == "solver_name");
        solverSection.entries.at(0).value = "some random solver name"; // another solver name
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "......"),
                Exception, Message("Solver section with name \"" + std::string(SOLVER_NAME) + "\" not found."));

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        CHECK(solverSection.name == "solver_entry");
        solverSection.name = "not_solver_entry"; // missing solver section
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS_MATCHES(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 1, "......"),
                Exception, Message("Solver section with name \"" + std::string(SOLVER_NAME) + "\" not found."));
    }

    SECTION("Overwrite invalid sections") {
        Config localConfig;
        Section& localSection = localConfig.getRootSection();
        localSection = cache.getConfig().getRootSection();

        Section& solverSection = localSection.sections.at(0);
        CHECK(solverSection.name == "solver_entry");
        CHECK(solverSection.entries.at(0).name == "solver_name");
        CHECK(solverSection.entries.at(0).value == std::string(SOLVER_NAME));
        CHECK(solverSection.entries.at(1).name == "solver_version");
        CHECK(solverSection.entries.at(1).value == std::to_string(SOLVER_VERSION));

        Section& guessSection = solverSection.sections.at(0);
        CHECK(guessSection.name == "guess_entry");
        CHECK(guessSection.entries.at(0).name == "template");
        CHECK(guessSection.entries.at(0).value == ".....");

        // invalid guess entry
        CHECK(guessSection.entries.at(2).name == "guess");
        guessSection.entries.at(2).value = "invalid value";
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 3, "....."));
        cache.setTopGuesses(SOLVER_NAME, SOLVER_VERSION, ".....", 3, {
            {"raies"s, 6.342236}, {"taies"s, 6.299622}, {"tarie"s, 6.299398}
        });
        REQUIRE(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 3, ".....") == std::vector<std::pair<std::string, double>>{
            {"raies"s, 6.342236}, {"taies"s, 6.299622}, {"tarie"s, 6.299398}
        });

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        // no guess entry
        solverSection.sections.clear();
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 3, "....."));
        cache.setTopGuesses(SOLVER_NAME, SOLVER_VERSION, ".....", 3, {
            {"raies"s, 6.342236}, {"taies"s, 6.299622}, {"tarie"s, 6.299398}
        });
        REQUIRE(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 3, ".....") == std::vector<std::pair<std::string, double>>{
            {"raies"s, 6.342236}, {"taies"s, 6.299622}, {"tarie"s, 6.299398}
        });

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        // invalid solver entry
        CHECK(solverSection.entries.at(1).name == "solver_version");
        solverSection.entries.at(1).value = "abc";
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 3, "....."));
        cache.setTopGuesses(SOLVER_NAME, SOLVER_VERSION, ".....", 3, {
            {"raies"s, 6.342236}, {"taies"s, 6.299622}, {"tarie"s, 6.299398}
        });
        REQUIRE(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 3, ".....") == std::vector<std::pair<std::string, double>>{
            {"raies"s, 6.342236}, {"taies"s, 6.299622}, {"tarie"s, 6.299398}
        });

        // restore state
        cache.setConfig(config);
        localSection = config.getRootSection();

        // no solver entry
        localSection.sections.clear();
        REQUIRE_NOTHROW(cache.setConfig(localConfig));
        REQUIRE_THROWS(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 3, "....."));
        cache.setTopGuesses(SOLVER_NAME, SOLVER_VERSION, ".....", 3, {
            {"raies"s, 6.342236}, {"taies"s, 6.299622}, {"tarie"s, 6.299398}
        });
        REQUIRE(cache.getTopGuesses(SOLVER_NAME, SOLVER_VERSION, 3, ".....") == std::vector<std::pair<std::string, double>>{
            {"raies"s, 6.342236}, {"taies"s, 6.299622}, {"tarie"s, 6.299398}
        });
    }


    SECTION("Setting an invalid cache entry") {
        Config invalidConfig;
        Section& invalidSection = invalidConfig.getRootSection();

        // missing both required entries
        REQUIRE_THROWS_MATCHES(cache.setConfig(invalidConfig), InvalidArgException, Message("Invalid configuration supplied as cache."));
        REQUIRE_THROWS_MATCHES(CacheConfig(invalidConfig), InvalidArgException, Message("Invalid configuration supplied as cache."));
        REQUIRE(cache.getConfig().getRootSection() == section);

        // missing file_timestamp
        invalidSection.entries.push_back(section.entries.at(0));
        REQUIRE_THROWS_MATCHES(cache.setConfig(invalidConfig), InvalidArgException, Message("Invalid configuration supplied as cache."));
        REQUIRE_THROWS_MATCHES(CacheConfig(invalidConfig), InvalidArgException, Message("Invalid configuration supplied as cache."));
        REQUIRE(cache.getConfig().getRootSection() == section);

        // missing file_path
        invalidSection.entries.at(0) = section.entries.at(1);
        REQUIRE_THROWS_MATCHES(cache.setConfig(invalidConfig), InvalidArgException, Message("Invalid configuration supplied as cache."));
        REQUIRE_THROWS_MATCHES(CacheConfig(invalidConfig), InvalidArgException, Message("Invalid configuration supplied as cache."));
        REQUIRE(cache.getConfig().getRootSection() == section);

        // non existing file
        invalidSection.entries = section.entries;
        invalidSection.entries.at(0).value = (TEST_OUT_DIR / "invalid_file").string();
        REQUIRE_THROWS_MATCHES(cache.setConfig(invalidConfig), InvalidArgException, Message("Invalid configuration supplied as cache."));
        REQUIRE_THROWS_MATCHES(CacheConfig(invalidConfig), InvalidArgException, Message("Invalid configuration supplied as cache."));
        REQUIRE(cache.getConfig().getRootSection() == section);

        // outdated timestamp
        invalidSection.entries = section.entries;
        invalidSection.entries.at(1).value = std::to_string(std::filesystem::file_time_type::min().time_since_epoch().count());
        REQUIRE_THROWS_MATCHES(cache.setConfig(invalidConfig), InvalidArgException, Message("Invalid configuration supplied as cache."));
        REQUIRE_THROWS_MATCHES(CacheConfig(invalidConfig), InvalidArgException, Message("Invalid configuration supplied as cache."));
        REQUIRE(cache.getConfig().getRootSection() == section);
    }
}

TEST_CASE("Loading/writing CacheConfig from/to file", "[config][CLI]") {
    Config config;

    // parsing
    REQUIRE_NOTHROW(config.loadFromFile(TEST_CONFIG_CACHE));
    REQUIRE(config.getRootSection() == CACHE1_SECTION);

    // writing
    REQUIRE_NOTHROW(std::filesystem::create_directories(TEST_OUT_DIR));
    std::filesystem::path outputFile = TEST_OUT_DIR / "out_config.txt";

    if (std::filesystem::is_regular_file(outputFile)) {
        REQUIRE(std::filesystem::remove(outputFile));
    }

    REQUIRE_NOTHROW(config.writeToFile(outputFile));
    REQUIRE(files_identical(outputFile, TEST_CONFIG_CACHE));
}

TEST_CASE("Populating a CacheConfig from scratch", "[config][CLI]") {
    CacheConfig cache{TEST_WORDLE_WORDS};

    cache.setTopGuesses(SOLVER_NAME, SOLVER_VERSION, ".....", 3, {
        {"raies"s, 6.342236}, {"taies"s, 6.299622}, {"tarie"s, 6.299398}
    });
    cache.setTopGuesses(SOLVER_NAME, SOLVER_VERSION, "s......", 3, {
        {"sauries"s, 7.654206}, {"sorties"s, 7.544587}, {"surites"s, 7.533810}
    });
    cache.setTopGuesses(SOLVER_NAME, SOLVER_VERSION, "f........", 3, {
        {"fruitiees"s, 9.514804}, {"feutrines"s, 9.460037}, {"fourniees"s, 9.453263}
    });

    // Make dictionary matches cache's one
    Section expectedSection = CACHE1_SECTION;
    expectedSection.entries.at(0).value = std::filesystem::absolute(TEST_WORDLE_WORDS).string();
    expectedSection.entries.at(1).value = std::to_string(std::filesystem::last_write_time(TEST_WORDLE_WORDS).time_since_epoch().count());

    REQUIRE(cache.getConfig().getRootSection() == expectedSection);
}
