#include "server.h"
#include "link.h"
#include "msg.h"
#include "net.h"
#include <string>
#include <iostream>
#include <stdlib.h>


rstatus_t req_recv(NetworkServer *proxy, Link *conn) {
	Buffer *msg = conn->msg_read();
	if (msg == NULL) {
		fprintf(stderr, "fd: %d, read failed, delete link", conn->fd());
		conn->mark_error();
		Fdevents *fdes = proxy->get_fdes();
		fdes->del(conn->fd());
		return CO_ERR;
	}

	std::string message = msg->data();
	fprintf(stderr, "req_recv: %s\n", msg->data());

	// find out if we have recorded this connection already or not
	// if not then send ping and expect "node:id" in reply
	if (!conn->conn_recorded) {

		// extract id from the reply
		if (message.find("node") != std::string::npos) {
			int node_reply = std::stoi( message.substr(message.find(":") + 1).c_str());
			fprintf(stderr, "Node: %d\n",std::stoi( message.substr(message.find(":") + 1).c_str() ));
			proxy->nodes[node_reply] = conn;
			conn->conn_recorded = true;
			//return CO_OK;
		}

//		// If not a reply ping the SSD
//		// populate the response buffer queue
//		char *data = "ping";
//		Buffer *rsp = new Buffer(data, sizeof(data));
//		conn->omsg_q.push_back(rsp);
//		Fdevents *fdes = proxy->get_fdes();
//		fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);
//	}
//	else {
//		data_object test;
//		test.ec_index = 23;
//		test.obj_no = 23; //obj_no
//		test.offset = 23; //offset in object
//		test.length = 23;  //size in pages
//	    test.operator_t = operator_write; //write/read
//		test.timestamp = 123.455;// start_time
//
//		test.rq_type =  not_need_flash_info; // request type; need flash information back?
//		test.flash_utilization = 123.456;
//		test.flash_victim_utilization = 123.456;
//		test.flash_full_blk_utilization = 123.456;
//
//		test.node_nr_erases = 23;
//		test.local_log_utilization = 123.456;
//
//		//std:string data = str()
//		//std::string test_str = std::to_string(test);

//		dataobj::Message d;
//		d.set_ec_index(23);
//		d.set_obj_no(23);
//		d.set_offset(23);
//		d.set_length(23);
//		d.set_operator_t(dataobj::Message::operator_write);
//		d.set_timestamp(123.45);
//		d.set_flash_utilization(123);
//		d.set_flash_victim_utilization(1232131);
//		d.set_flash_full_blk_utilization(123213);
//		d.set_rq_type(dataobj::Message::not_need_flash_info);
//		d.set_node_nr_erases(123);
//		d.set_local_log_utilization(123123);
//		std::string dobj;
//		d.SerializeToString(&dobj);
//		char bts[dobj.length()];
//		strcpy(bts, dobj.c_str());
//
//		Buffer *rsp = new Buffer(bts, sizeof(bts));
//		conn->omsg_q.push_back(rsp);
//		Fdevents *fdes = proxy->get_fdes();
//		fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);

	}

	return CO_OK;
}

rstatus_t rsp_send(NetworkServer *proxy, Link *conn) {

	// pop the response buffer queue
	// send response back
	Buffer *smsg;
	if (!conn->omsg_q.empty()) {
	smsg = conn->omsg_q.front(); conn->omsg_q.pop_front();
	}
	else{
		Fdevents *fdes = proxy->get_fdes();
		fdes->clr(conn->fd(), FDEVENT_OUT);  // yue: nothing to send so del ev
		return CO_OK;  // yue: nothing to send
	}
	if (smsg == NULL) {
		return CO_OK;  // yue: nothing to send
	}
	int len = conn->msg_write(smsg);
	if (len <= 0) {
		fprintf(stderr, "fd: %d, write: %d, delete link", conn->fd(), len);
		conn->mark_error();
		return CO_ERR;
	}

	return CO_OK;
}

