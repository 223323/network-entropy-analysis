#include <stdio.h>
#include <math.h>
#include <pcap/pcap.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include <cstdlib>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "network_layers.h"
#include <sys/stat.h>

#include "Entropy.h"
#include <string>
#include <memory>

#include <functional>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "process_entropy.h"
#include "fsd.h"





// ----- data for processing ------
int num_intervals;
int num_subintervals = NUM_SUBINTERVALS; // intervals per second
int max_time = MAX_TIME;
int num_ports = NUM_PORTS;
double window_size_seconds = 1.0;
int g_total_packets = 0;

std::vector<Interval> intervals;
Entropy* entropy_factory = new ShannonEntropy(0);
Fsd* fsd = 0;

// -----------------

// cmdline
bool use_byte_entropy = false;
int verbose_sleep1 = 0;
int verbose_sleep2 = 0;
bool verbose = true;
float entropy_arg = 2.0;
//

int t_total_packets = 0;
int t_total_bytes = 0;
int t_total_attack_bytes = 0;

int parse_pcap(std::string filename);
int parse_ns2(std::string filename);
void print_result();
void process_entropy();

int threshold_min = 0;
int threshold = 0;


void init_vectors() {
	num_intervals = max_time * num_subintervals/window_size_seconds;
	intervals.resize(num_intervals+1);
	for(auto &s : intervals) {
		s.num_src_ports.resize(num_ports);
		s.num_dst_ports.resize(num_ports);
		s.num_src_ips.resize(num_ports);
		s.num_dst_ips.resize(num_ports);
		s.num_packet_sizes.resize(NUM_ENTROPY_PACKET_SIZES);
	}
}

std::vector<std::string> split(const std::string& s, char delimiter) {
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter)) {
      tokens.push_back(token);
   }
   return tokens;
}

std::vector<int> ips;
int server_is_dest = 1;

int main(int argc, char* argv[]) {
	if(argc < 2) {
		printf("%s <pcap/ns2 file>\n", argv[0]);
		return -1;
	}
	fsd = new Fsd2();
	std::string filename = "nopcap";
	for(int i=1; i < argc; i++) {
		std::string arg = std::string(argv[i]);
		
		if(arg == "-t") {
			threshold = atoi(argv[++i]);
		} else if(arg == "-tm") {
			threshold_min = atoi(argv[++i]);
		} else if(arg == "--renyi") {
			entropy_factory = new RenyiEntropy(0);
		} else if(arg == "--renyi2") {
			// entropy_factory = new Renyi2Entropy(0);
		} else if(arg == "--tsalis") {
			entropy_factory = new TsalisEntropy(0);
		} else if(arg == "--tsalis2") {
			entropy_factory = new Tsalis2Entropy(0);
		} else if(arg == "--bhatiasingh") {
			entropy_factory = new BhatiaSinghEntropy(0);
		} else if(arg == "--ubriaco") {
			entropy_factory = new UbriacoEntropy(0);
		} else if(arg == "--shannon") {
			entropy_factory = new ShannonEntropy(0);
		} else if(arg == "--byte-entropy") {
			use_byte_entropy = true;
		} else if(arg == "--num-ports") {
			num_ports = atoi(argv[++i]);
		} else if(arg == "--max-time" || arg == "--end-time") {
			max_time = atoi(argv[++i]);
			printf("set end-time: %d\n", max_time);
		} else if(arg == "--subintervals") {
			num_subintervals = atoi(argv[++i]);
			printf("set subintervals: %d\n", num_subintervals);
		} else if(arg == "--time-scale") {
			window_size_seconds = atof(argv[++i]);
		} else if(arg == "--entropy-q" || arg == "-Q") {
			entropy_arg = atof(argv[++i]);
		} else if(arg == "--no-verbose") {
			verbose = false;
		} else if(arg == "--fsd1") {
			delete fsd;
			fsd = new MyFsd();
		} else if(arg == "--src") {
			server_is_dest = 0;
		} else if(arg == "--server-ips") {
			std::string s_ips(argv[++i]);
			std::cout << "ips: " << s_ips << "\n";
			for(auto ip : split(s_ips, ';')) {
				ips.push_back(inet_addr(ip.c_str()));
			}
		} else if(arg == "-vs1") {
			verbose_sleep1 = atoi(argv[++i]);
		} else if(arg == "-vs2") {
			verbose_sleep2 = atoi(argv[++i]);
		} else if(arg[0] != '-') {
			filename = arg;
		}
	}
	
	init_vectors();

	struct stat s;
	if(stat("output", &s) != 0) {
		mkdir("output", ACCESSPERMS);
	}
	size_t ext = filename.rfind('.');
	if(ext != std::string::npos && filename.substr(ext) == ".ns2") {
		if(parse_ns2(filename) != 0) {
			printf("\nfailed to open file \"%s\"\n", filename.c_str());
			return -1;
		}
	} else {
		if(parse_pcap(filename) != 0) {
			printf("\nfailed to open file \"%s\"\n", filename.c_str());
			return -1;
		}
	}
	
	process_entropy();
	print_result();
	
	return 0;
}




// -----------------------------------


void process_entropy() {
	int i, j;
	printf("processing entropy\n");
	
	int total_packets = 0;
	int total_bytes = 0;
	int total_syn = 0;
	int total_df = 0;
	
	
	for(int i=0; i < num_subintervals; i++) {
		total_packets += intervals[i].num_packets;
		total_syn += intervals[i].num_syn;
		total_df += intervals[i].num_df;
		total_bytes += intervals[i].num_bytes;
	}
	entropy_factory->SetQ(Q);
	fsd->fsd_prepare();
	t_total_bytes = total_bytes;

	// calculate entropy for each window
	for (j=0; j < num_intervals - num_subintervals; j++) {
		const int j1 = j + num_subintervals;
		if(verbose) {
			std::cout << "processing " << j << " / " << num_intervals << "\n";
		}
		
		// entropy for each window
		std::unique_ptr<Entropy> flag_df_entropy(entropy_factory->New());
		std::unique_ptr<Entropy> pktnum_entropy(entropy_factory->New());
		std::unique_ptr<Entropy> bytenum_entropy(entropy_factory->New());
		std::unique_ptr<Entropy> srcport_entropy(entropy_factory->New());
		std::unique_ptr<Entropy> dstport_entropy(entropy_factory->New());
		std::unique_ptr<Entropy> srcip_entropy(entropy_factory->New());
		std::unique_ptr<Entropy> dstip_entropy(entropy_factory->New());
		std::unique_ptr<Entropy> packet_sizes_entropy(entropy_factory->New());

		for (i=0; i < num_subintervals; i++) {
			if (intervals[j+i].num_packets != 0) {
				pktnum_entropy->Add( intervals[j+i].num_packets / (double)total_packets );
				bytenum_entropy->Add( intervals[j+i].num_bytes / (double)total_bytes );
				flag_df_entropy->Add( intervals[j+i].num_df / (double)total_df );
			}
		}

		intervals[j].ent_pktnum = pktnum_entropy->GetValue();
		intervals[j].ent_bytenum = bytenum_entropy->GetValue();
		intervals[j].tot_pktnum = total_packets;
		intervals[j].tot_syn = total_syn;
		
		for (i=0; i < num_ports; i++) {
			int k, srcp=0, dstp=0;
			int srcip=0, dstip=0;
			for (k=0; k < num_subintervals; k++) {
				auto &interval = intervals[j+k];
				srcp += interval.num_src_ports[i];
				dstp += interval.num_dst_ports[i];
				srcip += interval.num_src_ips[i];
				dstip += interval.num_dst_ips[i];
			}

			double total_bytes2 = use_byte_entropy ? total_bytes : total_packets;
			if (srcp != 0) srcport_entropy->Add(srcp / total_bytes2);
			if(srcip != 0) srcip_entropy->Add(srcip / total_bytes2);
			if (dstp != 0) dstport_entropy->Add(dstp / total_bytes2);
			if(dstip != 0) dstip_entropy->Add(dstip / total_bytes2);
		}
		
		for (i=0; i < NUM_ENTROPY_PACKET_SIZES; i++) {
			int ps = 0;
			for (int k=0; k < num_subintervals; k++) {
				ps += intervals[j+k].num_packet_sizes[i];
			}

			if (ps != 0) packet_sizes_entropy->Add(ps / (double)total_packets);
		}
		
		srcport_entropy->SetCount(UINT16_MAX);
		srcip_entropy->SetCount(UINT32_MAX);
		dstport_entropy->SetCount(UINT16_MAX);
		dstip_entropy->SetCount(UINT32_MAX);
		packet_sizes_entropy->SetCount(NUM_ENTROPY_PACKET_SIZES);
		flag_df_entropy->SetCount(g_total_packets);
		
		
		intervals[j].ent_srcport = srcport_entropy->GetValue();
		intervals[j].ent_dstport = dstport_entropy->GetValue();
		intervals[j].ent_srcip = srcip_entropy->GetValue();
		intervals[j].ent_dstip = dstip_entropy->GetValue();
		intervals[j].ent_pkt_sizes = packet_sizes_entropy->GetValue();
		intervals[j].ent_flag_df = flag_df_entropy->GetValue();

		if(verbose) {
			
			printf("%3d, pn=%4d, S(pn)=%0.2lf%%, S(bn)=%0.2lf%%, S(sp)=%0.2lf%%, S(dp)=%0.2lf%% S(sip)=%0.2lf%%, S(dip)=%0.2lf%%\n",
					j,
					total_packets,
					pktnum_entropy->GetValue(),
					bytenum_entropy->GetValue(),
					srcport_entropy->GetValue(),
					dstport_entropy->GetValue(),
					srcip_entropy->GetValue(),
					dstip_entropy->GetValue()
			);
			
			if(verbose_sleep2 > 0) {
				usleep(verbose_sleep2 * 1000);
			}
			
		}
		
		// sliding window swap
		// for example:
		//	1 2 [ 3 4 5 ] 6 7
		//	1 2 3 [ 4 5 6 ] 7
		// remove 3, add 6
		total_packets += (intervals[j1].num_packets - intervals[j].num_packets);
		total_syn += (intervals[j1].num_syn - intervals[j].num_syn);
		total_bytes += (intervals[j1].num_bytes - intervals[j].num_bytes);
		total_df += (intervals[j1].num_df - intervals[j].num_df);
		t_total_bytes += intervals[j1].num_bytes;
		
		// fsd entropy
		fsd->fsd_update(j1);
	}
}

void save_result_double(const char* filename, std::function<double(int)> r, int num) {
	char file[50];
	strcpy(file, OUTPUT);
	strcat(file, filename);
	FILE* f = fopen(file, "wt");
	fprintf(f, "%0.6lf", r(0));
	int i;
	for (i=1; i < num; i++) {
		fprintf(f, ",%0.6lf", r(i));
	}
	fprintf(f, "\n");
	fclose(f);
}

void save_result_int(const char* filename, std::function<int(int)> r, int num) {
	char file[50];
	strcpy(file, OUTPUT);
	strcat(file, filename);
	FILE* f = fopen(file, "wt");
	fprintf(f, "%d", r(0));
	
	int i;
	for (i=1; i < num; i++) {
		fprintf(f, ",%d", r(i));
	}
	fprintf(f, "\n");
	fclose(f);
}

void print_result() {

	FILE* outf;
	int i, sec;
	int n = num_intervals;
	
	save_result_double("num_df.txt", [](int i) { return intervals[i].num_df / std::max(1, intervals[i].num_packets); }, n);
	save_result_double("ent_df.txt", [](int i) { return intervals[i].ent_flag_df; }, n);
	save_result_double("ent_pn.txt", [](int i) { return intervals[i].ent_pktnum; }, n);
	save_result_double("ent_bn.txt", [](int i) { return intervals[i].ent_bytenum; }, n);
	save_result_double("ent_sp.txt", [](int i) { return intervals[i].ent_srcport; } , n);
	save_result_double("ent_dp.txt", [](int i) { return intervals[i].ent_dstport; }, n);
	save_result_double("ent_sip.txt", [](int i) { return intervals[i].ent_srcip; }, n);
	save_result_double("ent_dip.txt", [](int i) { return intervals[i].ent_dstip; }, n);
	save_result_double("ent_pktsize.txt", [](int i) { return intervals[i].ent_pkt_sizes; }, n);
	save_result_double("ent_fsd.txt", [](int i) { return intervals[i].ent_fsd; }, n);
	save_result_int("tot_pn.txt", [](int i) { return intervals[i].tot_pktnum; }, n);
	save_result_int("tot_syn.txt", [](int i) { return intervals[i].tot_syn; }, n);

	printf("\ntotal bytes: %ld\n", t_total_bytes);
	printf("total packets: %ld\n", t_total_packets);
}

std::string ip2str(uint32_t ipv4) {
	struct in_addr addr;
	addr.s_addr = ipv4;
	return std::string(inet_ntoa(addr));
}

int parse_pcap(std::string filename) {
	double time;
	int i, j, sec, sub_int;
	int pkt_size;
	int src_addr=0, src_port=0;
	int dst_addr=0, dst_port=0;
	char errbuff[80];
	struct pcap_pkthdr pcap_hdr;
	
	printf("\nopening pcap: %s\n", filename.c_str());
	
	if( access( filename.c_str(), F_OK ) == -1 ) {
		return -1;
	}
	
	pcap_t* p = pcap_open_offline(filename.c_str(), errbuff);
	
	if(!p) {
		return -1;
	}
	
	printf("\nreading pcap file: %s\n", filename.c_str());
	
	const u_char *pkt_data;
	int sec_first = 1;
	double sec_offs = 0;
	int last_sec = 0;
	while (pkt_data = pcap_next(p, &pcap_hdr)) {
		tcp_packet* pkt = (tcp_packet*)pkt_data;
		ptp_tcp_packet* ptp_pkt = (ptp_tcp_packet*)pkt_data;
		pkt_size = pcap_hdr.len;
		
		tcp_header* tcp = &pkt->tcp;
		ip_header* ip = &pkt->ip;
		
		int pkt_type = pcap_datalink(p);
		if(pkt_type == DLT_LINUX_SLL) {
			sll_header* sll = (sll_header*)pkt_data;
			ip = (ip_header*)(pkt_data+sizeof(sll_header));
			tcp = (tcp_header*)(ip+1);
			if(sll->proto == PROTO_L3_IPv4) {
				if(ip->proto != PROTO_L4_TCP) {
					continue;
				}
			} else {
				continue;
			}
		} else {
			if(ptp_pkt->ptp.type == PROTO_PTP) {
				tcp = &ptp_pkt->tcp;
				ip = &ptp_pkt->ip;
			}
		}

		if(sec_first) {
			sec_first = 0;
			sec_offs = pcap_hdr.ts.tv_sec + pcap_hdr.ts.tv_usec * 1e-6 - 0.01;
		}

		time = (double)(pcap_hdr.ts.tv_sec + pcap_hdr.ts.tv_usec * 1e-6) - sec_offs;
		sec = (int)time;
		sub_int = (int)(fmod(time, 1.0)*num_subintervals);
		
		if(sec >= max_time) {
			return 0;
		}
		
		g_total_packets++;
		
		const int i = num_intervals * time / (double)max_time;
		// const int i = sec*num_subintervals+sub_int;
		auto& interval = intervals[i];
		
		bool has_syn = false;

		if(ip->proto == PROTO_L4_TCP && tcp->flags == tcp_flags::SYN) {
			has_syn = true;
			interval.num_syn++;
		}
		
		if(verbose) {
			// print IPs
			printf("%lf %d src ip: [%s:%d] dst ip: [%s:%d] [%s]\n", 
				time, 
				sub_int, 
				ip2str(ip->saddr.ip).c_str(), 
				ntohs(tcp->sport), 
				ip2str(ip->daddr.ip).c_str(), 
				ntohs(tcp->dport), 
				has_syn ? "SYN" : ""
			);
			if(verbose_sleep1 > 0) {
				usleep(verbose_sleep1 * 1000);
			}
		}

		// n_bytes, n_packets, n_pkts[sport], n_pkts[dport]
		interval.num_bytes += pkt_size;
		interval.num_packets++;
		
		if(verbose)
			std::cout << "packet size: " << pkt_size << " ";
		if(pkt_size < interval.num_packet_sizes.size()) {
			interval.num_packet_sizes[pkt_size]++;
		}
		
		if(ip->proto == PROTO_L4_TCP) {
			src_port = ntohs(tcp->sport) % num_ports;
			dst_port = ntohs(tcp->dport) % num_ports;
		}
		
		// if(ip->flags_fo & (1 << 14)) {
		if( ip->flags_fo & (1 << (7-1)) ) {
			if(verbose) {
				std::cout << "DF ";
			}
			interval.num_df += 1;
		} else {
			// std::cout << "NO dont fragment flag\n";
		}
		
		if(verbose)
		std::cout << "\n";
		
		src_addr = ip->saddr.ip;
		dst_addr = ip->daddr.ip;
		
		if(ips.size() > 0) {
			uint32_t match = server_is_dest ? dst_addr : src_addr;
			// std::cout << "test: " << match << " : " << ips[1] <<"\n";
			if(!std::any_of(ips.begin(), ips.end(), [&](int i){ return i == match; })) {
				// skip packet
				continue;
			}
		}
		
		src_addr = src_addr % num_ports;
		dst_addr = dst_addr % num_ports;
		int psize = use_byte_entropy ? pkt_size : 1;
		
		interval.num_src_ports[src_port] += psize;
		interval.num_dst_ports[dst_port] += psize;
		interval.num_src_ips[src_addr] += psize;
		interval.num_dst_ips[dst_addr] += psize;
		
		// fsd
		fsd->fsd_insert(src_addr, src_port, dst_addr, dst_port, 0, pkt_size, i);
		
		t_total_packets++;
	}
	printf("finished reading %d packets\n", t_total_packets);
	
	return 0;
}

int parse_ns2(std::string filename) {
	FILE *f;

	char line[100];
	char pkt_type[5];
	char event_type;

	double time;
	int i, j, sec, sub_int;
	int dummy = 0, tcp_hdr = 0;

	int src_id, dst_id;
	int pkt_size, flow_id;
	int pkt_id, attr;

	int src_addr, src_port;
	int dst_addr, dst_port;

	f = fopen(filename.c_str(), "rt");
	if (f == NULL) {
		printf("File not found!\n");
		return -0;
	}
// #define USE_NAM_TRACE
#if defined(USE_NAM_TRACE)
	while (fgets(line, 100, f) != NULL) {
		if (line[0] == '+')
			break;
	}
#endif
	int has_syn;
	line[0] = 0;
	do {
		if (line[0] == 'r') {
#if defined(USE_NAM_TRACE)
			//r -t 1.01032 -s 0 -d 1 -p tcp -e 40 -c 0 -i 0 -a 0 -x {0.0 1.0 0 ------- null}
			sscanf(line, "%c -t %lf -s %d -d %d -p %s -e %d -c %d -i %d -a %d -x {%d.%d %d.%d",
			       &event_type, &time, &src_id, &dst_id, pkt_type,
			       &pkt_size, &flow_id, &pkt_id, &attr,
			       &src_addr, &src_port, &dst_addr, &dst_port);
#else
			//r 0.01064 2 1 tcp 40 ------- 0 2.0 0.0 0 0 0 0x0 0 0
			sscanf(line, "%c %lf %d %d %s %d ------- %d %d.%d %d.%d %d %d %d 0x%x",
			       &event_type, &time, &src_id, &dst_id, pkt_type, &pkt_size, &flow_id,
			       &src_addr, &src_port, &dst_addr, &dst_port,
			       &attr, &pkt_id, &dummy, &tcp_hdr );
#endif

			if (dst_id == 0) { //1 for Stanislav's scripts, 0 for large scale web
				sec = (int)time;
				sub_int = (int)(fmod(time, 1.0)*num_subintervals);
				i = sec*num_subintervals+sub_int;
				auto &interval = intervals[i];
				
				interval.num_packets++;
				interval.num_bytes += pkt_size;
				
				if(pkt_size < interval.num_packet_sizes.size()) {
					interval.num_packet_sizes[pkt_size]++;
				}
		
				has_syn = 0;
				if (tcp_hdr == 0xa) {
					interval.num_syn++;
					has_syn = 1;
				}
				g_total_packets++;
				
				if(verbose) {
					// print IPs
					printf("%lf %d src ip: [%d:%d] dst ip: [%d:%d] [%s]\n", 
						time, 
						sub_int, 
						src_addr, 
						src_id, 
						dst_addr, 
						dst_id, 
						has_syn ? "SYN" : ""
					);
					if(verbose_sleep1 > 0) {
						usleep(verbose_sleep1 * 1000);
					}
				}
				
				/*
				if( ip->flags_fo & (1 << (7-1)) ) {
					if(verbose) {
						std::cout << "DF ";
					}
					interval.num_df += 1;
				} else {
					// std::cout << "NO dont fragment flag\n";
				}
				*/
				interval.num_df += 1;
				int psize = use_byte_entropy ? pkt_size : 1;
				interval.num_src_ports[src_addr] += psize;
				interval.num_dst_ports[dst_addr] += psize;
				interval.num_src_ips[src_addr] += psize;
				interval.num_dst_ips[dst_addr] += psize;
			
				// fsd
				fsd->fsd_insert(src_addr, 0, dst_addr, 0, 0, pkt_size, i);
				t_total_packets++;
			}
		}
	} while (fgets(line, 100, f) != NULL);
	fclose(f);
	
	return 0;
}


