/*
Copyright (c) 2012-2014 The SSDB Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
*/
#ifndef NET_LINK_H_
#define NET_LINK_H_

#include <deque>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "net.h"
#include "server.h"
#include "link.h"
#include "fde.h"
#include "util/bytes.h"

class Link;
class NetworkServer;
class Buffer;

typedef rstatus_t (*link_recv_t)(NetworkServer *, Link *);
typedef rstatus_t (*link_send_t)(NetworkServer *, Link *);
typedef void (*link_msgq_t)(Link *, std::shared_ptr<Buffer> *);

class Link{
	private:
		int sock;
		bool noblock_;
		bool error_;
		std::vector<Bytes> recv_data;


		static int min_recv_buf;
		static int min_send_buf;
	public:
		const static int MAX_PACKET_SIZE = 128 * 1024 * 1024;

		char remote_ip[INET_ADDRSTRLEN];
		int remote_port;

		bool auth;
		bool ignore_key_range;

		std::shared_ptr<Buffer> input;
		std::shared_ptr<Buffer> output;

		std::deque<std::shared_ptr<Buffer>> imsg_q;
		std::deque<std::shared_ptr<Buffer>> omsg_q;

		double create_time;
		double active_time;

		Link();
		~Link();
		void close();
		void nodelay(bool enable=true);
		// noblock(true) is supposed to corperate with IO Multiplex,
		// otherwise, flush() may cause a lot unneccessary write calls.
		void noblock(bool enable=true);
		void keepalive(bool enable=true);
		void set_nw_func(); // yue

		int fd() const{
			return sock;
		}
		bool error() const{
			return error_;
		}
		void mark_error(){
			error_ = true;
		}

		static Link* connect(const char *ip, int port);
		static Link* listen(const char *ip, int port);
		Link* accept();

		// read network data info buffer
		int read();
		std::shared_ptr<Buffer> msg_read();  // yue
		int write();
		int msg_write(std::shared_ptr<Buffer> smsg);  // yue
		// flush buffered data to network
		// REQUIRES: nonblock
		int flush();

		// yue: the following are func pointers registered for different uses
		link_recv_t recv_nw;
		link_send_t send_nw;

		/**
		 * parse received data, and return -
		 * NULL: error
		 * empty vector: recv not ready
		 * vector<Bytes>: recv ready
		 */
		const std::vector<Bytes>* parse();  // yue: originally named recv()
		// wait until a response received.
		const std::vector<Bytes>* response();

		// need to call flush to ensure all data has flush into network
		int send(const std::vector<std::string> &packet);
		int send(const std::vector<Bytes> &packet);
		int send(const Bytes &s1);
		int send(const Bytes &s1, const Bytes &s2);
		int send(const Bytes &s1, const Bytes &s2, const Bytes &s3);
		int send(const Bytes &s1, const Bytes &s2, const Bytes &s3, const Bytes &s4);
		int send(const Bytes &s1, const Bytes &s2, const Bytes &s3, const Bytes &s4, const Bytes &s5);

		const std::vector<Bytes>* last_recv(){
			return &recv_data;
		}
		
		/** these methods will send a request to the server, and wait until a response received.
		 * @return
		 * NULL: error
		 * vector<Bytes>: response ready
		 */
		const std::vector<Bytes>* request(const Bytes &s1);
		const std::vector<Bytes>* request(const Bytes &s1, const Bytes &s2);
		const std::vector<Bytes>* request(const Bytes &s1, const Bytes &s2, const Bytes &s3);
		const std::vector<Bytes>* request(const Bytes &s1, const Bytes &s2, const Bytes &s3, const Bytes &s4);
		const std::vector<Bytes>* request(const Bytes &s1, const Bytes &s2, const Bytes &s3, const Bytes &s4, const Bytes &s5);

		bool conn_recorded;
};

#endif
