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

#ifndef BROKING_ABSTRACTCHANNELBASE_H_
#define BROKING_ABSTRACTCHANNELBASE_H_

namespace broking {

// Forward declare
class Subscription;

/**
 * Abstract Base class to unify all Channels.
 *
 * @author Moritz Höwer (Moritz.Hoewer@haw-hamburg.de)
 * @version 1.0
 */
class AbstractChannelBase {
public:

    /**
     * Virtual Destructor
     */
    virtual ~AbstractChannelBase() {
    }

    virtual void unsubscribe(const Subscription& subscription) = 0;
};

} // namespace broking


#endif /* BROKING_ABSTRACTCHANNELBASE_H_ */
/** @} */
