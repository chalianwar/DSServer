
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
	server_conn->is_recorded = false;
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
		events = fdes->wait(50); // yue: so far timeout hardcoded
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

