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
 * File: SolverTests.cpp
 */

#include <Alphadocte/Exceptions.h>
#include <Alphadocte/Hint.h>
#include <Alphadocte/MotusGameRules.h>
#include <Alphadocte/WordleGameRules.h>
#include <algorithm>
#include <memory>

#include <catch2/catch.hpp>

#include "TestDefinitions.h"
#include "stubs/SolverStub.h"

using Catch::Message;
using namespace Alphadocte;
using enum HintType;

TEST_CASE("Creating solver with invalid rules", "[solver][Lib]") {
    REQUIRE_THROWS_MATCHES(SolverStub({}), InvalidArgException, Message("rules cannot be null"));
}

TEST_CASE("Testing solver base class", "[solver][Lib]") {
    const auto& wordleDict = getWordleDict();
    std::shared_ptr<IGameRules> rules = std::make_shared<WordleGameRules>(wordleDict);
    SolverStub solver{rules};

    // check constructor
    REQUIRE(solver.getRules() == rules);
    REQUIRE(solver.getSolverName() == "SolverStub");
    REQUIRE(solver.getSolverVersion() == 1);
    REQUIRE(solver.getHints().empty());
    REQUIRE(solver.getPotentialGuesses().empty());
    REQUIRE(solver.getPotentialSolutions().empty());
    REQUIRE(solver.getTemplate().empty());

    SECTION("Changing rules") {
        std::shared_ptr<IGameRules> otherRules = std::make_shared<MotusGameRules>(getMotusDict());

        // change nothing
        solver.setRules(rules);
        REQUIRE(solver.getRules() == rules);

        solver.setRules({});
        REQUIRE(solver.getRules() == rules);

        // change rules with default state
        solver.setRules(otherRules);
        REQUIRE(solver.getRules() == otherRules);

        // back to origina rules
        solver.setRules(rules);
        REQUIRE(solver.getRules() == rules);

        // modify solver state
        solver.setTemplate(std::string(wordleDict->getWordSize(), '.'));
        solver.addHint(wordleDict->getAllWords().front(), std::vector(wordleDict->getWordSize(), WRONG));
        solver.addHint(wordleDict->getAllWords().back(), std::vector(wordleDict->getWordSize(), WRONG));

        auto hints = solver.getHints();
        auto guesses = solver.getPotentialGuesses();
        auto solutions = solver.getPotentialSolutions();
        auto templateStr = solver.getTemplate();

        // change nothing
        solver.setRules(rules);
        REQUIRE(solver.getRules() == rules);
        REQUIRE(solver.getHints() == hints);
        REQUIRE(solver.getPotentialGuesses() == guesses);
        REQUIRE(solver.getPotentialSolutions() == solutions);
        REQUIRE(solver.getTemplate() == templateStr);

        solver.setRules({});
        REQUIRE(solver.getRules() == rules);
        REQUIRE(solver.getRules() == rules);
        REQUIRE(solver.getHints() == hints);
        REQUIRE(solver.getPotentialGuesses() == guesses);
        REQUIRE(solver.getPotentialSolutions() == solutions);
        REQUIRE(solver.getTemplate() == templateStr);

        solver.setRules(otherRules);
        REQUIRE(solver.getRules() == otherRules);
        REQUIRE(solver.getHints().empty());
        REQUIRE(solver.getPotentialGuesses().empty());
        REQUIRE(solver.getPotentialSolutions().empty());
        REQUIRE(solver.getTemplate().empty());
    }

    SECTION("Set template word (wordle)") {
        std::vector<std::string_view> allWordsViews;
        std::transform(std::cbegin(wordleDict->getAllWords()), std::cend(wordleDict->getAllWords()),
                std::back_inserter(allWordsViews), [](const auto& str) {
            return std::string_view(str);
        });

        // invalid templates
        REQUIRE_THROWS_MATCHES(solver.setTemplate("abc45"), InvalidArgException, Message("invalid template, must contain either '.' or letters."));
        REQUIRE(solver.getTemplate().empty());

        REQUIRE_THROWS_MATCHES(solver.setTemplate("....!"), InvalidArgException, Message("invalid template, must contain either '.' or letters."));
        REQUIRE(solver.getTemplate().empty());

        // accept all words
        REQUIRE_NOTHROW(solver.setTemplate("....."));
        REQUIRE(solver.getTemplate() == ".....");
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == allWordsViews);

        // accept some words
        REQUIRE_NOTHROW(solver.setTemplate("a...."));
        REQUIRE(solver.getTemplate() == "a....");
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"agaca", "aient", "amont", "appat", "arroi"});

        REQUIRE_NOTHROW(solver.setTemplate("A....")); // invalid case should be corrected
        REQUIRE(solver.getTemplate() == "a....");
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"agaca", "aient", "amont", "appat", "arroi"});

        REQUIRE_NOTHROW(solver.setTemplate(".i.i."));
        REQUIRE(solver.getTemplate() == ".i.i.");
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"divin", "finie"});
    }

    SECTION("Set template word (motus)") {
        const auto& motusDict = getMotusDict();
        std::shared_ptr<IGameRules> motusRules = std::make_shared<MotusGameRules>(motusDict);
        solver.setRules(motusRules);
        REQUIRE(solver.getRules() == motusRules);

        // invalid templates
        REQUIRE_THROWS_MATCHES(solver.setTemplate("abc45"), InvalidArgException, Message("invalid template, must contain either '.' or letters."));
        REQUIRE(solver.getTemplate().empty());

        REQUIRE_THROWS_MATCHES(solver.setTemplate("....!"), InvalidArgException, Message("invalid template, must contain either '.' or letters."));
        REQUIRE(solver.getTemplate().empty());

        // accept some words
        REQUIRE_NOTHROW(solver.setTemplate("m....."));
        REQUIRE(solver.getTemplate() == "m.....");
        REQUIRE(solver.getPotentialGuesses()   == std::vector<std::string_view>{"mazout", "metiez", "minima", "mondes", "mouler"});
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"mazout", "metiez", "minima", "mondes", "mouler"});

        REQUIRE_NOTHROW(solver.setTemplate("M.....")); // invalid case should be corrected
        REQUIRE(solver.getTemplate() == "m.....");
        REQUIRE(solver.getPotentialGuesses()   == std::vector<std::string_view>{"mazout", "metiez", "minima", "mondes", "mouler"});
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"mazout", "metiez", "minima", "mondes", "mouler"});

        REQUIRE_NOTHROW(solver.setTemplate("mo...."));
        REQUIRE(solver.getTemplate() == "mo....");
        REQUIRE(solver.getPotentialGuesses()   == std::vector<std::string_view>{"mazout", "metiez", "minima", "mondes", "mouler"});
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"mondes", "mouler"});

        REQUIRE_NOTHROW(solver.setTemplate("n......"));
        REQUIRE(solver.getTemplate() == "n......");
        REQUIRE(solver.getPotentialGuesses()   == std::vector<std::string_view>{"notarie"});
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"notarie"});
    }

    SECTION("Adding hints (wordle)") {
        std::vector<std::string_view> allWordsViews;
        std::transform(std::cbegin(wordleDict->getAllWords()), std::cend(wordleDict->getAllWords()),
                std::back_inserter(allWordsViews), [](const auto& str) {
            return std::string_view(str);
        });

        // try 1
        solver.setTemplate(".....");
        REQUIRE(solver.getTemplate() == ".....");
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == allWordsViews);

        REQUIRE_NOTHROW(solver.addHint("bruir", {WRONG, WRONG, WRONG, WRONG, WRONG}));
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{
            "agaca", "amont", "appat", "chyle", "conga", "dakat", "metas",
            "pogna", "potes", "softs", "theme", "vanne", "welte"
        });

        REQUIRE_NOTHROW(solver.addHint("theme", {MISPLACED, WRONG, WRONG, MISPLACED, WRONG}));
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"amont"});

        REQUIRE_NOTHROW(solver.addHint("amont", {CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}));
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"amont"});

        // reset
        solver.reset();
        REQUIRE(solver.getTemplate().empty());
        REQUIRE(solver.getHints().empty());
        REQUIRE(solver.getPotentialGuesses().empty());
        REQUIRE(solver.getPotentialSolutions().empty());

        // try 2
        solver.setTemplate(".....");
        REQUIRE(solver.getTemplate() == ".....");
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == allWordsViews);

        REQUIRE_NOTHROW(solver.addHint("bolia", {CORRECT, WRONG, WRONG, WRONG, MISPLACED}));
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{
            "badge", "barbu", "battu"
        });

        REQUIRE_NOTHROW(solver.addHint("barbu", {CORRECT, CORRECT, WRONG, WRONG, CORRECT}));
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"battu"});

        REQUIRE_NOTHROW(solver.addHint("battu", {CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}));
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"battu"});

        // reset
        solver.reset();
        REQUIRE(solver.getTemplate().empty());
        REQUIRE(solver.getHints().empty());
        REQUIRE(solver.getPotentialGuesses().empty());
        REQUIRE(solver.getPotentialSolutions().empty());

        // try 3 - guess word in 1st try
        solver.setTemplate(".....");
        REQUIRE(solver.getTemplate() == ".....");
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == allWordsViews);

        REQUIRE_NOTHROW(solver.addHint("cause", {CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}));
        REQUIRE(solver.getPotentialGuesses() == allWordsViews);
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"cause"});
    }

    SECTION("Adding hints (motus)") {
        const auto& motusDict = getMotusDict();
        std::shared_ptr<IGameRules> motusRules = std::make_shared<MotusGameRules>(motusDict);
        solver.setRules(motusRules);
        REQUIRE(solver.getRules() == motusRules);

        // try 1
        solver.setTemplate("m.....");
        REQUIRE(solver.getTemplate() == "m.....");
        REQUIRE(solver.getPotentialGuesses() == std::vector<std::string_view>{
            "mazout", "metiez", "minima", "mondes", "mouler"
        });
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{
            "mazout", "metiez", "minima", "mondes", "mouler"
        });

        REQUIRE_NOTHROW(solver.addHint("metiez", {CORRECT, MISPLACED, WRONG, WRONG, WRONG, WRONG}));
        REQUIRE(solver.getPotentialGuesses() == std::vector<std::string_view>{
            "mazout", "metiez", "minima", "mondes", "mouler"
        });
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"mondes", "mouler"});

        REQUIRE_NOTHROW(solver.addHint("mouler", {CORRECT, CORRECT, WRONG, WRONG, CORRECT, WRONG}));
        REQUIRE(solver.getPotentialGuesses() == std::vector<std::string_view>{
            "mazout", "metiez", "minima", "mondes", "mouler"
        });
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"mondes"});

        REQUIRE_NOTHROW(solver.addHint("mondes", {CORRECT, CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}));
        REQUIRE(solver.getPotentialGuesses() == std::vector<std::string_view>{
            "mazout", "metiez", "minima", "mondes", "mouler"
        });
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"mondes"});

        // reset
        solver.reset();
        REQUIRE(solver.getTemplate().empty());
        REQUIRE(solver.getHints().empty());
        REQUIRE(solver.getPotentialGuesses().empty());
        REQUIRE(solver.getPotentialSolutions().empty());

        // try 3 - guess word in 1st try
        solver.setTemplate("t......");
        REQUIRE(solver.getTemplate() == "t......");
        REQUIRE(solver.getPotentialGuesses() == std::vector<std::string_view>{"tardive", "tunnels"});
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"tardive", "tunnels"});

        REQUIRE_NOTHROW(solver.addHint("tardive", {CORRECT, CORRECT, CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}));
        REQUIRE(solver.getPotentialGuesses() == std::vector<std::string_view>{"tardive", "tunnels"});
        REQUIRE(solver.getPotentialSolutions() == std::vector<std::string_view>{"tardive"});
   }

    SECTION("Adding invalid hints (wordle)") {
        REQUIRE_THROWS_MATCHES(solver.addHint("barbu", {WRONG, WRONG, WRONG, WRONG, WRONG}), Exception, Message("template needs to be set before adding hints."));

        solver.setTemplate(".....");
        REQUIRE_THROWS_MATCHES(solver.addHint("abcd", {WRONG, WRONG, WRONG, WRONG}), Exception, Message("guess is not a valid guess."));
        REQUIRE_THROWS_MATCHES(solver.addHint("abcde", {WRONG, WRONG, WRONG, WRONG, WRONG}), Exception, Message("guess is not a valid guess."));

        REQUIRE_THROWS_MATCHES(solver.addHint("barbu", {WRONG, WRONG, WRONG, WRONG, WRONG, WRONG}), Exception, Message("the number of hints does not match the guess' number of letters."));
    }

    SECTION("Adding invalid hints (motus)") {
        const auto& motusDict = getMotusDict();
        std::shared_ptr<IGameRules> motusRules = std::make_shared<MotusGameRules>(motusDict);
        solver.setRules(motusRules);
        REQUIRE(solver.getRules() == motusRules);


        REQUIRE_THROWS_MATCHES(solver.addHint("mondes", {WRONG, WRONG, WRONG, WRONG, WRONG, WRONG}), Exception, Message("template needs to be set before adding hints."));

        solver.setTemplate("m.....");
        REQUIRE_THROWS_MATCHES(solver.addHint("abcd", {WRONG, WRONG, WRONG, WRONG}), Exception, Message("guess is not a valid guess."));
        REQUIRE_THROWS_MATCHES(solver.addHint("abcde", {WRONG, WRONG, WRONG, WRONG, WRONG}), Exception, Message("guess is not a valid guess."));
        REQUIRE_THROWS_MATCHES(solver.addHint("muniras", {WRONG, WRONG, WRONG, WRONG, WRONG, WRONG, WRONG}), Exception, Message("guess is not a valid guess."));
        REQUIRE_THROWS_MATCHES(solver.addHint("amenda", {WRONG, WRONG, WRONG, WRONG, WRONG, WRONG}), Exception, Message("guess is not a valid guess."));

        REQUIRE_THROWS_MATCHES(solver.addHint("mondes", {WRONG, WRONG, WRONG, WRONG, WRONG}), Exception, Message("the number of hints does not match the guess' number of letters."));
    }
}
