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
 * File: TxtDictionary.cpp
 */

#include <Alphadocte/TxtDictionary.h>
#include <fstream>
#include <iostream>
#include <algorithm>


namespace Alphadocte {

TxtDictionary::TxtDictionary(std::filesystem::path filepath)
        : Dictionary{}, m_filepath(std::move(filepath))
        {}


// Getters
const std::filesystem::path& TxtDictionary::getFilepath() const {
    return m_filepath;
}

// Inherited methods
bool TxtDictionary::load() {
    if (!std::filesystem::is_regular_file(m_filepath) || isLoaded())
        return false;

    std::ifstream file{m_filepath};

    if (!file) {
        //std::cerr << "Error opening file " << m_filepath << std::endl;
        return false;
    }

    std::string line;
    bool syntaxError{false};

    // Process while file is readable (ie not end or file, and no errors)
    while (file.good()) {
        std::getline(file, line);

        if (line.empty()) {
            // Skip empty lines
            continue;
        }

        // check that is a single lower-case word
        if (std::any_of(cbegin(line), cend(line), [](char c){ return c < 'a' || c > 'z'; })) {
            syntaxError = true;
            break;
        }

        m_words.emplace_back(std::move(line));
    }

    // true if file have been read until the end, and if each lines are correctly formatted
    bool success = !syntaxError && file.eof();

    if (!success) {
        // Restore state to what it was before the failed loading
        m_words.clear();

        //std::cerr << "Error while reading file " << m_filepath << std::endl;
    }

    // Make sure the vector is sorted
    if (!std::is_sorted(std::cbegin(m_words), std::cend(m_words))) {
        //std::cerr << "Words from dictionary were not sorted, sorting them..." << std::endl;
        std::sort(std::begin(m_words), std::end(m_words));
    }

    // look for duplicated words in sorted vector
    for (auto it = std::cbegin(m_words); (it+1) != std::cend(m_words); it++) {
        if (*it == *(it+1)) {
            //std::cerr << "Dictionary contains duplicated words." << std::endl;
            // found duplicated words, invalid dictionary
            m_words.clear();
            return false;
        }
    }

    m_distribution = boost::uniform_int<size_t>{0, m_words.size() - 1};

    return success;
}

/*
 * Return if the dictionary is loaded, ie if load has been called while returning true.
 */
bool TxtDictionary::isLoaded() const {
    return !m_words.empty();
}

} /* namespace Alphadocte */
