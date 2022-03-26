#ifndef PROCESS_ENTROPY_H
#define PROCESS_ENTROPY_H

#include <vector>
#include "Entropy.h"

#define NUM_SUBINTERVALS 10
#define NUM_PORTS 5000
#define NUM_ENTROPY_PACKET_SIZES 1500
 #define MAX_TIME 12*3600 // in seconds
//#define MAX_TIME 500 // in seconds

extern int max_time;
#define Q entropy_arg

// default params
#define OUTPUT "output/"

#define NUM_SAMPLES MAX_TIME*NUM_SUBINTERVALS

extern int num_intervals;
extern int num_subintervals; // intervals per second
extern Entropy* entropy_factory;

struct Address {
	int src;
	int dst;
	uint64_t h;
};

struct Interval {
	int num_packets;
	int num_syn;
	int num_bytes;
	int num_df;
	
	std::vector<int> num_src_ports;
	std::vector<int> num_dst_ports;
	std::vector<int> num_src_ips;
	std::vector<int> num_dst_ips;
	std::vector<int> num_packet_sizes;
	
	// window
	double ent_pktnum;
	double ent_bytenum;
	double ent_srcport;
	double ent_dstport;
	double ent_srcip;
	double ent_dstip;
	double ent_flag_df;
	double ent_pkt_sizes;
	double ent_fsd;
	
	int dos_detection;
	int tot_pktnum;
	int tot_syn;
	
	// std::vector<std::pair<uint64_t, int>> fsd_traces;
	std::vector<std::pair<Address, int>> fsd_traces;
};
extern std::vector<Interval> intervals;

#endif
