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
		// no other reason for connection to be not set
		conn->conn_recorded = true;
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

