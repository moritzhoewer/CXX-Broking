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

#ifndef BROKING_SUBSCRIPTION_H_
#define BROKING_SUBSCRIPTION_H_

namespace broking {

// Forward declare
class AbstractChannelBase;

/**
 * Represents a Subscription to a Channel.
 *
 * @author  Moritz Höwer (Moritz.Hoewer@haw-hamburg.de)
 * @version 1.0
 */
class Subscription {
private:
	int id; ///< the unique id for the subscription
	AbstractChannelBase *channel; ///< Channel that we are subscribed to
	 bool persistent; ///< controls auto-unsubscribe in destructor
public:
	Subscription();
	Subscription(AbstractChannelBase& channel, bool persistent);

	// Class is move only
	/**
	 * Delete Copy Constructor
	 */
	Subscription(const Subscription&) = delete;
	/**
	 * Delete Copy assignment
	 */
	Subscription& operator=(const Subscription&) = delete;


	Subscription(Subscription&& other);
	Subscription& operator=(Subscription&& other);

	virtual ~Subscription();

	int getID() const;

	void unsubscribe();
};

} /* namespace broking */

#endif /* BROKING_SUBSCRIPTION_H_ */
/** @} */
