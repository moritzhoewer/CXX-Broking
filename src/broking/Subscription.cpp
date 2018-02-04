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

#include "broking/Subscription.h"
#include "broking/AbstractChannelBase.h"

#define LOG_MODULE "broking"
#include "logging/logging.h"
#include <mutex>

namespace broking {

/**
 * Threadsafe incrementer to ensure that the ID for new Subscriptions is always
 * unique.
 *
 * @return a unique subscription ID
 */
static int getSubscriptionID() {
	// counter and mutex stay the same across invocations
	static int subscriptionCounter = 0;
	static std::mutex mtxSubsriptionID;

	std::lock_guard<std::mutex> lock(mtxSubsriptionID);
	return subscriptionCounter++;
}

/**
 * Constructs a subscription with a unique ID
 *
 * @param channel the Channel this is associated with
 */
Subscription::Subscription(AbstractChannelBase& channel, bool persistent) :
		id(getSubscriptionID()), channel(&channel), persistent(persistent) {
	LOG_TRACE<< "Constructing Subscription with ID " << id << std::endl;
}

/**
 * Constructs an invalid Subscription
 */
Subscription::Subscription() :
		id(-1), channel(nullptr), persistent(true) {
	LOG_TRACE<< "Constructing invalid Subscription..." << std::endl;
}

/**
 * Move Construction.
 * Will take responsibility for the Subscription away from other.
 *
 * @param other the Subscription to be transferred to this.
 */
Subscription::Subscription(Subscription&& other) :
		id(other.id), channel(other.channel), persistent(other.persistent) {
	other.id = -1;
	other.channel = nullptr;
	other.persistent = true;

	LOG_TRACE<< "Moving Subscription with id " << id << std::endl;
}

/**
 * Move assignment.
 * This will take responsibility for the Subscription away from other.
 *
 * @param other the Subscription to be transferred to this.
 * @return this
 */
Subscription& Subscription::operator =(Subscription&& other) {
	// In case we were subscribed, we need to unsubscribe first.
	if (channel && !persistent) {
		unsubscribe();
	}

	id = other.id;
	channel = other.channel;
	persistent = other.persistent;

	other.id = -1;
	other.channel = nullptr;
	other.persistent = true;

	LOG_TRACE<< "Moving Subscription with id " << id << std::endl;

	return *this;
}

/**
 * Destructs a Subscription.
 * Will unsubscribe it, if persistent flag is not set.
 */
Subscription::~Subscription() {
	if (!persistent) {
		unsubscribe();
	}
}

/**
 * Unsubscribe the Subscription from the Channel
 */
void Subscription::unsubscribe() {
	if (channel) {
		LOG_TRACE<< "Unsubscribing Subscription with id " << id << std::endl;

		channel->unsubscribe(*this);
		channel = nullptr;
		id = -1;
	}

}

/**
 * @return the ID
 */
int Subscription::getID() const {
	return id;
}

} /* namespace broking */
/** @} */
