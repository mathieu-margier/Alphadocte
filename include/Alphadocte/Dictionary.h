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
 * File: Dictionnary.h
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <string>
#include <vector>

#include <boost/random/uniform_int.hpp>

namespace Alphadocte {

/*
 * Dictionary interface responsible for loading words of a language.
 */
class Dictionary {
public:
    // Default constructors/destructor
    Dictionary() = default;
    virtual ~Dictionary() = default;
    Dictionary(const Dictionary &other) = default;
    Dictionary(Dictionary &&other) = default;
    Dictionary& operator=(const Dictionary &other) = default;
    Dictionary& operator=(Dictionary &&other) = default;

    // Getters / setters
    /*
     * Return a const reference to the vector containing all of the dictionary words.
     * This vector is guaranteed to exist as long as this object exists.
     * The words in the vector are sorted.
     *
     * The vector is empty if #load() has not been called or has failed.
     */
    const std::vector<std::string>& getAllWords() const;

    // Methods
    /*
     * Draw a random word from all the dictionary words, with an uniform probability.
     *
     * Args:
     * + generator : random generator, which can be of type boost::mt19937 for instance.
     *               It must be initialized with a seed (ie std::time(0))
     *
     * Return an empty string if #load() has not been called or has failed.
     */
    template<typename Generator>
    std::string_view getRandomWord(Generator& generator) const {
        if (!isLoaded())
            return {};

        size_t i = m_distribution(generator);
        return m_words[i];
    }

    /*
     * Return if the given word is inside the dictionary.
     * Use binary search (O(log(n) complexity with n = std::size(word)).
     */
    bool contains(std::string_view word) const;

    // Abstract methods
    /*
     * Load the dictionary words. If already loaded, this function does nothing and returns false.
     *
     * Return true if all the words have been loaded successfully, false otherwise.
     */
    virtual bool load() = 0;

    /*
     * Return if the dictionnary is loaded, ie if load has been called while returning true.
     */
    virtual bool isLoaded() const = 0;

    // Fields
protected:
    std::vector<std::string> m_words;           // vector of words of the dictionary, sorted.
    boost::uniform_int<size_t> m_distribution;  // distribution to draw a random word from the dictionary,
                                                // must be updated if number of words changes
};

} /* namespace Alphadocte */

#endif /* DICTIONARY_H_ */
