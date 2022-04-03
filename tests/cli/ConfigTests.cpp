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
 * File: cli/ConfigTests.cpp
 */

#include <Alphadocte/Exceptions.h>
#include <catch2/catch.hpp>

#include "../../apps/Config.h"
#include "../TestDefinitions.h"

using Catch::Message;
using namespace Alphadocte;
using namespace Alphadocte::CLI;

// CONSTANTS
static const Section CONFIG1_SECTION{
    .name = "root",
    .entries = {
        Entry("file_path", "/usr/local/share/motus-solver/fr_dict.txt"),
        Entry("file_timestamp", "123456789")
    },
    .sections = {
        Section{
            .name = "solver_entry",
            .entries ={
                Entry("solver_name", "xxxxx"),
                Entry("solver_version", "1")
            },
            .sections = {
                Section{
                    .name = "guess_entry",
                    .entries = {
                        Entry("template", "....."),
                        Entry("guess", "abdef 5.99"),
                        Entry("guess", "defgh 4.98")
                    }
                }
            }
        }
    }
};

TEST_CASE("Test changing root section", "[config][CLI]") {
    Config config;
    Section& rootSection = config.getRootSection();
    REQUIRE(rootSection == Section{.name="root"});

    rootSection.entries.push_back({"entry", "value"});
    REQUIRE(rootSection == Section{.name="root", .entries={Entry("entry", "value")}});

    rootSection.sections.push_back(Section{.name="empty_section"});
    REQUIRE(rootSection == Section{.name="root", .entries={Entry("entry", "value")}, .sections={Section{.name="empty_section"}}});

    config.setRootSection(Section{});
    REQUIRE(config.getRootSection() == Section{});

    Section s1{
        .name="s1",
        .entries={Entry("e1", "v1"), Entry("e2", "v2")}
    };
    config.setRootSection(s1);
    REQUIRE(config.getRootSection() == s1);

    Section s2{
        .name="s2",
        .entries={Entry("e1", "v1"),},
        .sections={Section{.name="ss1", .entries={Entry("e2", "v2")}}, Section{}}
    };
    config.setRootSection(s2);
    REQUIRE(config.getRootSection() == s2);
}

TEST_CASE("Loading config from files", "[config][CLI]") {
    Config config;
    REQUIRE(config.getRootSection() == Section{.name="root"});

    SECTION("Testing example 1") {
        REQUIRE_NOTHROW(config.loadFromFile(TEST_CONFIG_EXAMPLE1));
        REQUIRE(config.getRootSection() == CONFIG1_SECTION);

        config.clear();
        REQUIRE(config.getRootSection() == Section{.name="root"});

        REQUIRE_NOTHROW(config.loadFromFile(TEST_CONFIG_EXAMPLE1_COPY));
        REQUIRE(config.getRootSection() == CONFIG1_SECTION);
    }

    SECTION("Loading invalid files") {
        Section section{.name="someSection", .entries={Entry("a", "b")}};
        config.setRootSection(section);

        REQUIRE_THROWS_MATCHES(config.loadFromFile(TEST_CONFIG_DIR / "invalid_file"), Exception, Message("File " + (TEST_CONFIG_DIR / "invalid_file").string() + " either does not exist, is not a file, or is not accessible."));
        REQUIRE(config.getRootSection() == section);

        REQUIRE_THROWS_MATCHES(config.loadFromFile(TEST_CONFIG_BAD_ENTRY), Exception, Message("Line 13: key error must have a value associated."));
        REQUIRE(config.getRootSection() == section);

        REQUIRE_THROWS_MATCHES(config.loadFromFile(TEST_CONFIG_BAD_ENTRY_COMMENT), Exception, Message("Line 13: key error must have a value associated."));
        REQUIRE(config.getRootSection() == section);

        REQUIRE_THROWS_MATCHES(config.loadFromFile(TEST_CONFIG_BAD_SECTION1), Exception, Message("Line 13: ending section with a different name (got random_entry, expected guess_entry)."));
        REQUIRE(config.getRootSection() == section);

        REQUIRE_THROWS_MATCHES(config.loadFromFile(TEST_CONFIG_BAD_SECTION2), Exception, Message("Reached end of file without closing section solver_entry begun at line 5."));
        REQUIRE(config.getRootSection() == section);
    }
}

TEST_CASE("Writing config files", "[config][CLI]") {
    Config config;
    REQUIRE_NOTHROW(std::filesystem::create_directories(TEST_OUT_DIR));

    SECTION("Testing example 1") {
        config.setRootSection(CONFIG1_SECTION);
        REQUIRE(config.getRootSection() == CONFIG1_SECTION);

        std::filesystem::path outputFile = TEST_OUT_DIR / "out_config.txt";

        if (std::filesystem::is_regular_file(outputFile)) {
            REQUIRE(std::filesystem::remove(outputFile));
        }

        REQUIRE_NOTHROW(config.writeToFile(outputFile));
        REQUIRE(files_identical(outputFile, TEST_CONFIG_EXAMPLE1_COPY));
    }
}
