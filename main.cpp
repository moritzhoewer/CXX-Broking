#include "broking/broking.h"
#include "util/util.h"
#include "util/chrono_literals.h"

#include <iostream>

int main() {
	auto& ch = GET_CHANNEL(int, "test");
	
	ch.subscribe([](int i){std::cout << "Received " << i << std::endl;}, true);
	
	ch.publish(1);
	ch.publish(42);
	
	SLEEP(100_ms);
}