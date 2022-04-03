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
 * File: HintTests.cpp
 */

#include <Alphadocte/Hint.h>
#include <sstream>

#include <catch2/catch.hpp>


using namespace Alphadocte;

TEST_CASE("Check hint operators", "[hint][Lib]") {
    std::ostringstream oss;
    REQUIRE(oss.str().empty());

    SECTION("Correct letter") {
        oss << HintType::CORRECT;

        REQUIRE(oss.str() == "correct");
    }

    SECTION("Misplaced letter") {
        oss << HintType::MISPLACED;

        REQUIRE(oss.str() == "not here");
    }

    SECTION("Wrong letter") {
        oss << HintType::WRONG;

        REQUIRE(oss.str() == "wrong");
    }
}

TEST_CASE("Check hint matching", "[hint][Lib]") {
    REQUIRE(matches("marie", "tarie", {
        HintType::WRONG, HintType::CORRECT, HintType::CORRECT, HintType::CORRECT, HintType::CORRECT
    }));
    REQUIRE_FALSE(matches("marie", "tarie", {
        HintType::CORRECT, HintType::CORRECT, HintType::CORRECT, HintType::CORRECT, HintType::CORRECT
    }));

    REQUIRE(matches("tarie", "tarie", {
        HintType::CORRECT, HintType::CORRECT, HintType::CORRECT, HintType::CORRECT, HintType::CORRECT
    }));
    REQUIRE_FALSE(matches("tarie", "tarie", {
        HintType::WRONG, HintType::CORRECT, HintType::CORRECT, HintType::CORRECT, HintType::CORRECT
    }));

    REQUIRE(matches("email", "maree", {
        HintType::MISPLACED, HintType::MISPLACED, HintType::WRONG, HintType::MISPLACED, HintType::WRONG
    }));
    REQUIRE_FALSE(matches("email", "maree", {
        HintType::MISPLACED, HintType::MISPLACED, HintType::WRONG, HintType::MISPLACED, HintType::MISPLACED
    }));

    REQUIRE(matches("maree", "email", {
        HintType::MISPLACED, HintType::MISPLACED, HintType::MISPLACED, HintType::WRONG, HintType::WRONG
    }));
    REQUIRE_FALSE(matches("maree", "email", {
        HintType::MISPLACED, HintType::MISPLACED, HintType::MISPLACED, HintType::MISPLACED, HintType::MISPLACED
    }));
    REQUIRE_FALSE(matches("maree", "email", {
        HintType::MISPLACED, HintType::MISPLACED, HintType::MISPLACED, HintType::CORRECT, HintType::WRONG
    }));

}

