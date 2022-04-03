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
 * File: WordleGameRules.h
 */

#ifndef WORDLEGAMERULES_H_
#define WORDLEGAMERULES_H_

#include <Alphadocte/FixedSizeDictionary.h>
#include <Alphadocte/IGameRules.h>

namespace Alphadocte {

/*
 * Model of Wordle's game rules :
 * + word to be found must be of length n
 * + any n-letter long word from the dictionary can be used as guess
 */
class WordleGameRules : public IGameRules {
public:
    // Constructors
    /*
     * Create the rules of a Motus-like game.
     *
     * Args :
     * + dictionary : the dictionary of fixed-size, which defines n
     * + maxGuesses : maximum number of guesses for a word, 6 by default
     *
     * Throw an InvalidArgException if dictionary is nullptr, or is not loaded.
     */
    WordleGameRules(std::shared_ptr<FixedSizeDictionary> dictionary, unsigned int maxGuesses = 6);

    // Default constructors/destructor
    virtual ~WordleGameRules() = default;
    WordleGameRules(const WordleGameRules &other) = default;
    WordleGameRules(WordleGameRules &&other) = default;
    WordleGameRules& operator=(const WordleGameRules &other) = default;
    WordleGameRules& operator=(WordleGameRules &&other) = default;

    // Getters/setters
    /*
     * Return the constant word size, defined by the dictionary.
     * All words (guesses and solution) must have the returned size.
     */
    word_size getWordSize() const;

    // Inherited methods
    /*
     * Return the dictionary specified by the rules.
     * Guaranteed to be not nullptr.
     */
    virtual std::shared_ptr<const Dictionary> getDictionary() const override;

    /*
     * Return the maximum number of guesses for one game.
     * A value of 0 means no limit on the number of guesses.
     */
    virtual unsigned int getMaxGuesses() const override;

    /*
     * Set the maximum number of guesses for one game.
     * A value of 0 means no limit on the number of guesses.
     */
    virtual void setMaxGuesses(unsigned int maxGuesses) override;

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
    bool isGuessValid(std::string_view word, std::string_view solution) const override;

    /*
     * Check if the given word can be used as a solution for a game.
     *
     * Args:
     * - word : the potential solution to check
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
     */
    std::string getTemplate(const Game& game) const override;

    // Fields
private:
    std::shared_ptr<FixedSizeDictionary> m_dictionary; // cannot be nullptr.
    unsigned int m_maxGuesses;
};

} /* namespace Alphadocte */

#endif /* WORDLEGAMERULES_H_ */
