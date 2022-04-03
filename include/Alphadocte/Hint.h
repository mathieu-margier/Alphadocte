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
 * File: Hint.h
 */

#ifndef HINT_H_
#define HINT_H_

#include <string_view>
#include <ostream>
#include <vector>

namespace Alphadocte {

/*
 * All the possible type of hint that is revealed for a letter of a word
 */
enum class HintType : char {
    WRONG,     // the letter is not in the solution
    MISPLACED, // the letter is in the solution, but not at this place
    CORRECT    // the letter is in the solution, and in this position
};

std::ostream& operator<<(std::ostream& os, HintType hintType);
/*
 * Check if the given word is compatible with the given hints associated with a previous hint,
 * ie if the word could be the solution based on a previous guess.
 *
 * Args :
 * - word : the word to check
 * - guess : a guess used previously
 * - hints : the hints associated to the guess
 */
bool matches(std::string_view word, std::string_view guess, const std::vector<HintType>& hints);

} /* namespace Alphadocte */

#endif /* HINT_H_ */
