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

#ifndef BROKING_BROKER_H_
#define BROKING_BROKER_H_

#include "broking/Channel.h"
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>

namespace broking {

/**
 * Broker in charge of handling the messages.
 *
 * @author  Moritz Höwer (Moritz.Hoewer@haw-hamburg.de)
 * @version 1.0
 */
class Broker {
private:
    std::mutex mtxChannelAccess; ///< mutex to protect access to the channels
    std::map<std::string, std::unique_ptr<AbstractChannelBase>> channels; ///< stores the channels

public:
    static Broker& getBroker(); // Singleton

private:
    /**
     * Default Constructor
     */
    Broker() = default; // Singleton

public:
    // Prevent moving and copying

    /**
     * Delete Copy-Constructor
     */
    Broker(const Broker&) = delete;

    /**
     * Delete Move-Constructor
     */
    Broker(Broker&&) = delete;

    /**
     * Delete Copy-Assignment
     */
    Broker& operator=(const Broker&) = delete;

    /**
     * Delete Move-Assignment
     */
    Broker& operator=(Broker&&) = delete;

    /**
     * Default Destructor.
     */
    virtual ~Broker() = default;

    template<typename T> Channel<T>& getChannel(std::string id);
};

/**
 * Get a reference to a channel with a specific ID.
 * The first call creates the channel, all subsequent calls return that same channel.
 *
 * @param id the ID of the Channel
 *
 * @return reference to the Channel<T> that corresponds to the ID
 *
 * @throws std::logic_error if requesting a channel that was created with another T
 */
template<typename T>
inline Channel<T>& Broker::getChannel(std::string id) {
    // Thread safety
    std::lock_guard<std::mutex> lock(mtxChannelAccess);

    auto result = channels.find(id);
    if (result == channels.end()) {
        auto channel = std::unique_ptr<AbstractChannelBase>(new Channel<T>(id));
        channels[id] = std::move(channel);
    }

    // cast the stored AbstratChannelBase* to a usable Channel<T>*
    Channel<T>* pointer = dynamic_cast<Channel<T>*>(channels[id].get());
    if (!pointer) {
        // dynamic_cast return nullptr if casting doesn't work.
        throw std::logic_error(
                "Failed to cast - Please ensure that the types match!!");
    }
    return *pointer;
}

} /* namespace broking */

#endif /* BROKING_BROKER_H_ */
/** @} */
