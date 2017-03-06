#include "server.h"
#include "net.h"
#define SINGLE_SERVER false


int main(int argc, char **argv){

	if (SINGLE_SERVER) {
		NetworkServer *proxy = new NetworkServer();
		proxy->main_loop();
	}
	else {
		NetworkServer *proxies[MAX_NODES];
		for (int count = 0; count < MAX_NODES; count++) {
			proxies[count] = new NetworkServer("127.0.0.1", 5000 + count);
			proxies[count]->main_loop();
		}
	}

	return 0;
}

