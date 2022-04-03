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
 * File: FixedSizeDictionary.cpp
 */

#include <Alphadocte/FixedSizeDictionary.h>
#include <utility>
#include <algorithm>
#include <iterator>


namespace Alphadocte {

// Constructors
FixedSizeDictionary::FixedSizeDictionary(std::shared_ptr<Dictionary> dictionary, word_size wordSize)
        : Dictionary{}, m_wordSize{wordSize}, m_internalDict{std::move(dictionary)}
        {}


// Getters/setters
word_size FixedSizeDictionary::getWordSize() const {
    return m_wordSize;
}

// Inherited methods
bool FixedSizeDictionary::load() {
    if (isLoaded()) {
        // Abort if already loaded or internal dictionary cannot be loaded
        return false;
    }

    if (!m_internalDict->isLoaded() && !m_internalDict->load()) {
        // Abort if internal dictionary is not loaded and fail to load
        return false;
    }
    // dictionary successfully loaded, no need to check if words are sorted and if there are duplicates

    const auto& internalWords = m_internalDict->getAllWords();
    std::copy_if(cbegin(internalWords), cend(internalWords), std::back_inserter(m_words),
            [this](const auto& word){ return word.size() == this->m_wordSize; });

    if (m_words.empty()) {
        // Could not find any word, consider loading has failed
        // Do not release internal dictionary in case load is called once again
        return false;
    }

    // Internal dictionary's words are supposed to be sorted, so there is no need to check
    // if the words are sorted, since copy_if preserves the order of the iterator.

    // We no longer need the internal dictionary, it can be freed if no one else uses it
    m_internalDict.reset();

    // Update probabilistic distribution
    m_distribution = boost::uniform_int<size_t>{0, m_words.size() - 1};

    return true;
}

/*
 * Return if the dictionary is loaded, ie if load has been called while returning true.
 */
bool FixedSizeDictionary::isLoaded() const {
    return !m_words.empty();
}

} /* namespace Alphadocte */
