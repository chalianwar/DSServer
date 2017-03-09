#include "server.h"
#include "net.h"

void wait_till_all_connected (NetworkServer *proxies[]) {
again:
	for (int i = 0; i < MAX_NODES; i++) {
		if (!proxies[i]->is_connected)
			goto again;
	}
}

int main(int argc, char **argv){

	if (SINGLE_SERVER) {
		NetworkServer *proxy = new NetworkServer();
		proxy->main_loop();
	}
	else {
		NetworkServer *proxies[MAX_NODES];
		for (int count = 0; count < MAX_NODES; count++) {
			proxies[count] = new NetworkServer("127.0.0.1", 5000 + count);
			proxies[count]->start_main();
		}

		wait_till_all_connected(proxies);

		for (int k = 0; k <100; k++) {
		data_object test;
		test.ec_index = k * k;
		test.obj_no = 23 +  k * k; //obj_no
		test.offset = 23; //offset in object
		test.length = 23;  //size in pages
		test.operator_t = operator_write; //write/read
		test.timestamp = 123.455;// start_time
		test.rq_type =  not_need_flash_info; // request type; need flash information back?
		test.flash_utilization = 123.456;
		test.flash_victim_utilization = 123.456;
		test.flash_full_blk_utilization = 123.456;
		test.node_nr_erases = 23;
		test.local_log_utilization = 123.456;
		proxies[0]->send_data_obj(test);
		}
	}

	while(1);

	return 0;
}

