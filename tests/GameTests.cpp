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
 * File: GameTests.cpp
 */

#include <Alphadocte/Exceptions.h>
#include <Alphadocte/Game.h>
#include <Alphadocte/MotusGameRules.h>
#include <Alphadocte/WordleGameRules.h>
#include <catch2/catch.hpp>

#include "TestDefinitions.h"

using namespace Alphadocte;
using enum HintType;
using Catch::Matchers::Message;

constexpr int UNLIMITED_GUESS_NB = 100;

TEST_CASE("Check game with invalid rules", "[game][Lib]") {
    REQUIRE_THROWS_MATCHES(Game({}), InvalidArgException, Message("rules cannot be null"));
}

TEST_CASE("Check game with motus rules", "[game][Lib]") {
    auto motusDict = getMotusDict();
    const auto& words = motusDict->getAllWords();
    std::shared_ptr<IGameRules> rules = std::make_shared<MotusGameRules>(motusDict);
    REQUIRE(rules->getMaxGuesses() == 6);

    Game game{rules};
    REQUIRE_FALSE(game.hasStarted());
    REQUIRE_FALSE(game.isOver());
    REQUIRE_FALSE(game.isWon());
    REQUIRE(game.getNbGuess() == 0);
    REQUIRE(game.getRules() == rules);

    SECTION("Set secret word") {
        REQUIRE(game.getWord().empty());

        REQUIRE_FALSE(rules->isSolutionValid("a"));
        REQUIRE_THROWS_MATCHES(game.setWord("a"), InvalidArgException, Message("the word a is not a valid solution"));

        REQUIRE(rules->isSolutionValid(words.front()));
        game.setWord(words.front());
        REQUIRE(game.getWord() == words.front());

        REQUIRE(rules->isSolutionValid(words.back()));
        game.setWord(words.back());
        REQUIRE(game.getWord() == words.back());
    }

    SECTION("Starting game in invalid state") {
        REQUIRE_THROWS_MATCHES(game.start(), Exception, Message("Cannot start game: no word has been set"));

        game.setWord(motusDict->getAllWords().front());
        game.start();
        REQUIRE_THROWS_MATCHES(game.start(), Exception, Message("Cannot start game: game has already been started"));
    }

    SECTION("Setting an invalid word") {
        REQUIRE_THROWS_MATCHES(game.setWord(""), InvalidArgException, Message("the word  is not a valid solution"));
        REQUIRE(game.getWord().empty());
        REQUIRE_THROWS_MATCHES(game.setWord("55555"), InvalidArgException, Message("the word 55555 is not a valid solution"));
        REQUIRE(game.getWord().empty());
        REQUIRE_THROWS_MATCHES(game.setWord("dico"), InvalidArgException, Message("the word dico is not a valid solution"));
        REQUIRE(game.getWord().empty());
        REQUIRE_THROWS_MATCHES(game.setWord("COMPARA"), InvalidArgException, Message("the word COMPARA is not a valid solution"));
        REQUIRE(game.getWord().empty());

        REQUIRE_THROWS_MATCHES(rules->getTemplate(game), Exception, Message("no word has been set."));
        REQUIRE(game.getWord().empty());
    }

    SECTION("Setting a word while playing") {
        std::string word1 = motusDict->getAllWords().front();
        std::string word2 = motusDict->getAllWords().back();
        REQUIRE(word1 != word2);

        game.setWord(word1);
        REQUIRE(game.getWord() == word1);
        game.start();
        REQUIRE(game.hasStarted());

        // changes nothings
        REQUIRE_NOTHROW(game.setWord(word1));
        REQUIRE(game.getWord() == word1);

        // try to change the word
        REQUIRE_THROWS_MATCHES(game.setWord(word2), Exception, Message("Cannot change word while the game is playing."));
        REQUIRE(game.getWord() == word1);

        // change words while not playing
        game.reset();
        game.setWord(word1);
        REQUIRE_NOTHROW(game.setWord(word2));
        REQUIRE(game.getWord() == word2);
    }

    SECTION("Playing and winning a game in maximum number of turns") {
        game.setWord("compara");
        game.start();
        REQUIRE(game.hasStarted());
        REQUIRE(game.getNbGuess() == 0);
        REQUIRE(game.getGuessesHints().empty());
        REQUIRE(game.getTriedGuesses().empty());

        std::vector<std::string> guesses;
        std::vector<std::vector<HintType>> hints;
        std::vector<std::tuple<std::string, std::vector<HintType>, std::string>> tries = {
                {"cedrela", {CORRECT, WRONG, WRONG, MISPLACED, WRONG, WRONG, CORRECT}, "c......"},
                {"chelems", {CORRECT, WRONG, WRONG, WRONG, WRONG, MISPLACED, WRONG}, "c.....a"},
                {"croupes", {CORRECT, MISPLACED, MISPLACED, WRONG, MISPLACED, WRONG, WRONG}, "c.....a"},
                {"croupal", {CORRECT, MISPLACED, MISPLACED, WRONG, MISPLACED, MISPLACED, WRONG}, "c.....a"},
                {"couvoit", {CORRECT, CORRECT, WRONG, WRONG, WRONG, WRONG, WRONG}, "c.....a"},
                {"compara", {CORRECT, CORRECT, CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}, "co....a"}
        };

        unsigned int i = 0;
        for (auto [guess, hint, templateStr] : tries) {
            // check template according to the rules
            REQUIRE(rules->getTemplate(game) == templateStr);

            // check actual guess
            REQUIRE(game.tryGuess(guess) == hint);
            guesses.emplace_back(std::move(guess));
            hints.emplace_back(std::move(hint));
            REQUIRE(game.getGuessesHints() == hints);
            REQUIRE(game.getTriedGuesses() == guesses);
            REQUIRE(game.getNbGuess() == ++i);

            if (i < std::size(tries)) {
                // should not be over
                REQUIRE_FALSE(game.isOver());
                REQUIRE_FALSE(game.isWon());
            }
        }

        // should be won
        REQUIRE(game.isOver());
        REQUIRE(game.isWon());
        REQUIRE(rules->getTemplate(game) == game.getWord());

        // no more guesses are possible
        REQUIRE_THROWS_MATCHES(game.tryGuess(guesses.front()), Exception, Message("Cannot try a guess: game is over"));
    }

    SECTION("Playing and loosing a game") {
        game.setWord("compara");
        game.start();
        REQUIRE(game.hasStarted());
        REQUIRE(game.getNbGuess() == 0);
        REQUIRE(game.getGuessesHints().empty());
        REQUIRE(game.getTriedGuesses().empty());

        std::vector<std::string> guesses;
        std::vector<std::vector<HintType>> hints;
        std::vector<std::tuple<std::string, std::vector<HintType>, std::string>> tries = {
                {"cedrela", {CORRECT, WRONG, WRONG, MISPLACED, WRONG, WRONG, CORRECT}, "c......"},
                {"chelems", {CORRECT, WRONG, WRONG, WRONG, WRONG, MISPLACED, WRONG}, "c.....a"},
                {"croupes", {CORRECT, MISPLACED, MISPLACED, WRONG, MISPLACED, WRONG, WRONG}, "c.....a"},
                {"croupal", {CORRECT, MISPLACED, MISPLACED, WRONG, MISPLACED, MISPLACED, WRONG}, "c.....a"},
                {"couvoit", {CORRECT, CORRECT, WRONG, WRONG, WRONG, WRONG, WRONG}, "c.....a"},
                {"cedrela", {CORRECT, WRONG, WRONG, MISPLACED, WRONG, WRONG, CORRECT}, "co....a"}
        };

        unsigned int i = 0;
        for (auto [guess, hint, templateStr] : tries) {
            // check template according to the rules
            REQUIRE(rules->getTemplate(game) == templateStr);

            // check actual guess
            REQUIRE(game.tryGuess(guess) == hint);
            guesses.emplace_back(std::move(guess));
            hints.emplace_back(std::move(hint));
            REQUIRE(game.getGuessesHints() == hints);
            REQUIRE(game.getTriedGuesses() == guesses);
            REQUIRE(game.getNbGuess() == ++i);

            if (i < std::size(tries)) {
                // should not be over
                REQUIRE_FALSE(game.isOver());
                REQUIRE_FALSE(game.isWon());
            }
        }

        // should be over and lost
        REQUIRE(game.isOver());
        REQUIRE_FALSE(game.isWon());
        REQUIRE(rules->getTemplate(game) == "co....a");

        // no more guesses are possible
        REQUIRE_THROWS_MATCHES(game.tryGuess(guesses.front()), Exception, Message("Cannot try a guess: game is over"));
    }

    SECTION("Playing and winning a game in one turn") {
        game.setWord("compara");
        game.start();
        REQUIRE(game.hasStarted());
        REQUIRE(game.getNbGuess() == 0);
        REQUIRE(game.getGuessesHints().empty());
        REQUIRE(game.getTriedGuesses().empty());


        REQUIRE(rules->getTemplate(game) == "c......");
        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());

        // check actual guess
        REQUIRE(game.tryGuess("compara") == std::vector{CORRECT, CORRECT, CORRECT, CORRECT, CORRECT, CORRECT, CORRECT});
        REQUIRE(game.getGuessesHints() == std::vector<std::vector<HintType>>{std::vector{CORRECT, CORRECT, CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}});
        REQUIRE(game.getTriedGuesses() == std::vector<std::string>{"compara"});
        REQUIRE(game.getNbGuess() == 1);

        // should be won
        REQUIRE(game.isOver());
        REQUIRE(game.isWon());
        REQUIRE(rules->getTemplate(game) == game.getWord());

        // no more guesses are possible
        REQUIRE_THROWS_MATCHES(game.tryGuess("compara"), Exception, Message("Cannot try a guess: game is over"));
    }

    SECTION("Trying invalid guesses") {
        REQUIRE_THROWS_MATCHES(game.tryGuess(motusDict->getAllWords().front()), Exception, Message("Cannot try a guess: game has not been started"));
        REQUIRE_THROWS_MATCHES(game.tryGuess("a"), Exception, Message("Cannot try a guess: game has not been started"));

        game.setWord("compara");
        REQUIRE_THROWS_MATCHES(game.tryGuess("compara"), Exception, Message("Cannot try a guess: game has not been started"));
        REQUIRE_THROWS_MATCHES(game.tryGuess("chelems"), Exception, Message("Cannot try a guess: game has not been started"));
        REQUIRE_THROWS_MATCHES(game.tryGuess("cubee"), Exception, Message("Cannot try a guess: game has not been started"));
        REQUIRE_THROWS_MATCHES(game.tryGuess("dorment"), Exception, Message("Cannot try a guess: game has not been started"));

        game.start();
        REQUIRE_THROWS_MATCHES(game.tryGuess("cubee"), Exception, Message("Cannot try a guess: invalid guess"));
        REQUIRE_THROWS_MATCHES(game.tryGuess("dorment"), Exception, Message("Cannot try a guess: invalid guess"));
        REQUIRE_NOTHROW(game.tryGuess("chelems"));
        REQUIRE_NOTHROW(game.tryGuess("compara"));

        // game is now over
        REQUIRE_THROWS_MATCHES(game.tryGuess("chelems"), Exception, Message("Cannot try a guess: game is over"));
    }

    SECTION("Resetting game") {
        // reset an empty game
        game.reset();
        REQUIRE_FALSE(game.hasStarted());
        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());
        REQUIRE(game.getWord().empty());
        REQUIRE(game.getTriedGuesses().empty());
        REQUIRE(game.getGuessesHints().empty());
        REQUIRE(game.getNbGuess() == 0);
        REQUIRE(game.getRules() == rules);

        game.setWord("compara");
        game.start();
        REQUIRE(game.hasStarted());
        game.tryGuess("chelems");
        game.tryGuess("compara");
        game.reset();
        REQUIRE_FALSE(game.hasStarted());
        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());
        REQUIRE(game.getWord().empty());
        REQUIRE(game.getTriedGuesses().empty());
        REQUIRE(game.getGuessesHints().empty());
        REQUIRE(game.getNbGuess() == 0);
        REQUIRE(game.getRules() == rules);

    }

    SECTION("Trying a game with unlimited number of guesses") {
        std::string solution = "absurdo";
        std::string guess = "allegie";
        rules->setMaxGuesses(0);
        game.setWord(solution);
        game.start();

        for (unsigned int i = 0; i < UNLIMITED_GUESS_NB; i++) {
            REQUIRE_NOTHROW(game.tryGuess(guess));
        }

        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());
    }

    SECTION("Changing game rules") {
        game.setWord("absurdo");
        game.start();
        game.tryGuess("allegie");
        REQUIRE(game.hasStarted());
        REQUIRE(game.getNbGuess() == 1);
        REQUIRE(game.getWord() == "absurdo");
        const auto& guesses = game.getTriedGuesses();
        const auto& hints = game.getGuessesHints();

        // change nothing
        game.setRules(nullptr);
        REQUIRE(game.hasStarted());
        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());
        REQUIRE(game.getTriedGuesses() == guesses);
        REQUIRE(game.getGuessesHints() == hints);
        REQUIRE(game.getWord() == "absurdo");

        game.setRules(rules);
        REQUIRE(game.hasStarted());
        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());
        REQUIRE(game.getTriedGuesses() == guesses);
        REQUIRE(game.getGuessesHints() == hints);
        REQUIRE(game.getWord() == "absurdo");

        // change rules (and reset game)
        auto otherRules = std::make_shared<WordleGameRules>(getWordleDict());
        game.setRules(otherRules);
        REQUIRE_FALSE(game.hasStarted());
        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());
        REQUIRE(game.getTriedGuesses().empty());
        REQUIRE(game.getGuessesHints().empty());
        REQUIRE(game.getWord().empty());
        REQUIRE(game.getRules() == otherRules);
    }
}

TEST_CASE("Check game with wordle rules", "[game][Lib]") {
    auto wordleDict = getWordleDict();
    const auto& words = wordleDict->getAllWords();
    std::shared_ptr<IGameRules> rules = std::make_shared<WordleGameRules>(wordleDict);
    REQUIRE(rules->getMaxGuesses() == 6);

    Game game{rules};
    REQUIRE_FALSE(game.hasStarted());
    REQUIRE_FALSE(game.isOver());
    REQUIRE_FALSE(game.isWon());
    REQUIRE(game.getNbGuess() == 0);
    REQUIRE(game.getRules() == rules);

    SECTION("Set secret word") {
        REQUIRE(game.getWord().empty());

        REQUIRE_FALSE(rules->isSolutionValid("a"));
        REQUIRE_THROWS_MATCHES(game.setWord("a"), InvalidArgException, Message("the word a is not a valid solution"));

        REQUIRE(rules->isSolutionValid(words.front()));
        game.setWord(words.front());
        REQUIRE(game.getWord() == words.front());

        REQUIRE(rules->isSolutionValid(words.back()));
        game.setWord(words.back());
        REQUIRE(game.getWord() == words.back());
    }

    SECTION("Starting game in invalid state") {
        REQUIRE_THROWS_MATCHES(game.start(), Exception, Message("Cannot start game: no word has been set"));

        game.setWord(wordleDict->getAllWords().front());
        game.start();
        REQUIRE_THROWS_MATCHES(game.start(), Exception, Message("Cannot start game: game has already been started"));
    }

    SECTION("Setting an invalid word") {
        REQUIRE_THROWS_MATCHES(game.setWord(""), InvalidArgException, Message("the word  is not a valid solution"));
        REQUIRE(game.getWord().empty());
        REQUIRE_THROWS_MATCHES(game.setWord("55555"), InvalidArgException, Message("the word 55555 is not a valid solution"));
        REQUIRE(game.getWord().empty());
        REQUIRE_THROWS_MATCHES(game.setWord("dico"), InvalidArgException, Message("the word dico is not a valid solution"));
        REQUIRE(game.getWord().empty());
        REQUIRE_THROWS_MATCHES(game.setWord("AMONT"), InvalidArgException, Message("the word AMONT is not a valid solution"));
        REQUIRE(game.getWord().empty());

        REQUIRE_THROWS_MATCHES(rules->getTemplate(game), Exception, Message("no word has been set."));
        REQUIRE(game.getWord().empty());
    }

    SECTION("Setting a word while playing") {
        std::string word1 = wordleDict->getAllWords().front();
        std::string word2 = wordleDict->getAllWords().back();
        REQUIRE(word1 != word2);

        game.setWord(word1);
        REQUIRE(game.getWord() == word1);
        game.start();
        REQUIRE(game.hasStarted());

        // changes nothings
        REQUIRE_NOTHROW(game.setWord(word1));
        REQUIRE(game.getWord() == word1);

        // try to change the word
        REQUIRE_THROWS_MATCHES(game.setWord(word2), Exception, Message("Cannot change word while the game is playing."));
        REQUIRE(game.getWord() == word1);

        // change words while not playing
        game.reset();
        game.setWord(word1);
        REQUIRE_NOTHROW(game.setWord(word2));
        REQUIRE(game.getWord() == word2);
    }

    SECTION("Playing and winning a game in maximum number of turns") {
        game.setWord("amont");
        game.start();
        REQUIRE(game.hasStarted());
        REQUIRE(game.getNbGuess() == 0);
        REQUIRE(game.getGuessesHints().empty());
        REQUIRE(game.getTriedGuesses().empty());

        std::vector<std::string> guesses;
        std::vector<std::vector<HintType>> hints;
        std::vector<std::tuple<std::string, std::vector<HintType>, std::string>> tries = {
                {"agaca", {CORRECT, WRONG, WRONG, WRONG, WRONG}, "....."},
                {"embas", {WRONG, CORRECT, WRONG, MISPLACED, WRONG}, "a...."},
                {"dakat", {WRONG, MISPLACED, WRONG, WRONG, CORRECT}, "am..."},
                {"aient", {CORRECT, WRONG, WRONG, CORRECT, CORRECT}, "am..t"},
                {"aient", {CORRECT, WRONG, WRONG, CORRECT, CORRECT}, "am.nt"},
                {"amont", {CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}, "am.nt"}
        };

        unsigned int i = 0;
        for (auto [guess, hint, templateStr] : tries) {
            // check template according to the rules
            REQUIRE(rules->getTemplate(game) == templateStr);

            // check actual guess
            REQUIRE(game.tryGuess(guess) == hint);
            guesses.emplace_back(std::move(guess));
            hints.emplace_back(std::move(hint));
            REQUIRE(game.getGuessesHints() == hints);
            REQUIRE(game.getTriedGuesses() == guesses);
            REQUIRE(game.getNbGuess() == ++i);

            if (i < std::size(tries)) {
                // should not be over
                REQUIRE_FALSE(game.isOver());
                REQUIRE_FALSE(game.isWon());
            }
        }

        // should be won
        REQUIRE(game.isOver());
        REQUIRE(game.isWon());
        REQUIRE(rules->getTemplate(game) == game.getWord());

        // no more guesses are possible
        REQUIRE_THROWS_MATCHES(game.tryGuess(guesses.front()), Exception, Message("Cannot try a guess: game is over"));
    }

    SECTION("Playing and loosing a game") {
        game.setWord("amont");
        game.start();
        REQUIRE(game.hasStarted());
        REQUIRE(game.getNbGuess() == 0);
        REQUIRE(game.getGuessesHints().empty());
        REQUIRE(game.getTriedGuesses().empty());

        std::vector<std::string> guesses;
        std::vector<std::vector<HintType>> hints;
        std::vector<std::tuple<std::string, std::vector<HintType>, std::string>> tries = {
                {"agaca", {CORRECT, WRONG, WRONG, WRONG, WRONG}, "....."},
                {"embas", {WRONG, CORRECT, WRONG, MISPLACED, WRONG}, "a...."},
                {"dakat", {WRONG, MISPLACED, WRONG, WRONG, CORRECT}, "am..."},
                {"aient", {CORRECT, WRONG, WRONG, CORRECT, CORRECT}, "am..t"},
                {"aient", {CORRECT, WRONG, WRONG, CORRECT, CORRECT}, "am.nt"},
                {"barbu", {WRONG, MISPLACED, WRONG, WRONG, WRONG}, "am.nt"}
        };

        unsigned int i = 0;
        for (auto [guess, hint, templateStr] : tries) {
            // check template according to the rules
            REQUIRE(rules->getTemplate(game) == templateStr);

            // check actual guess
            REQUIRE(game.tryGuess(guess) == hint);
            guesses.emplace_back(std::move(guess));
            hints.emplace_back(std::move(hint));
            REQUIRE(game.getGuessesHints() == hints);
            REQUIRE(game.getTriedGuesses() == guesses);
            REQUIRE(game.getNbGuess() == ++i);

            if (i < std::size(tries)) {
                // should not be over
                REQUIRE_FALSE(game.isOver());
                REQUIRE_FALSE(game.isWon());
            }
        }

        // should be over and lost
        REQUIRE(game.isOver());
        REQUIRE_FALSE(game.isWon());
        REQUIRE(rules->getTemplate(game) == "am.nt");

        // no more guesses are possible
        REQUIRE_THROWS_MATCHES(game.tryGuess(guesses.front()), Exception, Message("Cannot try a guess: game is over"));
    }

    SECTION("Playing and winning a game in one turn") {
        game.setWord("amont");
        game.start();
        REQUIRE(game.hasStarted());
        REQUIRE(game.getNbGuess() == 0);
        REQUIRE(game.getGuessesHints().empty());
        REQUIRE(game.getTriedGuesses().empty());


        REQUIRE(rules->getTemplate(game) == ".....");
        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());

        // check actual guess
        REQUIRE(game.tryGuess("amont") == std::vector{CORRECT, CORRECT, CORRECT, CORRECT, CORRECT});
        REQUIRE(game.getGuessesHints() == std::vector<std::vector<HintType>>{std::vector{CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}});
        REQUIRE(game.getTriedGuesses() == std::vector<std::string>{"amont"});
        REQUIRE(game.getNbGuess() == 1);

        // should be won
        REQUIRE(game.isOver());
        REQUIRE(game.isWon());
        REQUIRE(rules->getTemplate(game) == game.getWord());

        // no more guesses are possible
        REQUIRE_THROWS_MATCHES(game.tryGuess("amont"), Exception, Message("Cannot try a guess: game is over"));
    }

    SECTION("Trying invalid guesses") {
        REQUIRE_THROWS_MATCHES(game.tryGuess(wordleDict->getAllWords().front()), Exception, Message("Cannot try a guess: game has not been started"));
        REQUIRE_THROWS_MATCHES(game.tryGuess("a"), Exception, Message("Cannot try a guess: game has not been started"));

        game.setWord("amont");
        REQUIRE_THROWS_MATCHES(game.tryGuess("chelems"), Exception, Message("Cannot try a guess: game has not been started"));
        REQUIRE_THROWS_MATCHES(game.tryGuess("cubee"), Exception, Message("Cannot try a guess: game has not been started"));
        REQUIRE_THROWS_MATCHES(game.tryGuess("cause"), Exception, Message("Cannot try a guess: game has not been started"));
        REQUIRE_THROWS_MATCHES(game.tryGuess("amont"), Exception, Message("Cannot try a guess: game has not been started"));

        game.start();
        REQUIRE_THROWS_MATCHES(game.tryGuess("cubee"), Exception, Message("Cannot try a guess: invalid guess"));
        REQUIRE_THROWS_MATCHES(game.tryGuess("chelems"), Exception, Message("Cannot try a guess: invalid guess"));
        REQUIRE_NOTHROW(game.tryGuess("cause"));
        REQUIRE_NOTHROW(game.tryGuess("amont"));

        // game is now over
        REQUIRE_THROWS_MATCHES(game.tryGuess("cause"), Exception, Message("Cannot try a guess: game is over"));
    }

    SECTION("Resetting game") {
        // reset an empty game
        game.reset();
        REQUIRE_FALSE(game.hasStarted());
        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());
        REQUIRE(game.getWord().empty());
        REQUIRE(game.getTriedGuesses().empty());
        REQUIRE(game.getGuessesHints().empty());
        REQUIRE(game.getNbGuess() == 0);
        REQUIRE(game.getRules() == rules);

        game.setWord("amont");
        game.start();
        REQUIRE(game.hasStarted());
        game.tryGuess("cause");
        game.tryGuess("amont");
        game.reset();
        REQUIRE_FALSE(game.hasStarted());
        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());
        REQUIRE(game.getWord().empty());
        REQUIRE(game.getTriedGuesses().empty());
        REQUIRE(game.getGuessesHints().empty());
        REQUIRE(game.getNbGuess() == 0);
        REQUIRE(game.getRules() == rules);

    }

    SECTION("Trying a game with unlimited number of guesses") {
        std::string solution = "amont";
        std::string guess = "cause";
        rules->setMaxGuesses(0);
        game.setWord(solution);
        game.start();

        for (unsigned int i = 0; i < UNLIMITED_GUESS_NB; i++) {
            REQUIRE_NOTHROW(game.tryGuess(guess));
        }

        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());
    }

    SECTION("Changing game rules") {
        game.setWord("amont");
        game.start();
        game.tryGuess("cause");
        REQUIRE(game.hasStarted());
        REQUIRE(game.getNbGuess() == 1);
        REQUIRE(game.getWord() == "amont");
        const auto& guesses = game.getTriedGuesses();
        const auto& hints = game.getGuessesHints();

        // change nothing
        game.setRules(nullptr);
        REQUIRE(game.hasStarted());
        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());
        REQUIRE(game.getTriedGuesses() == guesses);
        REQUIRE(game.getGuessesHints() == hints);
        REQUIRE(game.getWord() == "amont");

        game.setRules(rules);
        REQUIRE(game.hasStarted());
        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());
        REQUIRE(game.getTriedGuesses() == guesses);
        REQUIRE(game.getGuessesHints() == hints);
        REQUIRE(game.getWord() == "amont");

        // change rules (and reset game)
        auto otherRules = std::make_shared<WordleGameRules>(getWordleDict());
        game.setRules(otherRules);
        REQUIRE_FALSE(game.hasStarted());
        REQUIRE_FALSE(game.isOver());
        REQUIRE_FALSE(game.isWon());
        REQUIRE(game.getTriedGuesses().empty());
        REQUIRE(game.getGuessesHints().empty());
        REQUIRE(game.getWord().empty());
        REQUIRE(game.getRules() == otherRules);
    }
}

TEST_CASE("Check hints computations", "[game][Lib]") {
    // check invalid inputs
    REQUIRE_THROWS_MATCHES(Game::computeHints("abc", "de"), InvalidArgException,
            Message("Cannot compute hints: words \"abc\" and \"de\" does not have the same size")
    );

    REQUIRE_THROWS_MATCHES(Game::computeHints("abcde", "Abcde"), InvalidArgException,
            Message("Cannot compute hints: words \"abcde\" and \"Abcde\" must be lower-case alphabetical characters.")
    ); // guesses with invalid chars

    REQUIRE_THROWS_MATCHES(Game::computeHints("ABCDE", "abcde"), InvalidArgException,
            Message("Cannot compute hints: words \"ABCDE\" and \"abcde\" must be lower-case alphabetical characters.")
    ); // guesses with invalid chars

    REQUIRE_THROWS_MATCHES(Game::computeHints("1eres", "abcde"), InvalidArgException,
            Message("Cannot compute hints: words \"1eres\" and \"abcde\" must be lower-case alphabetical characters.")
    ); // guesses with invalid chars

    REQUIRE_THROWS_MATCHES(Game::computeHints("abcde", "1eres"), InvalidArgException,
            Message("Cannot compute hints: words \"abcde\" and \"1eres\" must be lower-case alphabetical characters.")
    ); // guesses with invalid chars

    // check words with unique letters
    REQUIRE(Game::computeHints("marie", "tarie") == std::vector{
        WRONG, CORRECT, CORRECT, CORRECT, CORRECT
    });
    REQUIRE(Game::computeHints("tarie", "marie") == std::vector{
        WRONG, CORRECT, CORRECT, CORRECT, CORRECT
    });

    REQUIRE(Game::computeHints("tarie", "tarie") == std::vector{
        CORRECT, CORRECT, CORRECT, CORRECT, CORRECT
    });

    REQUIRE(Game::computeHints("raies", "culot") == std::vector{
        WRONG, WRONG, WRONG, WRONG, WRONG
    });

    REQUIRE(Game::computeHints("quart", "parts") == std::vector{
        WRONG, WRONG, MISPLACED, MISPLACED, MISPLACED
    });
    REQUIRE(Game::computeHints("parts", "quart") == std::vector{
        WRONG, MISPLACED, MISPLACED, MISPLACED, WRONG
    });

    // check words with repeated letters
    REQUIRE(Game::computeHints("email", "maree") == std::vector{
        MISPLACED, MISPLACED, MISPLACED, WRONG, WRONG
    });

    REQUIRE(Game::computeHints("maree", "email") == std::vector{
        MISPLACED, MISPLACED, WRONG, MISPLACED, WRONG
    });

    REQUIRE(Game::computeHints("clees", "maree") == std::vector{
        WRONG, WRONG, MISPLACED, CORRECT, WRONG
    });
    REQUIRE(Game::computeHints("maree", "clees") == std::vector{
        WRONG, WRONG, WRONG, CORRECT, MISPLACED
    });

    REQUIRE(Game::computeHints("puree", "maree") == std::vector{
        WRONG, WRONG, CORRECT, CORRECT, CORRECT
    });
    REQUIRE(Game::computeHints("maree", "puree") == std::vector{
        WRONG, WRONG, CORRECT, CORRECT, CORRECT
    });
}

TEST_CASE("Check template computation", "[game][Lib]") {
    // check invalid inputs

    REQUIRE_THROWS_MATCHES(Game::computeTemplate(5, {"abcde", "fghijk"}, {{WRONG, WRONG, WRONG, WRONG, WRONG}}), InvalidArgException,
            Message("number of guesses and hint vectors must be the same, and word(/hints) sizes also.")
    ); // not the same number of guesses and hints

    REQUIRE_THROWS_MATCHES(Game::computeTemplate(5, {"abcdef"}, {{WRONG, WRONG, WRONG, WRONG, WRONG}}), InvalidArgException,
            Message("number of guesses and hint vectors must be the same, and word(/hints) sizes also.")
    ); // guesses has not the correct length

    REQUIRE_THROWS_MATCHES(Game::computeTemplate(5, {"abcde"}, {{WRONG}}), InvalidArgException,
            Message("number of guesses and hint vectors must be the same, and word(/hints) sizes also.")
    ); // hints has not the correct length

    REQUIRE_THROWS_MATCHES(Game::computeTemplate(5, {"abc"}, {{WRONG, WRONG, WRONG}}), InvalidArgException,
            Message("number of guesses and hint vectors must be the same, and word(/hints) sizes also.")
    ); // guesses and hints have not the correct length

    REQUIRE_THROWS_MATCHES(Game::computeTemplate(5, {"1eres"}, {{WRONG, WRONG, WRONG, WRONG, WRONG}}), InvalidArgException,
            Message("guesses must contain only lower-case alphabetical characters.")
    ); // guesses with invalid chars

    REQUIRE_THROWS_MATCHES(Game::computeTemplate(5, {"ABCde"}, {{WRONG, WRONG, WRONG, WRONG, WRONG}}), InvalidArgException,
            Message("guesses must contain only lower-case alphabetical characters.")
    ); // guesses with invalid case

    // correct inputs

    REQUIRE(Game::computeTemplate(5, {}, {}) == "....."); // initial template

    REQUIRE(Game::computeTemplate(5, {"tarie"}, {
            {CORRECT, CORRECT, CORRECT, CORRECT, CORRECT}
    }) == "tarie"); // correct answer in one guess

    // correct answer in several steps
    REQUIRE(Game::computeTemplate(5, {"temps"}, {
            {CORRECT, WRONG, WRONG, WRONG, WRONG}
    }) == "t....");
    REQUIRE(Game::computeTemplate(5, {"temps", "macha"}, {
            {CORRECT, WRONG, WRONG, WRONG, WRONG},
            {WRONG, CORRECT, WRONG, WRONG, WRONG}
    }) == "ta...");
    REQUIRE(Game::computeTemplate(5, {"temps", "macha", "bords"}, {
            {CORRECT, WRONG, WRONG, WRONG, WRONG},
            {WRONG, CORRECT, WRONG, WRONG, WRONG},
            {WRONG, WRONG, CORRECT, WRONG, WRONG}
    }) == "tar..");
    REQUIRE(Game::computeTemplate(5, {"temps", "macha", "bords", "bouif"}, {
            {CORRECT, WRONG, WRONG, WRONG, WRONG},
            {WRONG, CORRECT, WRONG, WRONG, WRONG},
            {WRONG, WRONG, CORRECT, WRONG, WRONG},
            {WRONG, WRONG, WRONG, CORRECT, WRONG}
    }) == "tari.");
    REQUIRE(Game::computeTemplate(5, {"temps", "macha", "bords", "bouif", "boule"}, {
            {CORRECT, WRONG, WRONG, WRONG, WRONG},
            {WRONG, CORRECT, WRONG, WRONG, WRONG},
            {WRONG, WRONG, CORRECT, WRONG, WRONG},
            {WRONG, WRONG, WRONG, CORRECT, WRONG},
            {WRONG, WRONG, WRONG, WRONG, CORRECT}
    }) == "tarie");

    // partial answers
    REQUIRE(Game::computeTemplate(5, {"tarie"}, {
            {CORRECT, WRONG, CORRECT, WRONG, CORRECT}
    }) == "t.r.e");
    REQUIRE(Game::computeTemplate(5, {"tarie"}, {
            {CORRECT, MISPLACED, CORRECT, MISPLACED, CORRECT}
    }) == "t.r.e");
    REQUIRE(Game::computeTemplate(5, {"tarie"}, {
            {MISPLACED, MISPLACED, MISPLACED, MISPLACED, MISPLACED}
    }) == ".....");
    REQUIRE(Game::computeTemplate(5, {"tarie"}, {
            {WRONG, WRONG, WRONG, WRONG, WRONG}
    }) == ".....");

}

