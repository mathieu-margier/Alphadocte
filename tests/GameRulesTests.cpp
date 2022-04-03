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
 * File: GameRulesTests.cpp
 */

#include <Alphadocte/Exceptions.h>
#include <Alphadocte/Game.h>
#include <Alphadocte/MotusGameRules.h>
#include <Alphadocte/WordleGameRules.h>
#include <catch2/catch.hpp>

#include "TestDefinitions.h"

using namespace Alphadocte;


TEST_CASE("Check motus game rules", "[rules][Lib]") {
    auto motusDict = getMotusDict();

    // Initialize rules
    MotusGameRules rules{motusDict, 6};
    REQUIRE(rules.getMaxGuesses() == 6);
    REQUIRE(rules.getDictionary() == motusDict);

    SECTION("Check solutions") {
        // check valid solution
        REQUIRE(rules.isSolutionValid("defient"));
        REQUIRE(rules.isSolutionValid("mazout"));
        REQUIRE_FALSE(rules.isSolutionValid("rateau")); // not in dictionary
        REQUIRE_FALSE(rules.isSolutionValid("")); // empty word
        REQUIRE_FALSE(rules.isSolutionValid("MAZOUT")); // wrong case
        REQUIRE_FALSE(rules.isSolutionValid("......")); // invalid chars
        REQUIRE_FALSE(rules.isSolutionValid("1ier")); // invalid chars
    }

    SECTION("Check guesses") {
        // check valid guesses
        REQUIRE(rules.isGuessValid("defient", "defient"));
        REQUIRE(rules.isGuessValid("dorment", "defient"));
        REQUIRE_FALSE(rules.isGuessValid("deperi", "defient")); // not same size
        REQUIRE_FALSE(rules.isGuessValid("enferre", "defient")); // not the the same letter
        REQUIRE_FALSE(rules.isGuessValid("demence", "defient")); // not in the dictionary
        REQUIRE_FALSE(rules.isGuessValid("", "defient")); // empty guess
        REQUIRE_FALSE(rules.isGuessValid("enferre", "")); // empty answer

        REQUIRE(rules.isGuessValid("mazout", "mazout"));
        REQUIRE(rules.isGuessValid("metiez", "mazout"));
        REQUIRE_FALSE(rules.isGuessValid("mafiosi", "mazout")); // not same size
        REQUIRE_FALSE(rules.isGuessValid("robera", "mazout")); // not the the same letter
        REQUIRE_FALSE(rules.isGuessValid("maison", "mazout")); // not in the dictionary
        REQUIRE_FALSE(rules.isGuessValid("", "mazout")); // empty guess
        REQUIRE_FALSE(rules.isGuessValid("robera", "")); // empty answer
    }

    SECTION("Check templates") {
        using Catch::Matchers::Message;
        std::shared_ptr<IGameRules> rulesPtr = std::make_shared<MotusGameRules>(rules);
        Game game{rulesPtr};

        REQUIRE_THROWS_MATCHES(rulesPtr->getTemplate(game), InvalidArgException, Message("no word has been set."));

        game.setWord("defient");
        REQUIRE(rulesPtr->getTemplate(game) == "d......");

        game.setWord("mazout");
        REQUIRE(rulesPtr->getTemplate(game) == "m.....");
    }

    SECTION("Change max guesses") {
        // test setter
        rules.setMaxGuesses(5);
        REQUIRE(rules.getMaxGuesses() == 5);

        // cannot set 0
        rules.setMaxGuesses(0);
        REQUIRE(rules.getMaxGuesses() == 0);
    }
}

TEST_CASE("Check wordle game rules", "[rules][Lib]") {
    auto wordleDict = getWordleDict();

    // Initialize rules
    WordleGameRules rules{wordleDict, 6};
    REQUIRE(rules.getMaxGuesses() == 6);
    REQUIRE(rules.getDictionary() == wordleDict);

    SECTION("Check solutions") {
        // check valid solution
        REQUIRE(rules.isSolutionValid("agaca"));
        REQUIRE(rules.isSolutionValid("lobby"));
        REQUIRE_FALSE(rules.isSolutionValid("")); // empty word
        REQUIRE_FALSE(rules.isSolutionValid("rateau")); // wrong size
        REQUIRE_FALSE(rules.isSolutionValid("mange")); // not in dictionary
        REQUIRE_FALSE(rules.isSolutionValid("LOBBY")); // wrong case
        REQUIRE_FALSE(rules.isSolutionValid(".....")); // invalid chars
        REQUIRE_FALSE(rules.isSolutionValid("1iers")); // invalid chars
    }

    SECTION("Check guesses") {
        // check valid guesses
        REQUIRE(rules.isGuessValid("agaca", "agaca"));
        REQUIRE(rules.isGuessValid("amont", "agaca"));
        REQUIRE(rules.isGuessValid("lobby", "agaca"));
        REQUIRE_FALSE(rules.isGuessValid("rateau", "agaca")); // not same size
        REQUIRE_FALSE(rules.isGuessValid("mange", "agaca")); // not in dictionary
        REQUIRE_FALSE(rules.isGuessValid("", "agaca")); // empty guess
        REQUIRE_FALSE(rules.isGuessValid("amont", "")); // empty answer
    }

    SECTION("Check templates") {
        using Catch::Matchers::Message;
        std::shared_ptr<IGameRules> rulesPtr = std::make_shared<WordleGameRules>(rules);
        Game game{rulesPtr};

        REQUIRE_THROWS_MATCHES(rulesPtr->getTemplate(game), InvalidArgException, Message("no word has been set."));

        game.setWord("agaca");
        REQUIRE(rulesPtr->getTemplate(game) == ".....");

        game.setWord("lobby");
        REQUIRE(rulesPtr->getTemplate(game) == ".....");
    }

    SECTION("Change max guesses") {
        // test setter
        rules.setMaxGuesses(5);
        REQUIRE(rules.getMaxGuesses() == 5);

        // set unlimited
        rules.setMaxGuesses(0);
        REQUIRE(rules.getMaxGuesses() == 0);
    }
}
