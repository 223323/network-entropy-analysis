#ifndef NETWORK_LAYERS_H
#define NETWORK_LAYERS_H

#include <pcap/pcap.h>
#include <arpa/inet.h>
// #include "devices.h"

#define UDP_MAX_PACKET_DATA_SIZE 1500
#define TCP_MAX_PACKET_DATA_SIZE UDP_MAX_PACKET_DATA_SIZE

/* 4 bytes IP address */
typedef struct t_ip_address
{
	union {
		u_char bytes[4];
		u_int32_t ip;
	};
} ip_address;

typedef struct t_mac_address
{
	u_char bytes[6];
} __attribute__((packed)) mac_address;


/* IPv4 header */

typedef struct t_ip_header
{
	u_char	ver_ihl;		// Version (4 bits) + Internet header length (4 bits)
	u_char	tos;			// Type of service 
	u_short tlen;			// Total length 
	u_short identification; // Identification
	u_short flags_fo;		// Flags (3 bits) + Fragment offset (13 bits)
	u_char	ttl;			// Time to live
	u_char	proto;			// Protocol
	u_short crc;			// Header checksum
	ip_address	saddr;		// Source address
	ip_address	daddr;		// Destination address
	// u_int	op_pad;			// Option + Padding
} ip_header;

/* UDP header*/
typedef struct udp_header
{
	u_short sport;			// Source port
	u_short dport;			// Destination port
	u_short len;			// Datagram length
	u_short crc;			// Checksum
} __attribute__((packed)) udp_header;

enum tcp_flags
{
	NONE = 0, 
	FIN  = 1, 
	SYN  = 2, 
	RST  = 4, 
	PSH  = 8, 
	ACK  = 16,
	URG  = 32,
	ECE  = 64,
	CWR  = 128
};
	
typedef struct tcp_header {
	
	uint16_t sport;
	uint16_t dport;
	uint32_t seq_num;
	uint32_t ack_num;
	uint8_t data_offset_4msb;
	uint8_t flags;
	uint16_t window_size;
	uint16_t crc;
	uint16_t urgent_pointer;
	
	/* Options (if data offset > 5. Padded at the end with "0" bytes if necessary.) */
} __attribute__((packed)) tcp_header;

#define PROTO_L3_IPv4 8
#define PROTO_L4_TCP 6
#define PROTO_PTP 0x2100

typedef struct eth_header {
	mac_address dmac;
	mac_address smac;
	u_short proto_type;
} __attribute__((packed)) eth_header;

typedef struct t_packet {
	eth_header eth;
	ip_header ip;
} __attribute__((packed)) Packet;


#define IP_ADDR(a) (ip_address){ .ip = inet_addr(#a) } //str2ip(#a)
#define str2ip(a) (ip_address){ .ip = inet_addr(a) }

typedef struct udp_packet {
	eth_header eth;
	ip_header ip;
	udp_header udp;
	u_char data[UDP_MAX_PACKET_DATA_SIZE];
} __attribute__((packed)) udp_packet;

typedef struct tcp_packet {
	eth_header eth;
	ip_header ip;
	tcp_header tcp;
	u_char data[TCP_MAX_PACKET_DATA_SIZE];
} __attribute__((packed)) tcp_packet;

typedef struct ptp_header {
	uint16_t type;
} __attribute__((packed)) ptp_header;

typedef struct ptp_tcp_packet {
	ptp_header ptp;
	ip_header ip;
	tcp_header tcp;
	u_char data[TCP_MAX_PACKET_DATA_SIZE];
} __attribute__((packed)) ptp_tcp_packet;


#endif
