#include "server.h"
#include "link.h"
#include "msg.h"
#include "net.h"
#include <string>
#include "dataobj.pb.h"
const uint32_t MAGIC = 0x06121983;

int global_req_number = 0;

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
	char *ptr = msg->data();
	std::string abcd (msg->data(), msg->size());
	int parsed = 0;

	while(parsed < msg->size()) {
		size_t *sz  = (size_t *)ptr;
		size_t totalSize = *sz;
		ptr += sizeof(size_t);
		parsed += sizeof(size_t);

		std::string token(ptr, totalSize);
		dataobj::Message d;
		d.ParseFromString(token);
		int index = d.ec_index();
		fprintf(stderr, "--------------- INDEX: %d\n", index);
		ptr += totalSize;
		parsed += totalSize;
	}


//	std::string delimiter = std::to_string(MAGIC);
//	if(message.find(delimiter))
//		exit(-1);
//
//	size_t pos = 0;
//	std::string token;
//	while ((pos = message.find(delimiter)) != std::string::npos) {
//	    token = message.substr(0, pos);
//	    fprintf(stderr, "sub messages: %s - sub message size %d\n", token, token.size());
//		dataobj::Message dmessage;
//		dmessage.ParseFromString(token);
//		int req_num = dmessage.request_number();
//		fprintf(stderr, "Request number: %d\n", req_num);
//		message.erase(0, pos + delimiter.length());
//
//		dmessage.set_response_time(12345);
//
//		std::string dobj;
//		dmessage.SerializeToString(&dobj);
//		dobj.append(std::to_string(MAGIC).c_str());
//		char bts[dobj.length()];
//		strcpy(bts, dobj.c_str());
//
//		// populate the response buffer queue
//		//std::shared_ptr<Buffer> rsp = std::make_shared<Buffer>(bts, sizeof(bts));
//		std::shared_ptr<Buffer> rsp = std::make_shared<Buffer>(sizeof(bts));
//		rsp->append(bts, sizeof(bts));
//		conn->omsg_q.push_back(rsp);
//		Fdevents *fdes = proxy->get_fdes();
//		fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);
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

	//fprintf(stderr, "rsp_send: %s\n", smsg->data());
	int len = conn->msg_write(smsg);
	if (len <= 0) {
		fprintf(stderr, "fd: %d, write: %d, delete link", conn->fd(), len);
		conn->mark_error();
		return CO_ERR;
	}

	return CO_OK;
}

