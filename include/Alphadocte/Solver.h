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
 * File: Solver.h
 */

#ifndef SOLVER_H_
#define SOLVER_H_

#include <Alphadocte/Hint.h>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace Alphadocte {

// Forward declarations
class IGameRules;

/*
 * Base class defining the requirements of a solver.
 */
class Solver {
public:
    // Constructos
    /*
     * Base constructor initializing the solver;
     *
     * Args:
     * - rules : the rules of the game the solver is trying to solve.
     *           Also contains the dictionary used by the solver.
     * - sovlerName : a name that uniquely identify the solver (generally one by class)
     * - solverVersion : a number that is incremented each time the solver algorithm is modified
     *                   (ie previous results are invalidated)
     *
     * Throws:
     * - InvalidArgException : if rules is a nullptr
     */
    Solver(std::shared_ptr<IGameRules> rules, std::string solverName, unsigned int solverVersion);

    // Default constructors/destructor
    virtual ~Solver() = default;
    Solver(const Solver &other) = default;
    Solver(Solver &&other) = default;
    Solver& operator=(const Solver &other) = default;
    Solver& operator=(Solver &&other) = default;

    // Getters/Setters
    /*
     * Returns the current known hints.
     */
    const std::map<std::string, std::vector<HintType>>& getHints() const;

    /*
     * Get the rules of the game used by the solver.
     * Guaranteed to not be nullptr.
     */
    std::shared_ptr<const IGameRules> getRules() const;

    /*
     * Return the template for the solution. It is used to check if a guess is valid.
     *
     * It has the same length as the answer,
     * with each char being either '.' (allow any character),
     * or the letter of the word at its position.
     */
    std::string_view getTemplate() const;

    /*
     * Set the template for the solution. It is used to check if a guess is valid.
     *
     * It must have the same length as the answer,
     * with each char being either '.' (allow any character),
     * or the letter of the word at its position.
     *
     * Any invalid template is discarded.
     */
    void setTemplate(std::string wordTemplate);

    /*
     * Set the rules of the game used by the solver.
     * Reset the solver if those are different from the current rules.
     * Does nothing if the nullptr is given.
     *
     * Args:
     * - rules : the game's rules used by the solver
     */
    void setRules(std::shared_ptr<IGameRules> rules);

    /*
     * Return the vector of potential guesses accepted by the game.
     *
     * Strings belong to the dictionary.
     */
    const std::vector<std::string_view>& getPotentialGuesses() const;

    /*
     * Return the vector of potential solutions accepted by the game.
     * The potential solutions respect all the hints given so far.
     *
     * Strings belong to the dictionary.
     */
    const std::vector<std::string_view>& getPotentialSolutions() const;

    /*
     * Return a name that uniquely identify this solver
     * (or rather this solver's class, not this instance)
     */
    std::string_view getSolverName() const;

    /*
     * Return the version of the solver.
     * The version is incremented each time a significant (in regard to the results)
     * change is made to the algorithm.
     */
    unsigned int getSolverVersion() const;

    // Methods
    /*
     * Compute the next guess suggested by the solver.
     *
     * Abstract method.
     *
     * Throws:
     * - Exception : if the template has not been initiated.
     */
    virtual std::string computeNextGuess() const = 0;

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
    virtual std::vector<std::pair<std::string, double>> computeNextGuesses(size_t n) const = 0;

    /*
     * Add a hint to the solver, provided by the game after a guess.
     *
     * Args:
     * - guess : the guess that generated those hints
     * - hints : hint vector that will be stored in the solver
     */
    void addHint(std::string_view guess, const std::vector<HintType>& hints);

    /*
     * Reset the solver state, by erasing all hints.
     */
    void reset();

protected:
    void populateGuesses();

    void populateSolutions();

    // Fields
private:
    std::shared_ptr<IGameRules> m_rules;                   // cannot be nullptr
    std::map<std::string, std::vector<HintType>> m_hints;
    std::string m_wordTemplate;
    std::vector<std::string_view> m_potentialGuesses;
    std::vector<std::string_view> m_potentialSolutions;
    std::string m_solverName;
    unsigned int m_solverVersion;
};

} /* namespace Alphadocte */

#endif /* SOLVER_H_ */
