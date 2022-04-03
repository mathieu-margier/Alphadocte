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
 * File: cli/CommonTests.cpp
 */

#include <cstdlib>
#include <filesystem>

#include <catch2/catch.hpp>

#include "../../apps/Common.h"

using namespace Alphadocte::CLI;

#if defined ALPHADOCTE_OS_LINUX
static const char* const XDG_DATA_ENV_VALUE = getenv("XDG_DATA_DIRS");
static const char* const XDG_CACHE_ENV_VALUE = getenv("XDG_CACHE_HOME");
#elif defined ALPHADOCTE_OS_WINDOWS
#include <windows.h>
#else
#error "Unsupported OS"
#endif

static const std::filesystem::path WORKING_DIR = std::filesystem::current_path();

void restoreEnv();

TEST_CASE("Checking application folders") {
    restoreEnv();

    SECTION("Testing data folder") {
#if defined ALPHADOCTE_OS_LINUX
        unsetenv("XDG_DATA_DIRS");

        // current dir contains data, must be this folder
        REQUIRE(getDataPath(true) == std::filesystem::absolute(WORKING_DIR / "data"));

        setenv("XDG_DATA_DIRS", std::filesystem::absolute(WORKING_DIR / "data").c_str(), true);
        REQUIRE(getDataPath(false) == std::filesystem::absolute(WORKING_DIR / "data")); // check cache

        // current dir contains data, must be this folder, ignore xdg env var
        REQUIRE(getDataPath(true) == std::filesystem::absolute(WORKING_DIR / "data"));

        // set working dir so that there is no "data" folder
        std::filesystem::current_path(WORKING_DIR / "data");
        REQUIRE(getDataPath(false) == std::filesystem::absolute(WORKING_DIR / "data")); // check cache

        REQUIRE(getDataPath(true) == std::filesystem::absolute(WORKING_DIR / "data" / "alphadocte"));
        REQUIRE(getDataPath(false) == std::filesystem::absolute(WORKING_DIR / "data" / "alphadocte")); // check cache
#elif defined ALPHADOCTE_OS_WINDOWS
        // current dir contains data, must be this folder
        REQUIRE(getDataPath(true) == std::filesystem::absolute(WORKING_DIR / "data"));

        // move somewhere else, no more 'data' folder here
        std::filesystem::current_path(WORKING_DIR / "data");
        REQUIRE(getDataPath(false) == std::filesystem::absolute(WORKING_DIR / "data")); // check cache

        // still points to the previous path since executable is located inside the initial working dir
        REQUIRE(getDataPath(true) == std::filesystem::absolute(WORKING_DIR / "data"));
        REQUIRE(getDataPath(false) == std::filesystem::absolute(WORKING_DIR / "data")); // check cache
#else
#error "Unsupported OS"
#endif
    }

    SECTION("Testing cache folder") {
#if defined ALPHADOCTE_OS_LINUX
        unsetenv("XDG_CACHE_HOME");

        const char* const home = getenv("HOME");

        if (!home)
            FAIL("HOME environment variable must be set.");

        // use home
        REQUIRE(getCachePath(true) == std::filesystem::absolute(std::filesystem::path(home) / ".cache" / "alphadocte"));
        REQUIRE(std::filesystem::is_directory(getCachePath(false)));

        setenv("XDG_CACHE_HOME", std::filesystem::absolute(WORKING_DIR / "cache").c_str(), true);
        REQUIRE(getCachePath(false) == std::filesystem::absolute(std::filesystem::path(home) / ".cache" / "alphadocte")); // check cache
        REQUIRE(getCachePath(true) == std::filesystem::absolute(WORKING_DIR / "cache" / "alphadocte"));
        REQUIRE(std::filesystem::is_directory(getCachePath(false)));
        REQUIRE(getCachePath(false) == std::filesystem::absolute(WORKING_DIR / "cache" / "alphadocte")); // check cache
#elif defined ALPHADOCTE_OS_WINDOWS
        const char* const appdata = std::getenv("LOCALAPPDATA");

        if (!appdata)
            FAIL("LOCALAPPDATA environment variable must be set.");

        REQUIRE(getCachePath(true) == std::filesystem::absolute(std::filesystem::path(appdata) / "alphadocte" / "cache"));
        REQUIRE(std::filesystem::is_directory(getCachePath(false)));
        REQUIRE(getCachePath(false) == std::filesystem::absolute(std::filesystem::path(appdata) / "alphadocte" / "cache")); // check cache
#else
#error "Unsupported OS"
#endif
    }

    restoreEnv();
}

void restoreEnv() {
    // restore working dir
    std::filesystem::current_path(WORKING_DIR);

    // Restore env
#if defined ALPHADOCTE_OS_LINUX
    if (XDG_DATA_ENV_VALUE) {
        setenv("XDG_DATA_DIRS", XDG_DATA_ENV_VALUE, true);
    } else {
        unsetenv("XDG_DATA_DIRS");
    }

    if (XDG_CACHE_ENV_VALUE) {
        setenv("XDG_CACHE_HOME", XDG_CACHE_ENV_VALUE, true);
    } else {
        unsetenv("XDG_CACHE_HOME");
    }
#elif defined ALPHADOCTE_OS_WINDOWS
    // Nothing to restore
#else
#error "Unsupported OS"
#endif
}
