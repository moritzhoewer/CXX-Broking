/*
 * Copyright © 2017-2018 Steven Beyermann, Markus Blechschmidt, Moritz Höwer,
 * Lasse Lüder, Andre Radtke
 *
 * This software is licensed by MIT License.
 * See LICENSE for details.
 */
/**
 * @file
 * @author Moritz Höwer (Moritz.Hoewer@haw-hamburg.de)
 * \addtogroup util
 * @{
 */

#ifndef UTIL_CHRONO_LITERALS_H_
#define UTIL_CHRONO_LITERALS_H_

#include <chrono>

/**
 * literal for std::chrono::seconds
 */
inline std::chrono::seconds operator"" _s(unsigned long long l){
  return std::chrono::seconds(l);
}

/**
 * literal for std::chrono::milliseconds
 */
inline std::chrono::milliseconds operator"" _ms(unsigned long long l){
  return std::chrono::milliseconds(l);
}

/**
 * literal for std::chrono::microseconds
 */
inline std::chrono::microseconds operator"" _us(unsigned long long l){
  return std::chrono::microseconds(l);
}




#endif /* UTIL_CHRONO_LITERALS_H_ */
/** @} */
