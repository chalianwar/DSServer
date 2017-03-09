#ifndef _NET_H_
#define _NET_H_

#include "server.h"
#include "link.h"
#include "fde.h"
#include <queue>
#include "dataobj.pb.h"
#include "util/str.h"

class Link;

typedef enum trace_operator{
	operator_read = 1,
  		operator_write,
  		operator_trim,
}trace_operator_t;

typedef enum request_type{
	need_flash_info = 1,
		not_need_flash_info,
		shut_cluster,
}request_type_t;

struct data_object{
	uint32_t          obj_no; //obj_no
	uint32_t          offset; //offset in object
	uint16_t          length;  //size in pages
    trace_operator_t  operator_t; //write/read
	float             timestamp;// start_time

	int32_t          ec_index;//for data parity or replica //reply to total recv pages;

	request_type_t    rq_type; // request type; need flash information back?
	float             flash_utilization;
	float             flash_victim_utilization;
	float             flash_full_blk_utilization;

	uint32_t          node_nr_erases;
	float             local_log_utilization;
	uint64_t			  request_number;
	uint32_t			  response_time;
};

bool operator<(const data_object& lhs, const data_object& rhs);

struct LessThanByReqNumber
{
  bool operator()(const data_object& lhs, const data_object& rhs) const
  {
    return lhs.request_number < rhs.request_number;
  }
};

class NetworkServer {
	public:
		NetworkServer();
		rstatus_t main_loop();
		Link *proxy_listen();
		Link *connect_to_server();
		Link *accept_conn();
		rstatus_t send(const Fdevent *fde);
		rstatus_t recv(const Fdevent *fde);
		Fdevents *get_fdes();
		rstatus_t proc_info(Link *link);
		std::priority_queue<data_object, std::vector<data_object>, LessThanByReqNumber> pq;
		rstatus_t send_data_obj (data_object test);
	private:
		Fdevents *fdes;
		Link *client_conn;
		Link *server_conn;
		int conn_count;
};

#endif
