
#include "server.h"
#include "net.h"
#include "resp.h"
#include "dataobj.pb.h"

NetworkServer::NetworkServer(void) {
	fdes = new Fdevents();

	client_conn = Link::listen("127.0.0.1", 12345);
	if(client_conn == NULL){
		fprintf(stderr, "error opening server socket! %s\n", strerror(errno));
		exit(1);
	}
	conn_count = 0;
	client_conn->conn_recorded = false;
}

NetworkServer::NetworkServer(const std::string ip, uint32_t port) {

	fdes = new Fdevents();

	client_conn = Link::listen(ip.c_str(), port);
	if(client_conn == NULL){
		fprintf(stderr, "error opening server socket! %s\n", strerror(errno));
		exit(1);
	}
	conn_count = 0;
	client_conn->conn_recorded = false;

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
	fdes->set(client_conn->fd(), FDEVENT_IN, 0, client_conn);

	while (true) {
		events = fdes->wait(50); // yue: so far timeout hardcoded
		if (events == NULL) {
			return CO_ERR;
		}
		for (int i = 0; i < (int)events->size(); i++) {
			const Fdevent *fde = events->at(i);
			if (fde->data.ptr == client_conn) {
				Link *conn = accept_conn();
				if (conn) {
					conn_count++;
					remote_conn = conn;
					fdes->set(conn->fd(), FDEVENT_IN, 1, conn);

					// ping to get the node id and track it
					if (SINGLE_SERVER) {
						// perform function
						// populate the response buffer queue
						char *data = "ping";
						Buffer *rsp = new Buffer(data, sizeof(data));
						conn->omsg_q.push_back(rsp);
						Fdevents *fdes = get_fdes();

						fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);
					}
				}
			} else if (fde->events & FDEVENT_IN) { // ali: where we read data from client
				recv(fde);
			} else if (fde->events & FDEVENT_OUT) { // ali: where we send data to client
				send(fde);
			}
		}
	}

	return CO_OK;
}

//Send the data to the node
rstatus_t NetworkServer::send_data_obj (char bts[]) {

		Buffer *rsp = new Buffer(bts, sizeof(bts));
		remote_conn->omsg_q.push_back(rsp);
		Fdevents *fdes = get_fdes();
		fdes->set(remote_conn->fd(), FDEVENT_OUT, 1, remote_conn);
}


//Send the data to the node - we map each connection as single serving all SSDs
rstatus_t NetworkServer::send_data_obj_single_server (char bts[], uint32_t node_id) {

		Buffer *rsp = new Buffer(bts, sizeof(bts));
		nodes[node_id]->omsg_q.push_back(rsp);
		Fdevents *fdes = get_fdes();
		fdes->set(nodes[node_id]->fd(), FDEVENT_OUT, 1, nodes[node_id]);
}

//Convert the data_object to transferable char *
void convert_dataobj(data_object dobj, char* outStr) {

	dataobj::Message d;
	d.set_ec_index(dobj.ec_index);
	d.set_obj_no(dobj.obj_no);
	d.set_offset(dobj.offset);
	d.set_length(dobj.length);
	d.set_operator_t(dobj.operator_t == dataobj::Message::operator_write ? dataobj::Message::operator_write :
			(dobj.operator_t == dataobj::Message::operator_read ? dataobj::Message::operator_read :
							dataobj::Message::operator_trim));
	d.set_timestamp(dobj.timestamp);
	d.set_flash_utilization(dobj.flash_utilization);
	d.set_flash_victim_utilization(dobj.flash_victim_utilization);
	d.set_flash_full_blk_utilization(dobj.flash_full_blk_utilization);
	d.set_rq_type(dobj.rq_type == dataobj::Message::need_flash_info ? dataobj::Message::need_flash_info :
			(dobj.rq_type == dataobj::Message::not_need_flash_info ? dataobj::Message::not_need_flash_info :
							dataobj::Message::shut_cluster));
	d.set_node_nr_erases(dobj.node_nr_erases);
	d.set_local_log_utilization(dobj.local_log_utilization);

	std::string dobj2;
	d.SerializeToString(&dobj2);
	char bts[dobj2.length()];
	strcpy(bts, dobj2.c_str());
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

