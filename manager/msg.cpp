#include "server.h"
#include "link.h"
#include "msg.h"
#include "net.h"
#include <string>
#include <iostream>
#include <stdlib.h>


rstatus_t req_recv(NetworkServer *proxy, Link *conn) {
	std::shared_ptr<Buffer> msg = conn->msg_read();
	if (msg == NULL) {
		fprintf(stderr, "fd: %d, read failed, delete link", conn->fd());
		conn->mark_error();
		Fdevents *fdes = proxy->get_fdes();
		fdes->del(conn->fd());
		return CO_ERR;
	}

	std::string message = msg->data();
	fprintf(stderr, "req_recv: %s\n req_size: %d\n", msg->data(), msg->size());

	std::string delimiter = std::to_string(MAGIC);
	size_t pos = 0;
	std::string token;
	while ((pos = message.find(delimiter)) != std::string::npos) {
	    token = message.substr(0, pos);
	    fprintf(stderr, "sub messages: %s\n", token);
	    message.erase(0, pos + delimiter.length());
		dataobj::Response d;
		d.ParseFromString(token);
		std::string index = d.rsp();
		fprintf(stderr, "--------------- INDEX: %s\n", index);
	}


	//free(msg);
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
	fprintf(stderr, "rsp_send: %s\nrsp_size: %d\n", smsg, smsg->size());


//	std::string token = smsg->data();
//	dataobj::Message d;
//	d.ParseFromString(token);
//	int index = d.ec_index();
//	fprintf(stderr, "--------------- INDEX: %d\n", index);


	int len = conn->msg_write(smsg);
	if (len <= 0) {
		fprintf(stderr, "fd: %d, write: %d, delete link", conn->fd(), len);
		conn->mark_error();
		return CO_ERR;
	}

	return CO_OK;
}

