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
 * File: Solver.cpp
 */

#include <Alphadocte/Dictionary.h>
#include <Alphadocte/Exceptions.h>
#include <Alphadocte/IGameRules.h>
#include <Alphadocte/Solver.h>
#include <algorithm>


namespace Alphadocte {

Solver::Solver(std::shared_ptr<IGameRules> rules, std::string name, unsigned int version)
        : m_rules{std::move(rules)}, m_hints{},
          m_potentialGuesses{}, m_potentialSolutions{},
          m_solverName{std::move(name)}, m_solverVersion{version} {
    if (!m_rules) {
        throw InvalidArgException("rules cannot be null",
                "Alphadocte::Solver::Solver(std::shared_ptr<Alphadocte::IGameRules>, std::string, unsigned int)");
    }
}

// Getters/Setters
const std::map<std::string, std::vector<HintType>>& Solver::getHints() const {
    return m_hints;
}

std::shared_ptr<const IGameRules> Solver::getRules() const {
    return m_rules;
}

void Solver::setRules(std::shared_ptr<IGameRules> rules) {
    if (rules == m_rules || !rules) {
        // no change
        return;
    }

    m_rules = std::move(rules);
    reset();
}

std::string_view Solver::getTemplate() const {
    return m_wordTemplate;
}

void Solver::setTemplate(std::string wordTemplate) {
    // to lower case
    std::transform(std::begin(wordTemplate), std::end(wordTemplate), std::begin(wordTemplate),tolower);

    if (!std::all_of(std::cbegin(wordTemplate), std::cend(wordTemplate), [](char c){
            return c == '.' || (c >= 'a' && c <= 'z');
        })) {
        // invalid template
        throw InvalidArgException("invalid template, must contain either '.' or letters.",
                "Alphadocte::Solver::setTemplate(std::string)");
    }

    reset();
    m_wordTemplate = std::move(wordTemplate);

    if (m_wordTemplate.empty()) {
        return;
    }

    populateGuesses();
    populateSolutions();

    // update solutions
    m_potentialSolutions.erase(
            std::remove_if(
                    std::begin(m_potentialSolutions),
                    std::end(m_potentialSolutions),
                    [this](const auto& word) {

        // check solution matching template
        if (std::size(word) != std::size(this->m_wordTemplate))
            return true;

        for (size_t i = 0; i < std::size(word); i++) {
            if (this->m_wordTemplate.at(i) != '.' && this->m_wordTemplate.at(i) != word.at(i)) {
                return true;
            }
        }

        return false;
    }), std::end(m_potentialSolutions));

}

const std::vector<std::string_view>& Solver::getPotentialGuesses() const {
    return m_potentialGuesses;
}

const std::vector<std::string_view>& Solver::getPotentialSolutions() const {
    return m_potentialSolutions;
}

/*
 * Return a name that uniquely identify this solver
 * (or rather this solver's class, not this instance)
 */
std::string_view Solver::getSolverName() const {
    return m_solverName;
}

/*
 * Return the version of the solver.
 * The version is incremented each time a significant (in regard to the results)
 * change is made to the algorithm.
 */
unsigned int Solver::getSolverVersion() const {
    return m_solverVersion;
}

// Methods
void Solver::addHint(std::string_view guess, const std::vector<HintType>& hints) {
    if (m_wordTemplate.empty()) {
        throw Exception("template needs to be set before adding hints.",
                "Alphadocte::Solver::addHint(std::string_view, const std::vector<Alphadocte::HintType>&)");
    }

    if (!m_rules->isGuessValid(guess, m_wordTemplate)) {
        throw InvalidArgException("guess is not a valid guess.",
                "Alphadocte::Solver::addHint(std::string_view, const std::vector<Alphadocte::HintType>&)");
    }

    if (std::size(guess) != std::size(hints)) {
        throw InvalidArgException("the number of hints does not match the guess' number of letters.",
                "Alphadocte::Solver::addHint(std::string_view, const std::vector<Alphadocte::HintType>&)");
    }

    m_hints.emplace(guess, hints);

    // update solutions
    m_potentialSolutions.erase(
            std::remove_if(
                    std::begin(m_potentialSolutions),
                    std::end(m_potentialSolutions),
                    [&hints, &guess](const auto& word){  return !matches(word, guess, hints);  }
    ), std::end(m_potentialSolutions));

}

void Solver::reset() {
    m_hints.clear();
    m_wordTemplate.clear();
    m_potentialGuesses.clear();
    m_potentialSolutions.clear();
}

void Solver::populateGuesses() {
    if (m_wordTemplate.empty()) {
        throw InvalidArgException("template not set.",
                "Alphadocte::Solver::populateGuesses()");
    }

    const auto& allWords = m_rules->getDictionary()->getAllWords();

    m_potentialGuesses.clear();
    std::copy_if(allWords.cbegin(), allWords.cend(), std::back_inserter(m_potentialGuesses), [this](const auto& word){
        return this->m_rules->isGuessValid(word, this->m_wordTemplate);
    });
}

void Solver::populateSolutions() {
    if (m_wordTemplate.empty()) {
        throw InvalidArgException("template not set.",
                "Alphadocte::Solver::populateSolutions()");
    }

    const auto& allWords = m_rules->getDictionary()->getAllWords();

    m_potentialSolutions.clear();
    std::copy_if(allWords.cbegin(), allWords.cend(), std::back_inserter(m_potentialSolutions), [this](const auto& word){
        // check solution accepted by rules
        return this->m_rules->isSolutionValid(word);
    });
}

} /* namespace Alphadocte */
