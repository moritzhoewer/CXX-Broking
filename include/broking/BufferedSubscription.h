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

#ifndef BROKING_BUFFEREDSUBSCRIPTION_H_
#define BROKING_BUFFEREDSUBSCRIPTION_H_

#include "broking/Subscription.h"
#include "broking/ThreadSafeQueue.h"

#include <memory>
#include <stdexcept>

namespace broking {

/**
 * Specialized Subscription that contains a buffer for incoming messages.
 *
 * @author  Moritz Höwer (Moritz.Hoewer@haw-hamubrg.de)
 * @version 1.0
 */
template<typename T> class BufferedSubscription: public Subscription {
	// Alias to make code shorter
	using QueuePtr = std::shared_ptr<ThreadSafeQueue<T>>;
private:
	QueuePtr queue; ///< the ThreadSafeQueue<T> buffering the incoming messages.
public:
	BufferedSubscription(Subscription&& subscription, QueuePtr queue);

	// Move only
    #if __GNUC__ <= 4
	/**
	 * Delete Copy Constructor
	 */
	BufferedSubscription(const BufferedSubscription&) : Subscription() {
	    throw std::logic_error("Copy of BufferedSubscription should really not happen...");
	}
    #else
	BufferedSubscription(const BufferedSubscription&) = delete;
    #endif
	/**
	 * Delete Copy assignment
	 */
	BufferedSubscription& operator=(const BufferedSubscription&) = delete;

	BufferedSubscription(BufferedSubscription&& subscription);
	BufferedSubscription& operator=(BufferedSubscription&& subscription);

	virtual ~BufferedSubscription();

	bool hasMessage();
	T getMessage();

	void setOnNewElement(std::function<void(void)> callback);
	void unsetOnNewElement();
};

/**
 * Move-constructor
 */
template<typename T>
inline BufferedSubscription<T>::BufferedSubscription(
		BufferedSubscription&& subscription) :
		Subscription(std::move(subscription)), queue(subscription.queue) {
	subscription.queue = nullptr;
}

/**
 * Move-Assignment
 */
template<typename T>
inline BufferedSubscription<T>& BufferedSubscription<T>::operator =(
		BufferedSubscription&& subscription) {
	// explicitly call move-assignment of Subscription
	this = static_cast<Subscription&&>(subscription);
	queue = subscription.queue;

	subscription.queue = nullptr;
	return *this;
}

/**
 * Constructs a BufferedSubscription<T>.
 *
 * @param subscription the existing Subscription to turn into a BufferedSubscription<T>
 * @param queue the ThreadSafeQueue<T> to wrap
 */
template<typename T>
inline BufferedSubscription<T>::BufferedSubscription(
		Subscription&& subscription, QueuePtr queue) :
		Subscription(std::move(subscription)), queue(queue) {
}

/**
 * @return true if a message can be retrieved
 */
template<typename T>
inline bool BufferedSubscription<T>::hasMessage() {
	if (!queue) {
		throw std::logic_error(
				"Invalid BuferedSubscription - did you move it?");
	}
	return queue->canDequeue();
}

/**
 * Destructs a BufferedSubscription.
 * Will unset the event callback.
 */
template<typename T>
inline BufferedSubscription<T>::~BufferedSubscription() {
	unsetOnNewElement();
}

/**
 * @return retrieve the message.
 * @attention this WILL block if there is no message to retrieve!
 */
template<typename T>
inline T BufferedSubscription<T>::getMessage() {
	if (!queue) {
		throw std::logic_error(
				"Invalid BuferedSubscription - did you move it?");
	}
	return queue->dequeue();
}

/**
 * Changes the event callback for availability of new elements.
 *
 * @param callback the new callback
 */
template<typename T>
inline void BufferedSubscription<T>::setOnNewElement(
		std::function<void(void)> callback) {
	if (!queue) {
		throw std::logic_error(
				"Invalid BuferedSubscription - did you move it?");
	}
	queue->setOnNewElement(callback);
}

/**
 * Removes the event callback for availability of new elements.
 */
template<typename T>
inline void BufferedSubscription<T>::unsetOnNewElement() {
	if (!queue) {
		// silently ignore
		return;
	}
	queue->unsetOnNewElement();
}

} /* namespace broking */

#endif /* BROKING_BUFFEREDSUBSCRIPTION_H_ */
/** @} */
