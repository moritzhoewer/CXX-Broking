/*
 * Copyright © 2017-2018 Steven Beyermann, Markus Blechschmidt, Moritz Höwer,
 * Lasse Lüder, Andre Radtke
 *
 * This software is licensed by MIT License.
 * See LICENSE for details.
 */
/** 
 * @file
 * \addtogroup Broking
 * @{
 */

#ifndef BROKING_BROKING_H_
#define BROKING_BROKING_H_

#include "broking/Broker.h"

using namespace broking;

/**
 * Shortcut to getting a channel with type and ID.
 * Refer to Broker::getChannel<T>(std::string id) for details.
 */
#define GET_CHANNEL(type, id) \
    Broker::getBroker().getChannel<type>(id)


#endif /* INCLUDE_BROKING_BROKING_H_ */
/** @} */
