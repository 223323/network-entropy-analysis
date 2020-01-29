#ifndef FSD_H
#define FSD_H
#include <list>
#include <set>
#include "process_entropy.h"

struct Window {
	int last_seen;
	int value;
};

class Fsd {	
	public:
	std::list<std::pair<uint64_t, Window>> recent_fsd;
	std::set<uint64_t> all_fsd;
	virtual void fsd_prepare();
	virtual void fsd_update(int i) {}
	virtual void fsd_insert(int src_addr, int src_port, int dst_addr, int dst_port, int type, int pkt_size, int interval_idx);
	void fsd_update_recent(int i);
};

class MyFsd : public Fsd {
	public:
	void fsd_update(int i);
};

#define NUM_ADDR 1000
#define NUM_FLOWS 4000
class Fsd2 : public Fsd {
	public:
	int flows[NUM_ADDR][NUM_ADDR];
	int flow_bytes[NUM_SAMPLES][NUM_FLOWS];
	// std::unique_ptr<Entropy> ent_flowsize[NUM_SAMPLES];
	int num_flows;
	
	int flow_size[NUM_FLOWS];
	int flow_size_sub[NUM_FLOWS];
	int flow_counter[NUM_SAMPLES];
	
	void fsd_update(int i);
	void fsd_prepare();
};

#endif
