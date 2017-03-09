#include "server.h"
#include "link.h"
#include "msg.h"
#include "net.h"
#include <string>
#include <iostream>
#include <stdlib.h>


//rstatus_t req_recv(NetworkServer *proxy, Link *conn) {
//	std::shared_ptr<Buffer> msg = conn->msg_read();
//	if (msg == NULL) {
//		fprintf(stderr, "fd: %d, read failed, delete link", conn->fd());
//		conn->mark_error();
//		Fdevents *fdes = proxy->get_fdes();
//		fdes->del(conn->fd());
//		return CO_ERR;
//	}
//
//	std::string message = msg->data();
//	fprintf(stderr, "req_recv: %s\n req_size: %d\n", msg->data(), msg->size());
//
//	std::string delimiter = std::to_string(MAGIC);
//	size_t pos = 0;
//	std::string token;
//	while ((pos = message.find(delimiter)) != std::string::npos) {
//	    token = message.substr(0, pos);
//	    fprintf(stderr, "sub messages: %s\n", token);
//	    message.erase(0, pos + delimiter.length());
//		dataobj::Message d;
//		d.ParseFromString(token);
//		int req_num = d.request_number();
//		fprintf(stderr, "Response: %d\n", req_num);
//	}
//
//	return CO_OK;
//}

rstatus_t req_recv(NetworkServer *proxy, Link *conn) {
	std::shared_ptr<Buffer> msg = conn->msg_read();
	if (msg == NULL) {
		fprintf(stderr, "fd: %d, read failed, delete link", conn->fd());
		conn->mark_error();
		Fdevents *fdes = proxy->get_fdes();
		fdes->del(conn->fd());
		return CO_ERR;
	}

	//fprintf(stderr, "req_recv: %s\n req_size: %d\n", msg->data(), msg->size());
	char *ptr = msg->data();
	std::string abcd (msg->data(), msg->size());
	int parsed = 0;

	while(parsed < msg->size()) {

			uint32_t *magic;
			magic = (uint32_t *)ptr;
			if(*magic != MAGIC) {
				fprintf(stderr, "MAGIC NUMBER MISMATCH: %d\n", *magic);
				while(parsed < msg->size()) {
				ptr += 1;
				parsed += 1;
				uint32_t *magic;
				magic = (uint32_t *)ptr;
				if(*magic != MAGIC)
					ptr += sizeof(uint32_t);
					parsed += sizeof(uint32_t);
					continue;
				}
				break;
			}
			ptr += sizeof(uint32_t);
			parsed += sizeof(uint32_t);

			size_t *sz  = (size_t *)ptr;
			size_t totalSize = *sz;
			ptr += sizeof(size_t);
			parsed += sizeof(size_t);

			std::string token(ptr, totalSize);
			dataobj::Message d;
			d.ParseFromString(token);

			data_object dobj = convert_to_dobj(d);
			fprintf(stderr, "--------------- Reply Number: %d\n", dobj.request_number);
			ptr += totalSize;
			parsed += totalSize;

			if (parsed == msg->size()) {
				fprintf(stderr, "=====DONE====\n");
			}
		}

	return CO_OK;
}

rstatus_t rsp_send(NetworkServer *proxy, Link *conn) {

	// pop the response buffer queue
	// send response back
	std::shared_ptr<Buffer> smsg;
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
	//fprintf(stderr, "rsp_send: %s\nrsp_size: %d\n", smsg, smsg->size());

	int len = conn->msg_write(smsg);
	if (len <= 0) {
		fprintf(stderr, "fd: %d, write: %d, delete link", conn->fd(), len);
		conn->mark_error();
		return CO_ERR;
	}

	return CO_OK;
}

data_object convert_to_dobj (dataobj::Message d) {
	data_object rcv;
	rcv.ec_index = d.ec_index();
	rcv.obj_no = d.obj_no(); //obj_no
	rcv.offset = d.offset(); //offset in object
	rcv.length = d.length();  //size in pages
	rcv.operator_t = (d.operator_t() == dataobj::Message::operator_write ? trace_operator::operator_write :
			(d.operator_t() == dataobj::Message::operator_read ? trace_operator::operator_read :
					trace_operator::operator_trim));
	rcv.timestamp = d.timestamp();// start_time
	rcv.rq_type = (d.rq_type() == dataobj::Message::need_flash_info ? request_type_t::need_flash_info :
				(d.rq_type() == dataobj::Message::not_need_flash_info ? request_type_t::not_need_flash_info :
								request_type_t::shut_cluster));
	rcv.flash_utilization = d.flash_utilization();
	rcv.flash_victim_utilization = d.flash_victim_utilization();
	rcv.flash_full_blk_utilization = d.flash_full_blk_utilization();
	rcv.node_nr_erases = d.node_nr_erases();
	rcv.local_log_utilization = d.local_log_utilization();
	rcv.request_number = d.request_number();
	return rcv;
}

