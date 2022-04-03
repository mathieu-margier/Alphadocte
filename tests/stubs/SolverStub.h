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
 * File: stubs/SolverStub.h
 */

#ifndef SOLVERSTUB_H_
#define SOLVERSTUB_H_

#include <Alphadocte/Solver.h>

class SolverStub : public Alphadocte::Solver {
public:
    SolverStub(std::shared_ptr<Alphadocte::IGameRules> rules);

    // Default constructors/destructor
    virtual ~SolverStub() = default;
    SolverStub(const SolverStub &other) = default;
    SolverStub(SolverStub &&other) = default;
    SolverStub& operator=(const SolverStub &other) = default;
    SolverStub& operator=(SolverStub &&other) = default;

    // Overridden methods
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
     * Trust is an arbitrary number, the higher this number is,
     * the more likely good the guess is.
     *
     * Throws:
     * - Exception : if the template has not been initiated.
     */
    std::vector<std::pair<std::string, double>> computeNextGuesses(size_t n) const override;

};


#endif /* SOLVERSTUB_H_ */
