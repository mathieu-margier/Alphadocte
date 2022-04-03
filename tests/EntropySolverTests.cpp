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
 * File: EntropySolverTests.cpp
 */

#include <Alphadocte/EntropyMaximizer.h>
#include <Alphadocte/Exceptions.h>
#include <Alphadocte/Game.h>
#include <Alphadocte/WordleGameRules.h>
#include <cmath>
#include <filesystem>
#include <fstream>

#include <catch2/catch.hpp>

#include "TestDefinitions.h"

using Catch::Message;
using namespace Alphadocte;
using enum HintType;

static const auto WORDLE_ALL_WORDS_ENTROPY = Approx(log2(67)); // 67 solutions possibles
constexpr size_t GUESSES_CROP = 10;

//
void checkGuesses(const EntropyMaximizer& solver);

// helper functions
void generateHints(std::ostream& ostream, std::string guess, std::vector<std::string> solutions);

TEST_CASE("Creating entropy maximiser solver with invalid rules", "[solver][Lib]") {
    REQUIRE_THROWS_MATCHES(EntropyMaximizer({}), InvalidArgException, Message("rules cannot be null"));
}

TEST_CASE("Testing EntropyMaximizer solver", "[solver][Lib]") {
    const auto& wordleDict = getWordleDict();
    std::shared_ptr<IGameRules> rules = std::make_shared<WordleGameRules>(wordleDict);
    EntropyMaximizer solver{rules};
    REQUIRE(solver.getSolverName() == "entropy_maximizer");
    REQUIRE(solver.getSolverVersion() == 1);

    SECTION("Entropy computation") {
        // check the expected entropy by using hints automatically computed by Game::computeHints,
        // and count manually the occurrences of each of those hints (ie how many solutions matches a given list of hints)
        // See the hidden test case "Generate hints for wordle (debug only)" to generate the hints,
        // and count the occurrence quickly.

        double currentEntropy{}, actualGuessEntropy{};
        std::string guess;
        std::vector<HintType> hints;
        double n;

        solver.setTemplate(".....");
        n = std::size(solver.getPotentialSolutions());
        CHECK(n > 0);
        currentEntropy = solver.computeCurrentEntropy();
        REQUIRE(currentEntropy == WORDLE_ALL_WORDS_ENTROPY);

        // compute several guess entropy
        guess = "agaca";
        hints = {CORRECT, WRONG, WRONG, WRONG, WRONG};
        actualGuessEntropy = solver.computeActualEntropy(guess, hints);
        REQUIRE(solver.computeExpectedEntropy(guess) == Approx(
              -  1/n * log2( 1/n) * 7 //   singletons:  ooxxx, oxxvx, vvvvv, vxoxx, xoxov, xoxxv, xxxov
              -  2/n * log2( 2/n) * 2 //  2 solutions: oxoxx, oxxox
              -  3/n * log2( 3/n) * 2 //  3 solutions: vxxxx, xoxxx
              -  4/n * log2( 4/n) * 1 //  4 solutions: xxxox
              -  6/n * log2( 6/n) * 1 //  6 solutions: xxxxv
              - 13/n * log2(13/n) * 1 // 13 solutions: oxxxx
              - 27/n * log2(27/n) * 1 // 27 solutions: xxxxx
        ));
        REQUIRE(actualGuessEntropy == Approx(currentEntropy - log2(3))); // only 3 possible solutions : aient, amont, arroi

        guess = "boita";
        hints = {WRONG, MISPLACED, WRONG, MISPLACED, MISPLACED};
        actualGuessEntropy = solver.computeActualEntropy(guess, hints);
        REQUIRE(solver.computeExpectedEntropy(guess) == Approx(
              -  1/n * log2( 1/n) * 23 //  singletons: ovxxx, oxxxo, oxxxv, vvoxv, vvvvv, vvxxx, vxoxv, vxxvo,
                                       //              xooox, xooxo, xovxx, xoxoo, xoxox, xvxox, xvxvx, xvxxo,
                                       //              xvxxx, xxoox, xxovv, xxovx, xxxvv, xxxvx, xxxxv
              -  2/n * log2( 2/n) * 9 //  2 solutions: voxxo, vxoxx, vxxxo, xooxx, xoxxo, xoxxx, xvoxx, xxooo, xxxox
              -  3/n * log2( 3/n) * 3 //  3 solutions: xvxxv, xxoxo, xxxoo
              -  4/n * log2( 4/n) * 1 //  4 solutions: xxxxo
              -  6/n * log2( 6/n) * 1 //  6 solutions: xxxxx
              -  7/n * log2( 7/n) * 1 //  7 solutions: xxoxx
        ));
        REQUIRE(actualGuessEntropy == Approx(currentEntropy - log2(1))); // only 1 possible solutions : amont

        // add hints until unique solution found
        guess = "bruir";
        hints = {WRONG, WRONG, WRONG, WRONG, WRONG};
        actualGuessEntropy = solver.computeActualEntropy(guess, hints);
        REQUIRE(solver.computeExpectedEntropy(guess) == Approx(
              -  1/n * log2( 1/n) * 20 //  singletons: oxvxx, vooxx, voxxx, vvvvv, vxxvx, vxxxv, vxxxx, xooxx, xovxv,
                                       //              xoxox, xoxxv, xvxoo, xvxox, xvxvx, xvxxx, xxoox, xxvvx, xxvxx,
                                       //              xxxov, xxxxv
              -  2/n * log2( 2/n) * 3 //  2 solutions: oxxxx, vxoxx, xovxx
              -  3/n * log2( 3/n) * 2 //  3 solutions: vxxox, xxxvx
              -  4/n * log2( 4/n) * 1 //  4 solutions: xoxxx
              -  6/n * log2( 6/n) * 1 //  6 solutions: xxoxx
              - 12/n * log2(12/n) * 1 // 12 solutions: xxxox
              - 13/n * log2(13/n) * 1 // 13 solutions: xxxxx
        ));
        REQUIRE(actualGuessEntropy == Approx(currentEntropy - log2(13))); // 13 possible solutions, see test on base solver class
        solver.addHint(guess, hints);

        REQUIRE(solver.computeCurrentEntropy() == Approx(currentEntropy - actualGuessEntropy));
        currentEntropy = solver.computeCurrentEntropy();
        n = std::size(solver.getPotentialSolutions());
        REQUIRE(n > 0);

        guess = "theme";
        hints = {MISPLACED, WRONG, WRONG, MISPLACED, WRONG};
        actualGuessEntropy = solver.computeActualEntropy(guess, hints);
        REQUIRE(solver.computeExpectedEntropy(guess) == Approx(
              -  1/n * log2( 1/n) * 7 //   singletons: oxoox, oxoxv, oxoxx, oxxox, vvvvv, xvxxv, xxxxv
              -  3/n * log2( 3/n) * 2 //  3 solutions: oxxxx, xxxxx

        ));
        REQUIRE(actualGuessEntropy == Approx(currentEntropy - log2(1))); // only one remaining solution, the actual solution
        solver.addHint(guess, hints);

        REQUIRE(solver.computeCurrentEntropy() == Approx(0.));
        REQUIRE(solver.computeNextGuess() == "amont");
    }

    SECTION("Guesses computation") {
        solver.setTemplate(".....");

        // assume entropy computation is correct, check guesses computation
        checkGuesses(solver);

        // try 1
        solver.addHint("bruir", {WRONG, WRONG, WRONG, WRONG, WRONG});
        REQUIRE_NOTHROW(solver.addHint("theme", {MISPLACED, WRONG, WRONG, MISPLACED, WRONG}));
        checkGuesses(solver);
        REQUIRE_NOTHROW(solver.addHint("amont", {CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}));
        checkGuesses(solver);

        // reset
        solver.reset();
        REQUIRE(solver.getTemplate().empty());
        REQUIRE(solver.getHints().empty());
        REQUIRE(solver.getPotentialGuesses().empty());
        REQUIRE(solver.getPotentialSolutions().empty());

        // try 2
        solver.setTemplate(".....");
        REQUIRE(solver.getTemplate() == ".....");
        checkGuesses(solver);
        REQUIRE_NOTHROW(solver.addHint("bolia", {CORRECT, WRONG, WRONG, WRONG, MISPLACED}));
        checkGuesses(solver);
        REQUIRE_NOTHROW(solver.addHint("barbu", {CORRECT, CORRECT, WRONG, WRONG, CORRECT}));
        checkGuesses(solver);
        REQUIRE_NOTHROW(solver.addHint("battu", {CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}));
        checkGuesses(solver);

        // reset
        solver.reset();
        REQUIRE(solver.getTemplate().empty());
        REQUIRE(solver.getHints().empty());
        REQUIRE(solver.getPotentialGuesses().empty());
        REQUIRE(solver.getPotentialSolutions().empty());

        // try 3 - guess word in 1st try
        solver.setTemplate(".....");
        REQUIRE(solver.getTemplate() == ".....");
        checkGuesses(solver);
        REQUIRE_NOTHROW(solver.addHint("cause", {CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}));
        checkGuesses(solver);
    }

    SECTION("Solve games without solution") {
        REQUIRE_THROWS_MATCHES(solver.computeNextGuess(), Exception, Message("cannot compute next guess with an empty template."));
        REQUIRE_THROWS_MATCHES(solver.computeNextGuesses(GUESSES_CROP), Exception, Message("cannot compute next guess with an empty template."));

        // impossible case
        solver.setTemplate(".....");
        solver.addHint("barbu", {WRONG, WRONG, WRONG, WRONG, WRONG});
        REQUIRE(solver.computeActualEntropy("barbu", {CORRECT, WRONG, WRONG, WRONG, WRONG}) < 0);
        solver.addHint("barbu", {CORRECT, WRONG, WRONG, WRONG, WRONG});
        REQUIRE(solver.getPotentialSolutions().empty());
        REQUIRE(solver.computeNextGuess().empty());
        REQUIRE(solver.computeNextGuesses(GUESSES_CROP).empty());
        REQUIRE(solver.computeExpectedEntropy("barbu") == Approx(0.));
        REQUIRE(solver.computeExpectedEntropy("amont") == Approx(0.));
        REQUIRE(solver.computeCurrentEntropy() < 0);

        // reset
        solver.reset();
        REQUIRE(solver.getTemplate().empty());
        REQUIRE(solver.getHints().empty());
        REQUIRE(solver.getPotentialGuesses().empty());
        REQUIRE(solver.getPotentialSolutions().empty());

        // actual solution not in dictionary (here: "aller"
        solver.setTemplate(".....");
        solver.addHint("agaca", {CORRECT, WRONG, WRONG, WRONG, WRONG});
        // only 3 potential solutions left in dictionary: aient, amont, arroi
        solver.addHint("aient", {CORRECT, WRONG, MISPLACED, WRONG, WRONG});
        // no more potential solutions in dictionary
        REQUIRE(solver.getPotentialSolutions().empty());
        REQUIRE(solver.computeNextGuess().empty());
        REQUIRE(solver.computeNextGuesses(GUESSES_CROP).empty());
        REQUIRE(solver.computeExpectedEntropy("barbu") == Approx(0.));
        REQUIRE(solver.computeExpectedEntropy("amont") == Approx(0.));
        REQUIRE(solver.computeCurrentEntropy() < 0);
    }
}

void checkGuesses(const EntropyMaximizer& solver) {
    const auto& guesses = solver.getPotentialGuesses();
    const auto& solutions = solver.getPotentialSolutions();
    std::map<std::string_view, double> entropies;
    std::for_each(std::cbegin(guesses), std::cend(guesses), [&solver, &entropies](auto guess){
        entropies.emplace(guess, solver.computeExpectedEntropy(guess));
    });

    auto allGuesses = solver.computeNextGuesses(std::size(guesses));
    auto topGuesses = solver.computeNextGuesses(GUESSES_CROP);
    auto bestGuess = solver.computeNextGuess();

    // Check that entropy computation matches
    for (const auto& guessPair : allGuesses) {
        REQUIRE(entropies.at(guessPair.first) == Approx(guessPair.second));
    }

    // Check guesses order
    REQUIRE(std::is_sorted(std::cbegin(allGuesses), std::cend(allGuesses), [&solutions](auto guess1, auto guess2){
        // return true if guess1 is (strictly) better than guess2
        if (guess1.second > guess2.second) {
            // guess1 has more information
            return true;
        } else if (guess1.second == guess2.second) {
            // guess1 and guess2 has the same quantity of information
            // favor potential solution against non-solution:
            // guess1 is better than guess2 only if it is a potential solution, while guess2 is not
            return std::binary_search(std::cbegin(solutions), std::cend(solutions), guess1.first)
               && !std::binary_search(std::cbegin(solutions), std::cend(solutions), guess2.first);
        } else {
            // guess2 has less information
            return false;
        }
    }));

    // check guess
    REQUIRE(bestGuess == allGuesses.front().first);

    if (std::size(guesses) < GUESSES_CROP) {
        // entire array must be identical
        REQUIRE(topGuesses == allGuesses);
    } else {
        // allGuesses' sub array must be identical to topGuesses
        REQUIRE(std::equal(std::cbegin(topGuesses), std::cend(topGuesses), std::cbegin(allGuesses)));
    }
}


// Helper test case
// Generate hints for entropy manual computation
// the generated files show all the hints obtained from one guess
// with all the potential solutions
// We can then keep the hints, sort them and count the occurrences to
// manually compute the mean expected entropy, ie in bash :
// cat <file.txt> | awk -F: '{print $2;}' | sort | uniq -c
TEST_CASE("Generate hints for wordle (debug only)", "[.]") {
    const auto& wordleDict = getWordleDict();
    std::vector<std::tuple<std::string, std::string, std::vector<std::string>>> inputs = {
            {"wordle_bruir_all.txt", "bruir", wordleDict->getAllWords()},
            {"wordle_agaca_all.txt", "agaca", wordleDict->getAllWords()},
            {"wordle_boita_all.txt", "boita", wordleDict->getAllWords()},
            {"wordle_theme_bruir_step2.txt", "theme", {"agaca", "amont", "appat", "chyle", "conga", "dakat", "metas", "pogna", "potes", "softs", "theme", "vanne", "welte"}}

    };

    try {
        std::filesystem::path path{"gen_data"};
        std::filesystem::create_directories(path);
        std::ofstream os;

        for (const auto& tuple : inputs) {
            const auto& [filename, guess, solutions] = tuple;
            os = std::ofstream(path / filename);
            generateHints(os, guess, solutions);
            os.close();
            if (os.fail()) {
                throw std::runtime_error(std::string("could not close output file stream correctly."));
            }
        }


    } catch (const std::exception& e) {
        FAIL(e.what());
    }


}

void generateHints(std::ostream& ostream, std::string guess, std::vector<std::string> solutions) {
    if (!ostream.good()) {
        throw std::runtime_error(std::string("could not open output file stream."));
    }

    for (const auto& solution : solutions) {
        const auto hints = Game::computeHints(guess, solution);

        ostream << solution << ": ";
        for (HintType hint : hints) {
            switch (hint) {
            case CORRECT:
                ostream << 'v';
                break;
            case MISPLACED:
                ostream << 'o';
                break;
            case WRONG:
                ostream << 'x';
                break;
            default:
                throw std::runtime_error(std::string("Unknown hint encountered."));
            }
        }

        ostream << '\n';

        if (!ostream.good()) {
            throw std::runtime_error(std::string("error while writing to output stream."));
        }
    }
}
