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

#ifndef UTIL_UTIL_H_
#define UTIL_UTIL_H_

#include <thread>

/**
 * Sleeps on the current thread
 */
#define SLEEP(x) \
    std::this_thread::sleep_for(x)

#endif /* UTIL_UTIL_H_ */
/** @} */
