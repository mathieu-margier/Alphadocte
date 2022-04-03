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
 * File: MotusGameRules.h
 */

#ifndef MOTUSGAMERULES_H_
#define MOTUSGAMERULES_H_

#include <Alphadocte/IGameRules.h>

namespace Alphadocte {

/*
 * Model of Motus's game rules :
 * + the word to be found can be of an arbitrary length
 * + the first letter of the solution is known
 * + only words starting with the same letter as the solution can be used as guesses
 */
class MotusGameRules : public IGameRules {
public:
    // Constructor
    /*
     * Create the rules of a Motus-like game.
     *
     * Args :
     * + dictionary : shared pointer owning the dictionary
     * + maxGuesses : maximum number of guesses for a word, 6 by default
     *
     * Throw an InvalidArgException if dictionary is not loaded or is nullptr.
     */
    MotusGameRules(std::shared_ptr<Dictionary> dictionary, unsigned int maxGuesses = 6);

    // Default constructors/destructor
    virtual ~MotusGameRules() = default;
    MotusGameRules(const MotusGameRules &other) = default;
    MotusGameRules(MotusGameRules &&other) = default;
    MotusGameRules& operator=(const MotusGameRules &other) = default;
    MotusGameRules& operator=(MotusGameRules &&other) = default;

    // Inherited methods
    /*
     * Return the dictionary specified by the rules.
     * Guaranteed to be not nullptr.
     */
    std::shared_ptr<const Dictionary> getDictionary() const override;

    /*
     * Return the maximum number of guesses for one game.
     * A value of 0 means no limit on the number of guesses.
     */
    unsigned int getMaxGuesses() const override;

    /*
     * Set the maximum number of guesses for one game.
     * A value of 0 means no limit on the number of guesses.
     */
    void setMaxGuesses(unsigned int maxGuesses) override;

    /*
     * Checks if the given word can be used as a guess for the current game.
     *
     * Args:
     * - word : the guess to check
     * - solution : the solution of the game
     *
     * Returns true if the word can be used as a try.
     */
    bool isGuessValid(std::string_view word, std::string_view solution) const override;

    /*
     * Checks if the given word can be used as a guess for the current game.
     * Does not check if the solution is valid.
     *
     * Args:
     * - word : the guess to check
     * - solution : the solution of the game, which must have the same length as word
     *
     * Returns true if the word can be used as a solution for a game.
     */
    bool isSolutionValid(std::string_view word) const override;

    /*
     * Return a string representing a pattern for a new guess,
     * based on current hints (ie results from previous result).
     *
     * The string matches the size of the secret word, and each char is
     * either the correct letter at its position if found,
     * or '.' if not yet found.
     *
     * It always show the first letter in motus.
     */
    std::string getTemplate(const Game& game) const override;

    // Fields
private:
    std::shared_ptr<Dictionary> m_dictionary; // cannot be null
    unsigned int m_maxGuesses;
};

} /* namespace Alphadocte */

#endif /* MOTUSGAMERULES_H_ */
