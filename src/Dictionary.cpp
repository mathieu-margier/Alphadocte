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
 * File: Dictionary.cpp
 */

#include <Alphadocte/Dictionary.h>
#include <algorithm>



namespace Alphadocte {

const std::vector<std::string>& Dictionary::getAllWords() const {
    return m_words;
}

bool Dictionary::contains(std::string_view word) const {
    return std::binary_search(std::cbegin(m_words), std::cend(m_words), word);
}

} /* namespace Alphadocte */

