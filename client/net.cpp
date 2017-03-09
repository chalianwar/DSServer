
#include "server.h"
#include "net.h"
#include "resp.h"

NetworkServer::NetworkServer(void) {
	fdes = new Fdevents();

	server_conn = Link::connect("127.0.0.1", 5000);
	if(server_conn == NULL){
		fprintf(stderr, "error opening server socket! %s\n", strerror(errno));
		exit(1);
	}

	server_conn->set_nw_func();
	server_conn->conn_recorded = false;
}

Link *NetworkServer::accept_conn() {
	Link *conn = client_conn->accept();
	if (conn == NULL) {
		fprintf(stderr, "accept failed: %s", strerror(errno));
		return NULL;
	}

	conn->nodelay();
	conn->noblock();
	conn->create_time = millitime();
	conn->active_time = conn->create_time;

	conn->set_nw_func();

	return conn;
}

int NetworkServer::main_loop(void) {

	const Fdevents::events_t *events;

	//set client connections to be accepted
	fdes->set(server_conn->fd(), FDEVENT_IN, 1, server_conn);

	while (true) {
		events = fdes->wait(50000); // yue: so far timeout hardcoded
		if (events == NULL) {
			return CO_ERR;
		}
		for (int i = 0; i < (int)events->size(); i++) {
			const Fdevent *fde = events->at(i);
		if (fde->events & FDEVENT_IN) { // ali: where we read data from client
			recv(fde);
		} else if (fde->events & FDEVENT_OUT) { // ali: where we send data to client
			send(fde);
		}
		}
	}

	return CO_OK;
}

/*
 * ali: wrapper calling registered funcs
 */
rstatus_t NetworkServer::recv(const Fdevent *fde) {
	Link *conn = (Link *)fde->data.ptr;
	if (conn->error())	return CO_ERR;

	rstatus_t ret = conn->recv_nw(this, conn);
	if (ret != CO_OK) {
		fprintf(stderr, "recv on %d failed: %s", conn->fd(), strerror(errno));
	}

	return ret;
}

/*
 * ali: wrapper calling registered funcs
 */
rstatus_t NetworkServer::send(const Fdevent *fde) {
	Link *conn = (Link *)fde->data.ptr;
	if (conn->error())	return CO_ERR;

	rstatus_t ret = conn->send_nw(this, conn);
	if (ret != CO_OK) {
		fprintf(stderr, "send on %d failed: %s", conn->fd(), strerror(errno));
	}

	return ret;
}

Fdevents *NetworkServer::get_fdes() {
	return fdes;
}

bool operator<(const data_object& lhs, const data_object& rhs)
{
  return lhs.request_number < rhs.request_number;
}


//Send the data to the node
rstatus_t NetworkServer::send_data_obj (data_object test) {

	dataobj::Message dmessage;
	dmessage.set_ec_index(test.ec_index);
	dmessage.set_obj_no(test.obj_no);
	dmessage.set_offset(test.offset);
	dmessage.set_length(test.length);
	dmessage.set_operator_t(test.operator_t == dataobj::Message::operator_write ? dataobj::Message::operator_write :
			(test.operator_t == dataobj::Message::operator_read ? dataobj::Message::operator_read :
							dataobj::Message::operator_trim));
	dmessage.set_timestamp(test.timestamp);
	dmessage.set_flash_utilization(test.flash_utilization);
	dmessage.set_flash_victim_utilization(test.flash_victim_utilization);
	dmessage.set_flash_full_blk_utilization(test.flash_full_blk_utilization);
	dmessage.set_rq_type(test.rq_type == dataobj::Message::need_flash_info ? dataobj::Message::need_flash_info :
			(test.rq_type == dataobj::Message::not_need_flash_info ? dataobj::Message::not_need_flash_info :
							dataobj::Message::shut_cluster));
	dmessage.set_node_nr_erases(test.node_nr_erases);
	dmessage.set_local_log_utilization(test.local_log_utilization);
	dmessage.set_request_number(test.request_number);

	std::string dobj = dmessage.SerializeAsString();

	String *req = new String(8);
	req->append(MAGIC);
	req->append(dobj.size());
	req->append(dobj.c_str(), dobj.size());
	std::vector<char> v(req->data(), req->data() + req->size());
	char* bts = &v[0];

	//fprintf(stderr, "req: %s\nreq_size: %d\n", bts, sizeof(bts));
	std::shared_ptr<Buffer> rsp = std::make_shared<Buffer>(bts);
	rsp->append(bts, v.size());

	char *ptr = rsp->data();
	ptr += sizeof(uint32_t);
	size_t *sz  = (size_t *)ptr;
	size_t totalSize = *sz;
	ptr += sizeof(size_t);

	std::string token(ptr, totalSize);
	dataobj::Message d;
	d.ParseFromString(token);
	int index = d.ec_index();
	//fprintf(stderr, "--------------- INDEX: %d\n", index);

	//fprintf(stderr, "req: %s\nreq_size: %d\n", rsp->data(), rsp->size());
	server_conn->omsg_q.push_back(rsp);
	Fdevents *fdes = get_fdes();
	fdes->set(server_conn->fd(), FDEVENT_OUT, 1, server_conn);
	delete req;
}
