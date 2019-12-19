#include <cstdint>
#include <memory>
#include "fsd.h"
#include "process_entropy.h"

static uint64_t calc_hash(int sip, int sport, int dip, int dport, int proto) {
	return ((((sip*33 + sport)*33 + dip)*33 + dport)*33 + proto);
}
static uint32_t calc_hash2(int sip, int sport, int proto) {
	return ((sip*33 + sport)*33 + proto) * 33;
}


std::list<std::pair<uint64_t, Window>> recent_fsd;
std::set<uint64_t> all_fsd;

void Fsd::fsd_update_recent(int i) {
	for(auto& s : intervals[i].fsd_traces) {
		bool found=false;
		for(auto r = recent_fsd.begin(); r != recent_fsd.end(); ) {
			if(r->first == s.first.h) {
				found = true;
				int a = std::max(0,r->second.last_seen-num_subintervals);
				int b = std::min(r->second.last_seen, i-num_subintervals);
				for(int j = a; j < b; j++) {
					for(auto& s1 : intervals[j].fsd_traces) {
						if(s1.first.h == r->first) {
							r->second.value -= s1.second;
							break;
						}
					}
				}
				r->second.last_seen = i;
				r->second.value += s.second;
				break;
			} else {
				if(r->second.last_seen+num_subintervals < i) {
					r = recent_fsd.erase(r);
				} else {
					++r;
				}
			}
		}
		if(!found) {
			recent_fsd.push_back( { s.first.h, {.last_seen=i, .value=s.second} } );
		}
	}
}

// #include <fstream>
// std::ofstream out_fsd("output-fsd.bin", std::ios::out);
// std::ofstream out_entropy("output-entropy.bin", std::ios::out);
void MyFsd::fsd_update(int i) {
	int j = i - num_subintervals;
	if(j >= 0) {
		std::unique_ptr<Entropy> fsd_entropy(entropy_factory->New());
		double fsd_total = 0;
		for(auto &s : recent_fsd) {
			fsd_total += s.second.value;
		}
		for(auto &s : recent_fsd) {
			// out_fsd << (s.second.value / fsd_total) << ", " << "\n";
			fsd_entropy->Add( s.second.value / fsd_total );
		}
		fsd_entropy->SetCount(all_fsd.size());
		intervals[j].ent_fsd = fsd_entropy->GetValue();
		// out_entropy << fsd_entropy->GetValue() << ", " << "\n";
	}
	Fsd::fsd_update_recent(i);
}

void Fsd::fsd_prepare() {
	for(int i=0; i < num_subintervals; i++) {
		fsd_update_recent(i);
	}
}

void Fsd::fsd_insert(int src_addr, int src_port, int dst_addr, int dst_port, int type, int pkt_size, int interval_idx) {
	uint64_t h = calc_hash(src_addr, src_port, dst_addr, dst_port, type);
	uint32_t src = calc_hash2(src_addr, src_port, type) % 1000;
	uint32_t dst = calc_hash2(dst_addr, dst_port, type) % 1000;
	bool found = false;
	for(auto& tr : intervals[interval_idx].fsd_traces) {
		if(tr.first.h == h) {
			tr.second += pkt_size;
			found = true;
			break;
		}
	}
	if (!found) {
		Address a;
		a.src = src;
		a.dst = dst;
		a.h = h;
		intervals[interval_idx].fsd_traces.emplace_back(a, pkt_size);
	}
	all_fsd.insert(h);
}

// void fsd_done() {
	// out_fsd.close();
	// out_entropy.close();
// }


/*
// 1. determine flows
flows[src_addr][dst_addr] = 1;
flows[dst_addr][src_addr] = 1;
*/


// 2. assign flow_id
/*
for (i=0; i<NUM_PORTS-1; i++)
  {
    for (j=i+1; j<NUM_PORTS; j++)
    {
      if (flows[i][j] == 1)
      {
        flows[i][j] = num_flows;
        flows[j][i] = num_flows;
        num_flows++;
      }
      else
      {
        flows[i][j] = -1;
        flows[j][i] = -1;
      }
    }
  }
*/
#include <iostream>
void Fsd2::fsd_prepare() {
	for(auto &interval : intervals) {
		for(auto &e : interval.fsd_traces) {
			auto &i = e.first;
			flows[i.src][i.dst] = 1;
			flows[i.dst][i.src] = 1;
		}
	}
	num_flows = 0;

	// assign flow id
	int i,j;
	for (i=0; i<NUM_ADDR-1; i++) {
		for (j=i+1; j<NUM_ADDR; j++) {
			if (flows[i][j] == 1) {
				flows[i][j] = num_flows;
				flows[j][i] = num_flows;
				num_flows++;
			} else {
				flows[i][j] = -1;
				flows[j][i] = -1;
			}
		}
	}

	// 3. flow bytes
	for(int t=0; t < intervals.size(); t++) {
		for(auto& e : intervals[t].fsd_traces) {
			auto &i = e.first;
			flow_bytes[t][flows[i.src][i.dst]] += 1;
		}
	}

	// 4. flow_size, flow_counter
	for (j=0; j<NUM_SUBINTERVALS; j++) {
		for (i=0; i<num_flows; i++) {
			flow_size[i] += flow_bytes[j][i];
			flow_size_sub[i] += flow_bytes[j][i];
			if(flow_bytes[j][i]>0) {
				flow_counter[j]++;
			}
		}
	}
}

/*
// 3. flow bytes
//byte entropy
        //flow_bytes[sec*NUM_SUBINTERVALS+sub_int][flows[src_addr][dst_addr]] += pkt_size;
//packet entropy
		flow_bytes[sec*NUM_SUBINTERVALS+sub_int][flows[src_addr][dst_addr]] += 1;

// 4. flow_size, flow_counter
for (j=0; j<NUM_SUBINTERVALS; j++)
  {
    for (i=0; i<num_flows; i++)
    {
      flow_size[i] += flow_bytes[j][i];
      flow_size_sub[i] += flow_bytes[j][i];
	  if(flow_bytes[j][i]>0)
		  flow_counter[j]++;

    }
  }

// 5. entropy
while (j < max_time)
  {
    double flowsize_entropy = 0;
    int num_bytes = 0;

    for (i=0; i<num_flows; i++)
    {
	  if(flow_bytes[j][i]>0)
		  flow_counter[j-NUM_SUBINTERVALS]++;//IB

	  if (flow_size_sub[i] != 0)
        num_bytes += flow_size[i];
    }
    for (i=0; i<num_flows; i++)
    {
      if (flow_size_sub[i] != 0)
      {
		double p = flow_size[i] / (double)num_bytes;
        ent_flowsize[j-NUM_SUBINTERVALS] += pow(p,Q);
       }
		flow_size[i] += flow_bytes[j][i];
      flow_size_sub[i] += flow_bytes[j][i] - flow_bytes[j-NUM_SUBINTERVALS][i];
	}
}
*/

void Fsd2::fsd_update(int j) {
	
	double flowsize_entropy = 0;
	int num_bytes = 0;
	int i;
	for (i=0; i < num_flows; i++) {
		if(flow_bytes[j][i]>0) {
			flow_counter[j-NUM_SUBINTERVALS]++;//IB
		}

		if (flow_size_sub[i] != 0) {
			num_bytes += flow_size[i];
		}
	}

	std::unique_ptr<Entropy> fsd_entropy(entropy_factory->New());
	for (i=0; i < num_flows; i++) {
		if (flow_size_sub[i] != 0) {
			double p = flow_size[i] / (double)num_bytes;
			fsd_entropy->Add( p );
		}
		flow_size[i] += flow_bytes[j][i];
		flow_size_sub[i] += flow_bytes[j][i] - flow_bytes[j-NUM_SUBINTERVALS][i];
		fsd_entropy->SetCount(num_flows);
		intervals[j-num_subintervals].ent_fsd = fsd_entropy->GetValue();
	}
}
