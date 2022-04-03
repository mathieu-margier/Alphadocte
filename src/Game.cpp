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
 * File: Game.cpp
 */

#include <Alphadocte/Exceptions.h>
#include <Alphadocte/Game.h>
#include <Alphadocte/IGameRules.h>
#include <algorithm>


namespace Alphadocte {

// Constructors
Game::Game(std::shared_ptr<IGameRules> rules)
        : m_word{}, m_rules{std::move(rules)}, m_guesses{},
          m_hints{}, m_start{false}, m_win{false} {
    if (!m_rules) {
        throw InvalidArgException("rules cannot be null",
                "Alphadocte::Game::Game(std::shared_ptr<Alphadocte::IGameRules>)");
    }
}

// Getters/Setters
bool Game::hasStarted() const {
    return m_start;
}

bool Game::isOver() const {
    return m_win || (m_rules->getMaxGuesses() != 0 && std::size(m_guesses) >= m_rules->getMaxGuesses());
}

bool Game::isWon() const {
    return m_win;
}

unsigned int Game::getNbGuess() const {
    return std::size(m_guesses);
}

std::string_view Game::getWord() const {
    return m_word;
}

void Game::setWord(std::string word) {
    if (!m_rules->isSolutionValid(word))
        throw InvalidArgException("the word " + word + " is not a valid solution", "Alphadocte::Game::setWord(std::string)");

    if (m_start && word != m_word) {
        throw Exception("Cannot change word while the game is playing.", "Alphadocte::Game::setWord(std::string)");
    }

    m_word = word;
}

const std::vector<std::string>& Game::getTriedGuesses() const {
    return m_guesses;
}

const std::vector<std::vector<HintType>>& Game::getGuessesHints() const {
    return m_hints;
}

std::shared_ptr<const IGameRules> Game::getRules() const {
    return m_rules;
}

void Game::setRules(std::shared_ptr<IGameRules> rules) {
    if (m_rules == rules || !rules)
        // nothing to do
        return;

    if (hasStarted())
        reset();

    m_rules = rules;

    // Check if answer is still valid, otherwise clear it
    if (!m_rules->isSolutionValid(m_word))
        m_word = "";
}

// Methods
void Game::reset() {
    m_start = m_win = false;
    m_guesses.clear();
    m_hints.clear();
    m_word.clear();
}

void Game::start() {
    if (m_start)
        throw Exception("Cannot start game: game has already been started", "Alphadocte::Game::start()");

    if (m_word.empty())
        throw Exception("Cannot start game: no word has been set", "Alphadocte::Game::start()");

    m_start = true;
}

const std::vector<HintType>& Game::tryGuess(std::string word) {
    if (!hasStarted())
        throw Exception("Cannot try a guess: game has not been started", "Alphadocte::Game::tryGuess(std::string)");
    if (isOver())
        throw Exception("Cannot try a guess: game is over", "Alphadocte::Game::tryGuess(std::string)");
    if (!m_rules->isGuessValid(word, m_word))
        throw Exception("Cannot try a guess: invalid guess", "Alphadocte::Game::tryGuess(std::string)");

    m_guesses.emplace_back(std::move(word));
    m_hints.emplace_back(computeHints(m_guesses.back(), m_word));
    const auto& hints = m_hints.back();

    m_win = std::count(std::cbegin(hints), std::cend(hints), HintType::CORRECT) == std::distance(std::cbegin(hints), std::cend(hints));

    return hints;
}

// Static methods
std::vector<HintType> Game::computeHints(std::string_view word, std::string_view solution) {

    if (std::size(word) != std::size(solution)) {
        throw InvalidArgException("Cannot compute hints: words \"" + std::string(word) +
                "\" and \"" + std::string(solution) + "\" does not have the same size",
                "Alphadocte::Game::computeHints(std::string_view, std::string_view)");
    }

    if (!std::all_of(std::cbegin(word), std::cend(word), islower)
            || !std::all_of(std::cbegin(solution), std::cend(solution), islower)) {
        throw InvalidArgException("Cannot compute hints: words \"" + std::string(word) +
                "\" and \"" + std::string(solution) + "\" must be lower-case alphabetical characters.",
                "Alphadocte::Game::computeHints(std::string_view, std::string_view)");
    }

    std::vector<HintType> hints{std::size(word)};
    std::fill(std::begin(hints), std::end(hints), HintType::WRONG); // By default all hints are incorrect
    std::multiset<char> remainingLetters{std::cbegin(solution), std::cend(solution)};

    // First check the letters at correct positions
    for (word_size i = 0; i < std::size(word); i++) {
        if (word.at(i) == solution.at(i)) {
            hints.at(i) = HintType::CORRECT;
            remainingLetters.erase(remainingLetters.find(word.at(i)));
        }
    }

    // Then check if the other letters are at an incorrect position or just missing
    for (word_size i = 0; i < std::size(word); i++) {
        if (hints.at(i) != HintType::CORRECT) {
            // Make sure we don't have computed an hit for this position yet

            if (auto it = remainingLetters.find(word.at(i)); it != std::cend(remainingLetters)) {
                // Letter is in the word, but not at this position
                hints.at(i) = HintType::MISPLACED;
                remainingLetters.erase(it);

            }
            // else: letter is not in the word, leave at as invalid
        }
    }

    return hints;
}

std::string Game::computeTemplate(size_t wordSize, const std::vector<std::string>& guesses, const std::vector<std::vector<HintType>>& hints){
    std::string pattern;
    std::fill_n(std::back_inserter(pattern), wordSize, '.');

    if (std::size(guesses) != std::size(hints)
            || std::any_of(std::cbegin(guesses), std::cend(guesses), [wordSize](const auto& guess)  { return std::size(guess) != wordSize; })
            || std::any_of(std::cbegin(hints), std::cend(hints), [wordSize](const auto& hintVector) { return std::size(hintVector) != wordSize; })) {
        throw InvalidArgException("number of guesses and hint vectors must be the same, and word(/hints) sizes also.",
                "Alphadocte::Game::computeTemplate(size_t, const std::vector<std::string>&, const std::vector<std::vector<Alphadocte::HintType>>&)");
    }
    // no out of bounds for range [0, wordSize) from now on

    // check if guesses's chars are valid (not checking if valid per rules)
    for (const auto& guess : guesses) {
        if (!std::all_of(std::cbegin(guess), std::cend(guess), islower)) {
            throw InvalidArgException("guesses must contain only lower-case alphabetical characters.",
                    "Alphadocte::Game::computeTemplate(size_t, const std::vector<std::string>&, const std::vector<std::vector<Alphadocte::HintType>>&)");
        }
    }

    for (size_t i = 0; i < std::size(guesses); i++) {
        for (word_size j = 0; j < wordSize; j++) {
            if (hints[i][j] == HintType::CORRECT) {
                pattern[j] = guesses[i][j];
            }
        }
    }

    return pattern;
}

} /* namespace Alphadocte */
