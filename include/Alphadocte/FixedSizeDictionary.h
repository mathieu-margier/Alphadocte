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
 * File: FixedSizeDictionary.h
 */

#ifndef FIXEDSIZEDICTIONARY_H_
#define FIXEDSIZEDICTIONARY_H_

#include <Alphadocte/Dictionary.h>
#include <memory>

#include <Alphadocte/Alphadocte.h>

namespace Alphadocte {

/*
 * Dictionary which only select words of a given length from an underlying dictionary.
 */
class FixedSizeDictionary : public Dictionary {
public:
    // Constructors
    /*
     * Initialize a fixed-size dictionary, which keeps only words of a given length from a dictionary.
     *
     * Args:
     * + dictionary : shared pointer owning the underlying dictionary
     * + wordSize : size of the words wanted in this dictionary
     */
    FixedSizeDictionary(std::shared_ptr<Dictionary> dictionary, word_size wordSize);

    // Default constructors / destructor
    virtual ~FixedSizeDictionary() = default;
    FixedSizeDictionary(const FixedSizeDictionary &other) = default;
    FixedSizeDictionary(FixedSizeDictionary &&other) = default;
    FixedSizeDictionary& operator=(const FixedSizeDictionary &other) = default;
    FixedSizeDictionary& operator=(FixedSizeDictionary &&other) = default;

    // Getters / setters
    /*
     * Return the size of the words in this dictionary.
     */
    word_size getWordSize() const;

    // Inherited methods
    /*
     * Load the dictionary words. If already loaded, this function does nothing and returns false.
     *
     * Return true if all the words have been loaded successfully, false otherwise.
     */
    bool load() override;

    /*
     * Return if the dictionary is loaded, ie if load has been called while returning true.
     */
    bool isLoaded() const override;

    // Fields
private:
    const word_size m_wordSize;
    std::shared_ptr<Dictionary> m_internalDict;
};

} /* namespace Alphadocte */

#endif /* FIXEDSIZEDICTIONARY_H_ */
