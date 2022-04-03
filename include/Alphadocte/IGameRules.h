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
 * File: GameRules.h
 */

#ifndef IGAMERULES_H_
#define IGAMERULES_H_

#include <string_view>
#include <memory>

namespace Alphadocte {

class Game;

class Dictionary;

/*
 * Interface to model all the potential rules for variant of Motus/Wordle-like games.
 */
class IGameRules {
public:
    // Default constructors/destructor
    IGameRules() = default;
    virtual ~IGameRules() = default;
    IGameRules(const IGameRules &other) = default;
    IGameRules(IGameRules &&other) = default;
    IGameRules& operator=(const IGameRules &other) = default;
    IGameRules& operator=(IGameRules &&other) = default;

    // Methods
    /*
     * Return the dictionary specified by the rules.
     */
    virtual std::shared_ptr<const Dictionary> getDictionary() const = 0;

    /*
     * Return the maximum number of guesses for one game.
     * A value of 0 means no limit on the number of guesses.
     */
    virtual unsigned int getMaxGuesses() const = 0;

    /*
     * Set the maximum number of guesses for one game.
     * A value of 0 means no limit on the number of guesses.
     */
    virtual void setMaxGuesses(unsigned int maxGuesses) = 0;

    /*
     * Checks if the given word can be used as a guess for the current game.
     * Does not check if the solution is valid.
     *
     * Args:
     * - word : the guess to check
     * - solution : the solution of the game, which must have the same length as word
     *
     * Returns true if the word can be used as a try.
     */
    virtual bool isGuessValid(std::string_view word, std::string_view solution) const = 0;

    /*
     * Check if the given word can be used as a solution for a game.
     *
     * Args:
     * - word : the potential solution to check
     *
     * Returns true if the word can be used as a solution for a game.
     */
    virtual bool isSolutionValid(std::string_view word) const = 0;

    /*
     * Return a string representing a pattern for a new guess,
     * based on current hints (ie results from previous result).
     *
     * The string matches the size of the secret word, and each char is
     * either the correct letter at its position if found,
     * or '.' if not yet found.
     */
    virtual std::string getTemplate(const Game& game) const = 0;
};

} /* namespace Alphadocte */

#endif /* IGAMERULES_H_ */
