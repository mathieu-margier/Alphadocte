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
 * File: WordleGameRules.cpp
 */

#include <Alphadocte/Dictionary.h>
#include <Alphadocte/Exceptions.h>
#include <Alphadocte/Game.h>
#include <Alphadocte/WordleGameRules.h>
#include <utility>


namespace Alphadocte {

// Constructors
WordleGameRules::WordleGameRules(std::shared_ptr<FixedSizeDictionary> dictionary, unsigned int maxGuesses)
        : IGameRules{}, m_dictionary{std::move(dictionary)}, m_maxGuesses{maxGuesses} {
    if (!m_dictionary || !m_dictionary->isLoaded())
        throw InvalidArgException("dictionary is null or not loaded.",
                "Alphadocte::WordleGameRules::WordleGameRules(std::shared_ptr<Alphadocte::FixedSizeDictionary>, unsigned int)");
}


// Getters/setters
word_size WordleGameRules::getWordSize() const {
    return m_dictionary->getWordSize();
}

// Inherited methods
std::shared_ptr<const Dictionary> WordleGameRules::getDictionary() const {
    return m_dictionary;
}

unsigned int WordleGameRules::getMaxGuesses() const {
    return m_maxGuesses;
}

void WordleGameRules::setMaxGuesses(unsigned int maxGuesses) {
    m_maxGuesses = maxGuesses;
}

bool WordleGameRules::isGuessValid(std::string_view word, std::string_view answer) const {
    return std::size(answer) == getWordSize() && std::size(word) == getWordSize() && m_dictionary->contains(word);
}

bool WordleGameRules::isSolutionValid(std::string_view word) const {
    return std::size(word) == getWordSize() && m_dictionary->contains(word);
}

std::string WordleGameRules::getTemplate(const Game& game) const {
    if (game.getWord().empty()) {
        throw InvalidArgException("no word has been set.",
                "Alphadocte::WordleGameRules::getTemplate(const Alphadocte::Game&) const");
    }

    return Game::computeTemplate(std::size(game.getWord()), game.getTriedGuesses(), game.getGuessesHints());
}

} /* namespace Alphadocte */
