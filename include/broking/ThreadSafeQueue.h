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

#ifndef BROKING_THREADSAFEQUEUE_H_
#define BROKING_THREADSAFEQUEUE_H_

#include "util/optional.hpp"

#include <deque>
#include <mutex>
#include <functional>
#include <condition_variable>

namespace broking {

/**
 * A thread safe implementation of a queue with blocking and nonblocking operations
 *
 * @author  Moritz Höwer (Moritz.Hoewer@haw-hamburg.de)
 * @version 1.0
 */
template<typename T> class ThreadSafeQueue {
private:
    std::mutex mtxAccess; ///< protects access to the queue
    std::condition_variable cvDequeue; ///< condition variable for dequeue
    std::condition_variable cvEnqueue; ///< condition variable for enqueue
    std::deque<T> queue; ///< the underlying STL container representing the queue
    int maxSize; ///< maximum size of the queue
    std::function<void(void)> notifyCallback; ///< gets called by enqueue
public:
    ThreadSafeQueue(int size);

    // prevent moving and copying
    /**
     * Delete Copy-Constructor
     */
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;

    /**
     * Delete Move-Constructor
     */
    ThreadSafeQueue(ThreadSafeQueue&&) = delete;

    /**
     * Delete Copy-Assignment
     */
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    /**
     * Delete Move-Assignment
     */
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

    bool canEnqueue();
    bool canDequeue();

    bool tryEnqueue(T message);
    void enqueue(T message);

    std::experimental::optional<T> tryDequeue();
    T dequeue();

    void setOnNewElement(std::function<void(void)> callback);
    void unsetOnNewElement();

private:
    bool canEnqueue_();
    bool canDequeue_();

    void enqueue_(T message);
    T dequeue_();

};

static auto DO_NOTHING_CALLBACK = [](){};

/**
 * Constructs a ThreadSafeQueue<T>.
 *
 * @param size the (maximum) size of the queue
 */
template<typename T>
inline ThreadSafeQueue<T>::ThreadSafeQueue(int size) :
        maxSize(size), notifyCallback(DO_NOTHING_CALLBACK) {
}

/**
 * Non-blocking enqueue.
 *
 * @param message the message to enqueue
 * @retval true successfully enqueued
 * @retval false unable to enqueue
 */
template<typename T>
inline bool ThreadSafeQueue<T>::tryEnqueue(T message) {
    std::lock_guard<std::mutex> lock(mtxAccess);
    if (canEnqueue_()) {
        enqueue_(message);
        return true;
    } else {
        return false;
    }
}

/**
 * Enqueue a message.
 * @attention will BLOCK if there is no space
 *
 * @param message the message to enqueue
 */
template<typename T>
inline void ThreadSafeQueue<T>::enqueue(T message) {
    std::unique_lock<std::mutex> lock(mtxAccess);
    while (!canEnqueue_()) {
        cvEnqueue.wait(lock);
    }
    enqueue_(message);
}

/**
 * Internal implementation of enqueue.
 * @pre caller must hold mtxAccess!
 * @param message
 */
template<typename T>
inline void ThreadSafeQueue<T>::enqueue_(T message) {
    queue.push_back(message);
    notifyCallback();
    cvDequeue.notify_one();
}

/**
 * Non-Blocking dequeue.
 * @return the message wrapped in an optional, or an empty optional if there is
 *         no message to dequeue
 */
template<typename T>
inline std::experimental::optional<T> ThreadSafeQueue<T>::tryDequeue() {
    std::lock_guard<std::mutex> lock(mtxAccess);
    if (canDequeue_()) {
        return {dequeue_()};
    } else {
        return std::experimental::nullopt;
    }
}

/**
 * Dequeue a message.
 * @attention will BLOCK if there is no message to dequeue
 *
 * @return the message
 */
template<typename T>
inline T ThreadSafeQueue<T>::dequeue() {
    std::unique_lock<std::mutex> lock(mtxAccess);
    while (!canDequeue_()) {
        cvDequeue.wait(lock);
    }
    return dequeue_();
}

/**
 * Changes the event callback for enqueueing new elements.
 *
 * @param callback the new callback
 */
template<typename T>
inline void ThreadSafeQueue<T>::setOnNewElement(
		std::function<void(void)> callback) {
	notifyCallback = callback;
}

/**
 * Removes the callback.
 */
template<typename T>
inline void ThreadSafeQueue<T>::unsetOnNewElement() {
	notifyCallback = DO_NOTHING_CALLBACK;
}

/**
 * Internal implementation of dequeue.
 * @pre caller must hold mtxAccess!
 * @return the message that was dequeued
 */
template<typename T>
inline T ThreadSafeQueue<T>::dequeue_() {
    auto result = queue.front();
    queue.pop_front();
    cvEnqueue.notify_one();

    return result;
}

/**
 * Checks if there is a message to be dequeued
 *
 * @retval true there is a message to be dequeued
 * @retval false there is no message to be dequeued
 */
template<typename T>
inline bool ThreadSafeQueue<T>::canDequeue() {
    std::lock_guard<std::mutex> lock(mtxAccess);
    return canDequeue_();
}

/**
 * Internal implementation of canDequeue.
 * @pre caller must hold mtxAccess!
 *
 * @retval true there is a message to be dequeued
 * @retval false there is no message to be dequeued
 */
template<typename T>
inline bool ThreadSafeQueue<T>::canDequeue_() {
    return !queue.empty();
}

/**
 * Checks if there is a space for a message
 *
 * @retval true there is space for a message to be queued
 * @retval false there is no space for a message to be queued
 */
template<typename T>
inline bool ThreadSafeQueue<T>::canEnqueue() {
    std::lock_guard<std::mutex> lock(mtxAccess);
    return canEnqueue_();
}

/**
 * Internal implementation of canEnqueue.
 * @pre caller must hold mtxAccess!
 *
 * @retval true there is space for a message to be queued
 * @retval false there is no space for a message to be queued
 */
template<typename T>
inline bool ThreadSafeQueue<T>::canEnqueue_() {
    return queue.size() < (unsigned) maxSize;
}

} /* namespace broking */

#endif /* BROKING_THREADSAFEQUEUE_H_ */
/** @} */
