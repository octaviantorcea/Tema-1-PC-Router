#include <queue.h>
#include "skel.h"
#include "rtable.h"
#include "arp_table.h"

#define BROADCAST_ADDRESS "ff:ff:ff:ff:ff:ff"

int main(int argc, char *argv[])
{
	packet m;
	int rc;

	init(argc - 2, argv + 2);

	struct rtable *rtable = parse_rtable(argv[1]);
	queue packet_queue = queue_create();
	struct arp_table *arp_table = new_arp_table();

	while (1) {
		rc = get_packet(&m);
		DIE(rc < 0, "get_message");

		struct ether_header *eth_hdr = (struct ether_header *)m.payload;
		struct arp_header *arp_hdr = parse_arp(m.payload);
		struct icmphdr *icmp_hdr = parse_icmp(m.payload);

		// if it's an ICMP packet
		if (icmp_hdr != NULL) {
			struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));

			// if it's an ICMP_ECHO and the ip address matches the router's ip address send an ICMP_ECHOREPLY
			if (icmp_hdr->type == ICMP_ECHO && ip_hdr->daddr == inet_addr(get_interface_ip(m.interface))) {
				send_icmp(ip_hdr->saddr, ip_hdr->daddr, eth_hdr->ether_dhost, eth_hdr->ether_shost,
						ICMP_ECHOREPLY, 0, m.interface, icmp_hdr->un.echo.id, icmp_hdr->un.echo.sequence);

				continue;
			}
		}

		// if it's an ARP packet
		if (arp_hdr != NULL) {
			// if it's an ARP_REQUEST and the ip address matches the router's ip address send an ARP_REPLY
			if (ntohs(arp_hdr->op) == ARPOP_REQUEST && arp_hdr->tpa == inet_addr(get_interface_ip(m.interface))) {
				struct ether_header *new_eth_hdr = (struct ether_header *) malloc(sizeof(struct ether_header));
				get_interface_mac(m.interface, new_eth_hdr->ether_shost);
				build_ethhdr(new_eth_hdr, new_eth_hdr->ether_shost, eth_hdr->ether_shost, eth_hdr->ether_type);
				send_arp(arp_hdr->spa, arp_hdr->tpa, new_eth_hdr, m.interface, htons(ARPOP_REPLY));

				free(new_eth_hdr);
				continue;
			}

			// if it's an ARP_REPLY
			if (ntohs(arp_hdr->op) == ARPOP_REPLY) {
				add_arp_entry(arp_table, ntohl(arp_hdr->spa), arp_hdr->sha);

				// if the queue is not empty
				if (!queue_empty(packet_queue)) {
					packet *new_packet = queue_deq(packet_queue);
					struct ether_header *new_eth_hdr = (struct ether_header *)new_packet->payload;
					get_interface_mac(new_packet->interface, new_eth_hdr->ether_shost);
					build_ethhdr(new_eth_hdr, new_eth_hdr->ether_shost, arp_hdr->sha, new_eth_hdr->ether_type);
					send_packet(new_packet->interface, new_packet);

					continue;
				}
			}

			continue;
		}

		// then it's just an IP packet and it should be forwarded
		struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));

		// verifies checksum
		if (ip_checksum(ip_hdr, sizeof(struct iphdr))) {
			continue;
		}

		// if TTL <= 1 send time exceeded
		if (ip_hdr->ttl <= 1) {
			send_icmp_error(ip_hdr->saddr, ip_hdr->daddr, eth_hdr->ether_dhost, eth_hdr->ether_shost,
						ICMP_TIME_EXCEEDED, ICMP_EXC_TTL, m.interface);

			continue;
		}

		// decrement ttl
		ip_hdr->ttl--;

		// updates checksum
		ip_hdr->check = 0;
		ip_hdr->check = ip_checksum(ip_hdr, sizeof(struct iphdr));


		// search for the most specific rtable_entry
		struct rtable_entry *best_route = get_best_route(rtable, ntohl(ip_hdr->daddr));

		// try to find a route
		if (best_route == NULL) {
			// can't find a route
			send_icmp_error(ip_hdr->daddr, inet_addr(get_interface_ip(m.interface)), eth_hdr->ether_dhost, eth_hdr->ether_shost,
						ICMP_DEST_UNREACH, ICMP_NET_UNREACH, m.interface);

			continue;
		}

		// try to find a match in arp_table
		struct arp_table_entry *match = get_arp_entry(arp_table, best_route->next_hop);

		// didn't find a match
		if (match == NULL) {
			// insert a copy of the packet in queue
			m.interface = best_route->interface;
			packet *copy = (packet *) malloc(sizeof(packet));
			memcpy(copy, &m, sizeof(m));
			queue_enq(packet_queue, copy);
			
			// send an ARP_REQUEST
			struct ether_header *new_eth_hdr = (struct ether_header *) malloc(sizeof(struct ether_header));
			get_interface_mac(best_route->interface, new_eth_hdr->ether_shost);
			hwaddr_aton(BROADCAST_ADDRESS, new_eth_hdr->ether_dhost);
			build_ethhdr(new_eth_hdr, new_eth_hdr->ether_shost, new_eth_hdr->ether_dhost, htons(ETHERTYPE_ARP));
			send_arp(htonl(best_route->next_hop), inet_addr(get_interface_ip(best_route->interface)), new_eth_hdr, best_route->interface, htons(ARPOP_REQUEST));

			free(new_eth_hdr);
			continue;
		}

		// modify source and destination MAC Addresses
		get_interface_mac(best_route->interface, eth_hdr->ether_shost);
		build_ethhdr(eth_hdr, eth_hdr->ether_shost, match->mac, eth_hdr->ether_type);
			
		// send the packet
		send_packet(best_route->interface, &m);
	}

	return 0;
}
