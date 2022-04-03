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
 * File: Hint.cpp
 */

#include <Alphadocte/Hint.h>
#include <set>

#include <Alphadocte/Alphadocte.h>

namespace Alphadocte {

std::ostream& operator<<(std::ostream& os, HintType hintType) {
    switch (hintType) {
    case HintType::CORRECT:
        os << "correct";
        break;
    case HintType::MISPLACED:
        os << "not here";
        break;
    case HintType::WRONG:
        os << "wrong";
        break;
    default:
        os << "undefined";
        break;
    }

    return os;
}

bool matches(std::string_view word, std::string_view guess, const std::vector<HintType>& hints) {
    if (std::size(word) != std::size(guess) || std::size(guess) != std::size(hints)) {
        // All arguments must have the same length
        return false;
    }
    // No need to check out of bounds from now on

    std::multiset<char> remainingLetters(std::cbegin(word), std::cend(word));

    // First check correct guess
    for (word_size i = 0; i < std::size(hints); i++) {
        switch (hints[i]) {
        case HintType::CORRECT:
            if (word[i] != guess[i]) {
                // CORRECT hint not respected (letter must be at this position)
                return false;
            } else {
                // CORRECT hint respected
                // remove one occurrence of this letter from the word (guaranteed to be in the set)
                remainingLetters.erase(remainingLetters.find(word[i]));
            }
            break;
        default:
            // Ignore non-correct hints
            break;
        }
    }

    // Then check other hints
    // Order (left to right) is important for duplicated letters, as for those letters
    // the misplaced hints always appear before the wrong hints.
    // (e.g for a solution who has only one 'e' at the start, the guess "maree" will have the first 'e' misplaced,
    // and the second one wrong, the reverse order will never appear)
    for (word_size i = 0; i < std::size(hints); i++) {
        switch (hints[i]) {
        case HintType::MISPLACED:
            if (word[i] == guess[i]) {
                // MISPLACED hint not respected (letter cannot be at this position)
                return false;
            } else if (auto it = remainingLetters.find(guess[i]); it != std::end(remainingLetters)) {
                // MISPLACED hint respected (letter not at this position, but elsewhere in the word)
                remainingLetters.erase(it);
            } else {
                // MISPLACED hint not respected (letter must appear in the word elsewhere)
                return false;
            }
            break;
        case HintType::WRONG:
            if (auto it = remainingLetters.find(guess[i]); it != std::cend(remainingLetters)) {
                // WRONG hint not respected (letter must not appear in the word)
                return false;
            }
            break;
        default:
            // Ignore correct hints
            break;
        }
    }

    // All hints are respected
    return true;
}

} /* namespace Alphadocte */
