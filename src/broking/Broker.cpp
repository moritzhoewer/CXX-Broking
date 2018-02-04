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

#include "broking/Broker.h"

namespace broking {

/**
 * Name for the WARNING_CHANNEL
 */
static constexpr auto WARNING_CHANNEL_NAME = "InternalChannel.Warnings";

/**
 * Meyers-Singleton
 *
 * @return the Broker
 */
Broker& Broker::getBroker() {
    static Broker instance;
    return instance;
}

/**
 * Kinda hacky "singleton" - creates the WARNING_CHANNEL for infrormation about
 * non critical messages (Severity::WARNING) that are dropped.
 */
Channel<std::string> WARNING_CHANNEL(WARNING_CHANNEL_NAME);


} /* namespace broking */
/** @} */
