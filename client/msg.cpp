#include "server.h"
#include "link.h"
#include "msg.h"
#include "net.h"
#include <string>
#include "dataobj.pb.h"
const uint32_t MAGIC = 0x06121983;

int global_req_number = 0;

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> v;
    auto i = 0;
    auto pos = s.find(delim);
    while (pos != std::string::npos) {
      v.push_back(s.substr(i, pos-i));
      i = ++pos;
      pos = s.find(delim, pos);

      if (pos == std::string::npos)
         v.push_back(s.substr(i, s.length()));
    }
}

rstatus_t req_recv(NetworkServer *proxy, Link *conn) {
	std::shared_ptr<Buffer> msg = conn->msg_read();
	if (msg == NULL) {
		fprintf(stderr, "fd: %d, read failed, delete link", conn->fd());
		conn->mark_error();
		Fdevents *fdes = proxy->get_fdes();
		fdes->del(conn->fd());
		return CO_ERR;
	}

	fprintf(stderr, "req_recv: %s\n req_size: %d\n", msg->data(), msg->size());
	const std::string message = msg->data();

	std::string delimiter = std::to_string(MAGIC);
//	std::vector<std::string> v  = split(message, delimiter.c_str());
//
//	for (std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); i++) {
//		std::string a = *i;
//		dataobj::Message d;
//		d.ParseFromString(a);
//		int index = d.ec_index();
//		fprintf(stderr, "--------------- INDEX: %d\n", index);
//
//	}


//	while (msg->size()) {
//			char *curr_ptr = msg->data();
//			conn->msg_parse(msg);
//			int req_sz = msg->size_diff();
//			Buffer *req = new Buffer(req_sz);
//			req->append(curr_ptr, req_sz);
//			conn->clear_recv_data();
//
//			size_t pos = 0;
//			std::string token;
//		    fprintf(stderr, "sub messages: %s\n sub message size %d\n", token, token.size());
//			dataobj::Message d;
//			d.ParseFromString(token);
//			int index = d.ec_index();
//			fprintf(stderr, "--------------- INDEX: %d\n", index);
//
//			std::string dobj;
//			dataobj::Response drsp;
//			std::string dresponse= "pong";
//			drsp.set_rsp(dresponse);
//			drsp.SerializeToString(&dobj);
//			dobj.append(std::to_string(MAGIC).c_str());
//			char bts[dobj.length()];
//			strcpy(bts, dobj.c_str());
//
//	//		//dresponse.append(std::to_string(global_req_number).c_str());
//	//		char bts[dresponse.length()];
//	//		strcpy(bts, dresponse.c_str());
//
//			// populate the response buffer queue
//			Buffer *rsp = new Buffer(bts, sizeof(bts));
//			conn->omsg_q.push_back(rsp);
//			Fdevents *fdes = proxy->get_fdes();
//			fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);
//	}



	size_t pos = 0;
	std::string token;
	while ((pos = message.find(delimiter)) != std::string::npos) {
	    token = message.substr(0, pos);
	    fprintf(stderr, "sub messages: %s\n sub message size %d\n", token, token.size());
		dataobj::Message d;
		d.ParseFromString(token);
		int index = d.ec_index();
		fprintf(stderr, "--------------- INDEX: %d\n", index);
		message.erase(0, pos + delimiter.length());

		std::string dobj;
		dataobj::Response drsp;
		std::string dresponse= "pong";
		drsp.set_rsp(dresponse);
		drsp.SerializeToString(&dobj);
		dobj.append(std::to_string(MAGIC).c_str());
		char bts[dobj.length()];
		strcpy(bts, dobj.c_str());

//		//dresponse.append(std::to_string(global_req_number).c_str());
//		char bts[dresponse.length()];
//		strcpy(bts, dresponse.c_str());

		// populate the response buffer queue
		//std::shared_ptr<Buffer> rsp = std::make_shared<Buffer>(bts, sizeof(bts));
		std::shared_ptr<Buffer> rsp = std::make_shared<Buffer>(sizeof(bts));
		rsp->append(bts, sizeof(bts));
		conn->omsg_q.push_back(rsp);
		Fdevents *fdes = proxy->get_fdes();
		fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);

	}
	//fprintf(stderr, "sub messages: %s\n", message);

//	// if we have not replied yet the node id reply it now if message is ping
//	if (!conn->is_recorded) {
//		char *data;
//		if (message.find("ping") != std::string::npos) {
//			data = "node:1";
//			conn->is_recorded = true;
//			// populate the response buffer queue
//			Buffer *rsp = new Buffer(data, sizeof(data));
//			conn->omsg_q.push_back(rsp);
//			Fdevents *fdes = proxy->get_fdes();
//			fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);
//		}
//		else {
//		}
//		conn->is_recorded = true;
//	}

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

	fprintf(stderr, "rsp_send: %s\n", smsg->data());
	int len = conn->msg_write(smsg);
	if (len <= 0) {
		fprintf(stderr, "fd: %d, write: %d, delete link", conn->fd(), len);
		conn->mark_error();
		return CO_ERR;
	}

	return CO_OK;
}

