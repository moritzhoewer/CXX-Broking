# Broking
Short usage guide and example for the Broking Module

## Usage
To use logging framework, include `util/broking/broking.h`.  

## Getting a channel
A channel can be obtained by calling `GET_CHANNEL(type, "name")`. The macro calls `Broker::getBroker().getChannel<type>("name")` which returns a reference to a  `Channel<type>`.

**Warning**:  
Calling `GET_CHANNEL` with an existing ID but with a different type compared to the type it was created with will throw a `std::logic_error` because of incompatible types.

## Publishing to a channel
You can publish to a channel using the `publish` function. The message is buffered to be delivered to all subscribers later. If there are no subscribers, the message is dropped.  
If the publish buffer is full, the function will block until there is space again.  


An optional second parameter to `publish` specifies a `Severity` (default: `Severity::ERROR`). If a message with `Severity::ERROR` can not be passed to a Subscriber, the programm terminates with an exception. If a message with `Severity::WARNING` can not be passed to a Subscriber, execution continues but an information about the loss is sent to the `WARNING_CHANNEL`.


## Subscribing a channel
There are two ways to subscribe to a channel.

### Subscribing a callback (synchronous)
If handling the message is a **short!** operation, a callback can be subscribed to the channel by passing it to `subscribe` - the signature of the callback is `void(T)` where `T` is the type of the Channel.  
The call returns a `Subscription` which can be used to unsubscribe from the channel later.


The callback will be called by the channel with each new message that is published.

### Subscribing with a buffer (asynchronous)
If handling the message takes long or might be delayed, a buffer can be subscribed to the channel by calling `subscribe` without parameters.  
The call retruns a `BufferedSubscription<T>` where `T` is the type of the channel, which provides access to the buffer and can also be used to unsubscribe from the channel later.

If a specific size is required for the buffer, it can be passed to `subscribe` as a parameter.

When a message is published, it will be copied to the buffer and can be accessed by calling `getMessage()` on the `BufferedSubscription<T>`.  
**Warning**:  
this call will block, if there is no message in the buffer - `hasMessage` can be used beforehand to check if there is a message

## Example
```
#include "broking/broking.h"
#include "util/util.h"
#include "util/chrono_literals.h"
#include <iostream>

int main() {
	// Create two channels
	Channel<int>& intCh = GET_CHANNEL(int, "intCh");
	auto& doubleCh = GET_CHANNEL(double, "doubleCh");

	// get Channel<double> as Channel<int> will fail
	//auto& fail = GET_CHANNEL(int, "doubleCh");

	// Subscribe callback to integer channel
	Subscription intCallback = intCh.subscribe([](int i){
		std::cout << "Int Callback received " << i << std::endl;
	});

	// Subscribe callback to WARNING_CHANNEL and ignore return value
	WARNING_CHANNEL.subscribe([](std::string message){
		LOG_WARNING << message << std::endl;
	});

	//Subscribe buffer with default size to int channel
	BufferedSubscription<int> intBuffer1 = intCh.subscribe();

	// Subscribe buffer with size 2 to int channel
	auto intBuffer2 = intCh.subscribe(2);

	// Publish some messages
	intCh.publish(1);

	// Wait, because of asynchronous transmit
	SLEEP(100_ms);

	// unsubscribe int callback
	intCh.unsubscribe(intCallback);

	// publish another message
	intCh.publish(2);

	// Wait, because of asynchronous transmit
	SLEEP(100_ms);

	// Read the buffer of intBuffer1
	while(intBuffer1.hasMessage()){
		std::cout << "Int Buffer1 has " << intBuffer1.getMessage() << std::endl;
	}

	// intBuffer2 is full!
	intCh.publish(3, Severity::WARNING);
	intCh.publish(4);

	SLEEP(100_ms);
}
```
will output
```
Int Callback received 1
Int Buffer1 has 1
Int Buffer1 has 2
[2017-11-03 18:10:13][WARNING][../src/main.cpp:21 (operator())]: Dropped a message on Channel "intCh" - Subscriber 3 didn't accept...
[2017-11-03 18:10:13][ ERROR ][/media/sf_Programmieren/Uni/ESEP-WS17/include/broking/Channel.h:158 (processingLoop)]: Dropped critical Message on Channel "intCh" - Subscriber 3 didn't accept!
terminate called after throwing an instance of 'std::runtime_error'
  what():  Dropped critical message on Channel "intCh"

```
