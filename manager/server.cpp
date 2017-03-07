#include "server.h"
#include "net.h"

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

		while(1) {
		char *data;
			data_object test;
			test.ec_index = 23;
			test.obj_no = 23; //obj_no
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

			dataobj::Message d;
			d.set_ec_index(test.ec_index);
			d.set_obj_no(test.obj_no);
			d.set_offset(test.offset);
			d.set_length(test.length);
			d.set_operator_t(test.operator_t == dataobj::Message::operator_write ? dataobj::Message::operator_write :
					(test.operator_t == dataobj::Message::operator_read ? dataobj::Message::operator_read :
									dataobj::Message::operator_trim));
			d.set_timestamp(test.timestamp);
			d.set_flash_utilization(test.flash_utilization);
			d.set_flash_victim_utilization(test.flash_victim_utilization);
			d.set_flash_full_blk_utilization(test.flash_full_blk_utilization);
			d.set_rq_type(test.rq_type == dataobj::Message::need_flash_info ? dataobj::Message::need_flash_info :
					(test.rq_type == dataobj::Message::not_need_flash_info ? dataobj::Message::not_need_flash_info :
									dataobj::Message::shut_cluster));
			d.set_node_nr_erases(test.node_nr_erases);
			d.set_local_log_utilization(test.local_log_utilization);

			std::string dobj2;
			d.SerializeToString(&dobj2);
			char bts[dobj2.length()];
			strcpy(bts, dobj2.c_str());


			proxies[0]->convert_dataobj(test, data);
			proxies[0]->send_data_obj(data);
		}
	}


	return 0;
}

