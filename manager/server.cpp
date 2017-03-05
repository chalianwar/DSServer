#include "server.h"
#include "net.h"
#include "dataobj.pb.h"

int main(int argc, char **argv){

	NetworkServer *proxy = new NetworkServer();
	proxy->main_loop();

	dataobj::Message d;
	d.set_ec_index(23);
	d.set_obj_no(23);
	d.set_offset(23);
	d.set_length(23);
	d.set_operator_t(dataobj::Message::operator_write);
	d.set_timestamp(123.45);
	d.set_flash_utilization(123);
	d.set_flash_victim_utilization(1232131);
	d.set_flash_full_blk_utilization(123213);
	d.set_rq_type(dataobj::Message::not_need_flash_info);
	d.set_node_nr_erases(123);
	d.set_local_log_utilization(123123);
	std::string dobj;
	d.SerializeToString(&dobj);
	char bts[dobj.length()];
	strcpy(bts, dobj.c_str());

//	Buffer *rsp = new Buffer(bts, sizeof(bts));
//	conn->omsg_q.push_back(rsp);
//	Fdevents *fdes = proxy->get_fdes();
//	fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);

	return 0;
}

