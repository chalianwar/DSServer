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

//	const std::vector<Bytes> messages = conn->msg_breakup(message);
//
//	for (std::vector<Bytes>::const_iterator i = messages.begin(); i != messages.end(); i++) {
//
//				dataobj::Message dmessage;
//				dmessage.ParseFromString(i->data());
//				int index = dmessage.ec_index();
//				fprintf(stderr, "--------------- INDEX: %d\n", index);
//
//				// populate the response buffer queue
//				// reply back with ack.
//				std::string dobj;
//				dataobj::Response drsp;
//				std::string dresponse= "ack";
//				drsp.set_rsp(dresponse);
//				drsp.SerializeToString(&dobj);
//				dobj.append(std::to_string(MAGIC).c_str());
//				char bts[dobj.length()];
//				strcpy(bts, dobj.c_str());
//				std::shared_ptr<Buffer> rsp = std::make_shared<Buffer>(sizeof(bts));
//				rsp->append(bts, sizeof(bts));
//				conn->omsg_q.push_back(rsp);
//				Fdevents *fdes = proxy->get_fdes();
//				fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);
//	}


	std::string delimiter = std::to_string(MAGIC);
	size_t pos = 0;
	std::string token;
	while ((pos = message.find(delimiter)) != std::string::npos) {
	    token = message.substr(0, pos);
	    fprintf(stderr, "sub messages: %s - sub message size %d\n", token, token.size());
		dataobj::Message dmessage;
		dmessage.ParseFromString(token);
		int index = dmessage.ec_index();
		fprintf(stderr, "INDEX: %d\n", index);
		message.erase(0, pos + delimiter.length());

		std::string dobj;
		dataobj::Response drsp;
		std::string dresponse= "ack";
		drsp.set_rsp(dresponse);
		drsp.SerializeToString(&dobj);
		dobj.append(std::to_string(global_req_number));
		dobj.append(std::to_string(MAGIC).c_str());
		char bts[dobj.length()];
		strcpy(bts, dobj.c_str());

		// populate the response buffer queue
		//std::shared_ptr<Buffer> rsp = std::make_shared<Buffer>(bts, sizeof(bts));
		std::shared_ptr<Buffer> rsp = std::make_shared<Buffer>(sizeof(bts));
		rsp->append(bts, sizeof(bts));
		conn->omsg_q.push_back(rsp);
		Fdevents *fdes = proxy->get_fdes();
		fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);
		global_req_number++;
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

	//fprintf(stderr, "rsp_send: %s\n", smsg->data());
	int len = conn->msg_write(smsg);
	if (len <= 0) {
		fprintf(stderr, "fd: %d, write: %d, delete link", conn->fd(), len);
		conn->mark_error();
		return CO_ERR;
	}

	return CO_OK;
}

