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
 * File: EntropyMaximizer.h
 */

#ifndef ENTROPYMAXIMIZER_H_
#define ENTROPYMAXIMIZER_H_

#include <Alphadocte/Solver.h>

namespace Alphadocte {

/*
 * Solver that produces guess by maximizing the expected entropy associated with the hint information.
 * Inspired by videos from 3blue1brown (EN: https://www.youtube.com/watch?v=v68zYyaEmEA
 * and https://www.youtube.com/watch?v=fRed0Xmc2Wg)
 * and Science Etonnante (FR: https://www.youtube.com/watch?v=iw4_7ioHWF4)
 */
class EntropyMaximizer : public Solver {
public:
    /*
     * Initialize the entropy maximizer solver.
     *
     * Args:
     * - rules : the rules of the game the solver is trying to solve.
     *           Also contains the dictionary used by the solver.
     *
     * Throws:
     * - InvalidArgException : if rules is a nullptr
     */
    EntropyMaximizer(std::shared_ptr<IGameRules> rules);

    // Default constructors/destructor
    virtual ~EntropyMaximizer() = default;
    EntropyMaximizer(const EntropyMaximizer &other) = default;
    EntropyMaximizer(EntropyMaximizer &&other) = default;
    EntropyMaximizer& operator=(const EntropyMaximizer &other) = default;
    EntropyMaximizer& operator=(EntropyMaximizer &&other) = default;

    // Inherited methods
    /*
     * Compute the next guess suggested by the solver.
     *
     * Abstract method.
     *
     * Throws:
     * - Exception : if the template has not been initiated.
     */
    std::string computeNextGuess() const override;

    /*
     * Compute the next guesses suggested by the solver.
     *
     * Abstract method.
     *
     * Args :
     * - n : the number of guesses wanted. The actual number of guesses
     *       returned might be lower, if not enough solutions are available.
     *
     * Return the (sorted by descending trust) vector of pairs
     * of guesses and their trust values.
     * Can also be empty if no solution are available.
     *
     * For this class, trust is the expected entropy (in bits) revealed by the guess.
     * The higher this number is, the more likely good the guess is.
     *
     * Throws:
     * - Exception : if the template has not been initiated.
     */
    std::vector<std::pair<std::string, double>> computeNextGuesses(size_t n) const override;

    //  Own methods
    /*
     * Compute the actual entropy (in bits) revealed by the guess,
     * ie the actual quantity of information gained.
     * It will be subtracted from the current entropy when the guess and its hints  added.
     * Must be called before adding the hints to the solver,
     * since it relies on the potential solutions available,
     * which are updated when adding hints.
     *
     * Args :
     * - guess : the guess used
     * - hints : the hints revealed by the guess
     *
     * Returns an arbitrary negative value if none known word matches the guess' hints
     * (cannot compute log of 0, either the actual solution is not in the dictionary,
     *  or the hints are incorrect)
     */
    double computeActualEntropy(std::string_view guess, const std::vector<HintType>& hints) const;

    /*
     * Compute the expected entropy loss (in bits) that a guess can reveal,
     * based on the potential solutions so far.
     * It represents the mean quantity of information gained by this guess,
     * which can be expected to be removed from the current entropy.
     *
     * Args:
     * - word : the candidate word used as a guess
     */
    double computeExpectedEntropy(std::string_view guess) const;

    /*
     * Compute the number of bits of the current entropy, which
     * measure the quantity of missing information in order to win the game.
     * It is the (logarithmic) size of the solutions space.
     *
     * Returns an arbitrary negative value if there is no potential solution in the dictionary.
     */
    double computeCurrentEntropy() const;
};

} /* namespace Alphadocte */

#endif /* ENTROPYMAXIMIZER_H_ */
