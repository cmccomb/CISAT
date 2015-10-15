//  ┌─┬─────┐  ││  CISAT: The Cognitively-Inspired Simulated
//  ├─┼─────┤  ││         Annealing Teams Modeling Framework
//  │ │ McC │  ││  include
//  └─┴─────┘  ││   └─utilities
//             ││      └─stats.hpp

#ifndef UTILITIES__STATS_HPP
#define UTILITIES__STATS_HPP

#include <vector>
#include <cstdlib>
#include <cmath>
#include <numeric>
#include <cfloat>
#include <deque>

// Function to return maximum value of vector
long double vector_maximum(std::vector<long double> v);

// Function that returns the index of the maximum value
long vector_argmax(std::vector<long double> v);

// Function to return minimum value of vector
long double vector_minimum(std::vector<long double> v);

// Function that returns the index of the minimum value
long vector_argmin(std::vector<long double> v);

// Compute mean of a vector
long double mean(std::vector<long double> x);

// Compute mean of a deque
long double stdev(std::deque<long double> x);

// Compute standard deviation of a list
long double stdev(std::deque<long double> x);

long double apply_weighting(std::vector<long double> a, std::vector<long double> b);

#endif