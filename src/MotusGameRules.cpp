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
 * File: MotusGameRules.cpp
 */

#include <Alphadocte/Dictionary.h>
#include <Alphadocte/Exceptions.h>
#include <Alphadocte/Game.h>
#include <Alphadocte/MotusGameRules.h>
#include <utility>


namespace Alphadocte {

MotusGameRules::MotusGameRules(std::shared_ptr<Dictionary> dictionary, unsigned int maxGuesses)
        : IGameRules{}, m_dictionary{std::move(dictionary)}, m_maxGuesses{maxGuesses} {
    if (!m_dictionary || !m_dictionary->isLoaded())
        throw InvalidArgException("dictionary is null or not loaded.",
                "Alphadocte::MotusGameRules::MotusGameRules(std::shared_ptr<Alphadocte::Dictionary>, unsigned int)");
}

// Inherited methods
std::shared_ptr<const Dictionary> MotusGameRules::getDictionary() const {
    return m_dictionary;
}

unsigned int MotusGameRules::getMaxGuesses() const {
    return m_maxGuesses;
}

void MotusGameRules::setMaxGuesses(unsigned int maxGuesses) {
    m_maxGuesses = maxGuesses;
}

bool MotusGameRules::isGuessValid(std::string_view word, std::string_view answer) const  {
    if (answer.empty())
        return false;

    return std::size(word) == std::size(answer) && word[0] == answer[0] && m_dictionary->contains(word);
}

bool MotusGameRules::isSolutionValid(std::string_view word) const {
    return !word.empty() && m_dictionary->contains(word);
}

std::string MotusGameRules::getTemplate(const Game& game) const {
    if (game.getWord().empty()) {
        throw InvalidArgException("no word has been set.",
                "Alphadocte::MotusGameRules::getTemplate(const Alphadocte::Game) const");
    }

    auto word = game.getWord();
    std::string pattern = Game::computeTemplate(std::size(word), game.getTriedGuesses(), game.getGuessesHints());

    // first letter is always known
    pattern.at(0) = word.at(0);

    return pattern;
}

} /* namespace Alphadocte */
