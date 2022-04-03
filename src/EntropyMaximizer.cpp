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
 * File: EntropyMaximizer.cpp
 */

#include <Alphadocte/Dictionary.h>
#include <Alphadocte/EntropyMaximizer.h>
#include <Alphadocte/Exceptions.h>
#include <Alphadocte/Game.h>
#include <Alphadocte/IGameRules.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <vector>


// Define operator < as lexical order to be used in a map or a set
bool operator<(const std::vector<Alphadocte::HintType>& lhs, const std::vector<Alphadocte::HintType>& rhs) {
    if (std::size(lhs) != std::size(rhs)) {
        throw Alphadocte::InvalidArgException("Cannot compare hint vectors that does not have the same size",
                "operator<(const std::vector<Alphadocte::HintType>&, const std::vector<Alphadocte::HintType>&)");
    }

    for (size_t i = 0;  i < std::size(lhs); i++) {
        // no need to check out of bounds

        if (lhs[i]< rhs[i])
            return true;
        else if (lhs[i] > rhs[i])
            return false;
        // else evaluate next element
    }

    return false;
}

namespace Alphadocte {

EntropyMaximizer::EntropyMaximizer(std::shared_ptr<IGameRules> rules)
        : Solver(rules, "entropy_maximizer", 1) {}

std::string EntropyMaximizer::computeNextGuess() const {
    auto guessEntropy = computeNextGuesses(1);

    if (guessEntropy.empty()) {
        // no more guess available (solution is probably not in dictionary, or hints are incorrect)
        return "";
    } else {
        return guessEntropy[0].first;
    }
}

std::vector<std::pair<std::string, double>> EntropyMaximizer::computeNextGuesses(size_t n) const {
    std::vector<std::pair<std::string, double>> entropies;

    auto templateWord = getTemplate();
    const auto& solutions = getPotentialSolutions();

    if (templateWord.empty()) {
        throw Exception("cannot compute next guess with an empty template.",
                "Alphadocte::EntropyMaximizer::computeNextGuesses(size_t) const");
    }

    if (solutions.empty()) {
        // no more solutions, cannot
        return entropies;
    } else if (std::size(solutions) == 1) {
        // no need to compute entropies, only one solution possible
        // no choice means 0 bit of entropy
        entropies.emplace_back(std::make_pair(solutions[0], 0));
        return entropies;
    }

    const auto& guesses = getPotentialGuesses();

    // evaluate all guesses
    double expectedEntropy{};
    for (const auto& guess : guesses) {
        expectedEntropy = computeExpectedEntropy(guess);

        // insert guess in the sorted array (sort by descending order of entropy,
        // and favor potential solutions in case of equality)
        bool inserted{false};
        for (auto it = std::begin(entropies); it != std::end(entropies); it++) {
            if (it->second < expectedEntropy) {
                // this guess has a better entropy than the next guess in array
                inserted = true;
                entropies.insert(it, std::make_pair(std::string(guess), expectedEntropy));
                break;
            } else if (it->second == expectedEntropy &&
                    std::find(std::cbegin(solutions), std::cend(solutions), guess) != std::cend(solutions)) {
                // this guess has the same entropy than the next guess in array,
                // but this is also a potential solution -> prioritizes it
                inserted = true;
                entropies.insert(it, std::make_pair(std::string(guess), expectedEntropy));
                break;
            }
        }

        if (!inserted) {
            entropies.emplace_back(std::make_pair(guess, expectedEntropy));
        }
    }

    // keep only top n entries, or less if array is smaller
    entropies.erase(std::begin(entropies) + std::min(n, std::size(entropies)), std::end(entropies));

    return entropies;
}

double EntropyMaximizer::computeActualEntropy(std::string_view guess, const std::vector<HintType>& hints) const {
    size_t occurences{};

    for (std::string_view solution : getPotentialSolutions()) {
        if (hints == Game::computeHints(guess, solution)) {
            occurences++;
        }
    }

    if (occurences == 0) {
        // no solution matches the given hints, either the hints are wrong
        // or the actual solution is not in the dictionary
        return -1;
    }

    return -log2(occurences / static_cast<double>(std::size(getPotentialSolutions())));
}

double EntropyMaximizer::computeExpectedEntropy(std::string_view guess) const {
    std::map<std::vector<HintType>, size_t> occurrences;

    for (std::string_view solution : getPotentialSolutions()) {
        std::vector<HintType> hints = Game::computeHints(guess, solution);

        try {
            occurrences.at(hints)++;
        } catch (const std::out_of_range& e) {
            // first occurrence found
            occurrences.emplace(hints, 1);
        }
    }

    double entropy{};
    double n = static_cast<double>(std::size(getPotentialSolutions()));

    for (const auto& pair : occurrences) {
        entropy += - (pair.second / n) * log2(pair.second / n);
    }

    return entropy;
}

double EntropyMaximizer::computeCurrentEntropy() const {
    const auto& solutions = getPotentialSolutions();

    if (solutions.empty()) {
        // cannot compute entropy of void
        return -1;
    } else {
        return log2(std::size(solutions));
    }
}

} /* namespace Alphadocte */

