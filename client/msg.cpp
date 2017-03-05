#include "server.h"
#include "link.h"
#include "msg.h"
#include "net.h"
#include <string>
#include "dataobj.pb.h"

rstatus_t req_recv(NetworkServer *proxy, Link *conn) {
	Buffer *msg = conn->msg_read();
	if (msg == NULL) {
		fprintf(stderr, "fd: %d, read failed, delete link", conn->fd());
		conn->mark_error();
		Fdevents *fdes = proxy->get_fdes();
		fdes->del(conn->fd());
		return CO_ERR;
	}

	fprintf(stderr, "req_recv: %s\n", msg->data());
	const std::string message = msg->data();

	// if we have not replied yet the node id reply it now if message is ping
	if (!conn->is_recorded) {
		char *data;
		if (message.find("ping") != std::string::npos) {
			data = "node:1";
			conn->is_recorded = true;
		}
		else {
			data = "pong";
		}
		// populate the response buffer queue
		Buffer *rsp = new Buffer(data, sizeof(data));
		conn->omsg_q.push_back(rsp);
		Fdevents *fdes = proxy->get_fdes();
		fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);
	} else {
		dataobj::Message d;
		d.ParseFromString(message);
		int index = d.ec_index();

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

