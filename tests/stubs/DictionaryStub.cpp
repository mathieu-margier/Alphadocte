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
 * File: stubs/DictionaryStub.cpp
 */

#include <algorithm>

#include "DictionaryStub.h"

namespace Alphadocte {

DictionaryStub::DictionaryStub(std::vector<std::string> words)
: Dictionary() {
    m_words = std::move(words);
    std::sort(std::begin(m_words), std::end(m_words));
    m_distribution = boost::uniform_int<size_t>{0, m_words.size() - 1};
}

bool DictionaryStub::load() {
    return false;
}

bool DictionaryStub::isLoaded() const {
    return true;
}

} /* namespace Alphadocte */
