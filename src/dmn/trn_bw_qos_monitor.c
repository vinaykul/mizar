// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_agent_xdp_usr.c
 * @author The Mizar Team
 *
 * @brief User space APIs to program transit agent
 *
 * @copyright Copyright (c) 2022 The Authors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>


#include "bpf/bpf.h"
#include "bpf/libbpf.h"
#include "trn_log.h"
#include "trn_datamodel.h"

/* Defaults */
#define HOST_IF_NAME "ehost-1"
#define INIT_WAIT_INTERVAL_SEC 10
#define BANDWIDTH_POLL_INTERVAL_SEC 2
#define MIN_LOW_PRIORITY_BW_PCT 10.00
#define MAX_LOW_PRIORITY_BW_PCT 80.00
#define BUFFER_BW_PCT 10.00

static long int link_speed_bytes_per_sec = -1;
static unsigned int interface_ip_addr = 0;

static int tx_stats_map_fd = -1;
const char *tx_stats_map_path="/sys/fs/bpf/tx_stats_map";

static int bw_qos_monitor_config_map_fd = -1;
const char *bw_qos_config_map_path = "/sys/fs/bpf/tc/globals/bw_qos_config_map";

static struct tx_stats_t last_poll_tx_stats = {0};
static unsigned long current_egress_bw_limit_bytes_per_sec = 0;

int init_tx_stats_map_fd()
{
	if (tx_stats_map_fd == -1) {
		int fd = bpf_obj_get(tx_stats_map_path);
		if (fd <= 0) {
			TRN_LOG_WARN("bw_qos_monitor-%d: Failure getting tx_stats_map_fd. errno=%d:%s",
					__LINE__, errno, strerror(errno));
			return -1;
		}
		tx_stats_map_fd = fd;
	}
	TRN_LOG_INFO("bw_qos_monitor: tx_stats_map_fd: %d\n", tx_stats_map_fd);
	return tx_stats_map_fd;
}

int init_bw_qos_monitor_config_map_fd()
{
	if (bw_qos_monitor_config_map_fd == -1) {
		int fd = bpf_obj_get(bw_qos_config_map_path);
		if (fd <= 0) {
			TRN_LOG_WARN("bw_qos_monitor-%d: Failure getting bw_qos_config_map_fd. errno=%d:%s",
					__LINE__, errno, strerror(errno));
			return -1;
		}
		bw_qos_monitor_config_map_fd = fd;
	}
	TRN_LOG_INFO("bw_qos_monitor: bw_qos_monitor_config_map_fd: %d\n", bw_qos_monitor_config_map_fd);
	return bw_qos_monitor_config_map_fd;
}

int get_link_speed(const char* name)
{
	int rv, fd;
	long speed = -1;
	struct ifreq ifr;
	struct ethtool_cmd ethcmd = {0};

	if (strlen(name) >= IF_NAMESIZE) {
		return -ENAMETOOLONG;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, name, IFNAMSIZ-1);

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd < 0)
		return fd;

	while (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
		TRN_LOG_INFO("bw_qos_monitor: Waiting for interface %s ...\n", name);
		sleep(INIT_WAIT_INTERVAL_SEC);
	}

	ifr.ifr_data = (void *)&ethcmd;
	ethcmd.cmd = ETHTOOL_GSET; /* "Get settings" */
	if (ioctl(fd, SIOCETHTOOL, ifr) == -1) {
		/* Unknown */
		return -1;
	} else {
		/* Convert Mbps to bytes/sec */
		speed = ethtool_cmd_speed(&ethcmd);
		speed = 3000; //TODO: DO-NOT-MERGE: This is demo hack because virtio does not report link speed.
		speed = (speed * 1000) * (1000 / 8);
	}

	do {
		rv = close(fd);
	} while (rv == -1 && errno == EINTR);

	TRN_LOG_INFO("bw_qos_monitor: Interface: %s LinkSpeed: %ld bytes/second\n", name, speed);
	return speed;
}

int get_interface_ip(const char* name, unsigned int* ipaddr)
{
	int rv, fd;
	struct ifreq ifr;
	*ipaddr = 0;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return fd;

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, name, IFNAMSIZ-1);

	if (ioctl(fd, SIOCGIFADDR, &ifr) == -1) {
		return -1;
	} else {
		*ipaddr = htonl(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
	}

	do {
		rv = close(fd);
	} while (rv == -1 && errno == EINTR);

	TRN_LOG_INFO("bw_qos_monitor: Interface: %s IP: 0x%x\n", name, *ipaddr);
	return 0;
}

void process_bandwidth_allocation(const char *name, float redirect_bw_bytes_per_sec)
{
	UNUSED(name);
	int err;
	struct bw_qos_config_key_t bwqoskey = {0};
	struct bw_qos_config_t bwqoscfg = {0};
	float pct_redirect_bw_bytes_per_sec = (redirect_bw_bytes_per_sec * 100) / link_speed_bytes_per_sec;

	bwqoskey.saddr = interface_ip_addr;
	err = bpf_map_lookup_elem(bw_qos_monitor_config_map_fd, &bwqoskey, &bwqoscfg);
	if (err) {
		TRN_LOG_ERROR("bw_qos_monitor-%d: BPF map lookup for bw_qos_config_map failed. Err: %d:%s.", __LINE__, err, strerror(err));
		return;
	}
	current_egress_bw_limit_bytes_per_sec = bwqoscfg.egress_bandwidth_bytes_per_sec;
	float current_egress_bw_limit_bytes_per_sec_pct = (current_egress_bw_limit_bytes_per_sec * 100) / link_speed_bytes_per_sec;

	float pct_redirect_bw_bytes_per_sec_with_buffer = pct_redirect_bw_bytes_per_sec + BUFFER_BW_PCT;
	pct_redirect_bw_bytes_per_sec_with_buffer = (pct_redirect_bw_bytes_per_sec_with_buffer > 90.00) ? 90.00 : pct_redirect_bw_bytes_per_sec_with_buffer;
	float available_egress_bw_limit_bytes_per_sec_pct = 100.00 - pct_redirect_bw_bytes_per_sec_with_buffer;
	available_egress_bw_limit_bytes_per_sec_pct = (available_egress_bw_limit_bytes_per_sec_pct > MAX_LOW_PRIORITY_BW_PCT) ? MAX_LOW_PRIORITY_BW_PCT : available_egress_bw_limit_bytes_per_sec_pct;
	available_egress_bw_limit_bytes_per_sec_pct = (available_egress_bw_limit_bytes_per_sec_pct < MIN_LOW_PRIORITY_BW_PCT) ? MIN_LOW_PRIORITY_BW_PCT : available_egress_bw_limit_bytes_per_sec_pct;

//TRN_LOG_WARN("bw_qos_monitor: VDBG: RDIR_BW_BYTPSEC: %.2f, LNKSPD_BYTPSEC: %ld PCT_RDIR_BW_BYTPSEC: %.2f, CUR_EGR_BWLM_BYTPSEC: %lu, PCT_CUR_EGR_BWLM_BYTPSEC: %.2f",
//		redirect_bw_bytes_per_sec, link_speed_bytes_per_sec, pct_redirect_bw_bytes_per_sec, current_egress_bw_limit_bytes_per_sec, current_egress_bw_limit_bytes_per_sec_pct);

	if (available_egress_bw_limit_bytes_per_sec_pct != current_egress_bw_limit_bytes_per_sec_pct) {
		// Update bw_qos_config_map_fd map
		bwqoscfg.egress_bandwidth_bytes_per_sec = (unsigned long)((int)available_egress_bw_limit_bytes_per_sec_pct * (link_speed_bytes_per_sec / 100));
		err = bpf_map_update_elem(bw_qos_monitor_config_map_fd, &bwqoskey, &bwqoscfg, 0);
TRN_LOG_WARN("bw_qos_monitor-DBG: BPF-EDT-MAP-UPDATE. CUR_BW_LIM_BYTES_PER_SEC: %luK (%.2f%%) --> NEW_BW_LIM_BYTES_PER_SEC: %lld (%.2f%%)",
		current_egress_bw_limit_bytes_per_sec, current_egress_bw_limit_bytes_per_sec_pct, bwqoscfg.egress_bandwidth_bytes_per_sec, available_egress_bw_limit_bytes_per_sec_pct);
//TRN_LOG_WARN("bw_qos_monitor-DBG: BPF-EDT-MAP-UPDATE. CUR_BW_LIM_BYTPSEC: %luK (%.2f%%)  NEW_BW_LIM_BYT_SEC: %lld (%.2f%%). Err: %d:%s.",
//		current_egress_bw_limit_bytes_per_sec, current_egress_bw_limit_bytes_per_sec_pct, bwqoscfg.egress_bandwidth_bytes_per_sec, available_egress_bw_limit_bytes_per_sec_pct, err, strerror(err));
		if (err) {
			TRN_LOG_ERROR("bw_qos_monitor-%d: BPF map update for bw_qos_config_map failed. Err: %d:%s.", __LINE__, err, strerror(err));
			return;
		}
	}
}

void* bw_qos_monitor(void *argv)
{
	UNUSED(argv);

	int err;
	struct tx_stats_key_t key = {0};
	struct tx_stats_t txstats;

	link_speed_bytes_per_sec = get_link_speed(HOST_IF_NAME);
	if (link_speed_bytes_per_sec < 0) {
		TRN_LOG_ERROR("bw_qos_monitor-%d: Unable to determine link speed for interface %s. Err: %d:%s.",
				__LINE__, HOST_IF_NAME, err, strerror(err));
		return NULL;
	}

	while (init_tx_stats_map_fd() < 0) {
		TRN_LOG_WARN("bw_qos_monitor: Waiting for tx_stats_map create...");
		sleep(INIT_WAIT_INTERVAL_SEC);
	}
	while (init_bw_qos_monitor_config_map_fd() < 0) {
		TRN_LOG_WARN("bw_qos_monitor: Waiting for bw_qos_config_map create...");
		sleep(INIT_WAIT_INTERVAL_SEC);
	}
	do {
		TRN_LOG_WARN("Waiting for tx_stats_map initialize...");
		err = bpf_map_lookup_elem(tx_stats_map_fd, &key, &txstats);
		sleep(BANDWIDTH_POLL_INTERVAL_SEC);
	} while (err);
	memcpy(&last_poll_tx_stats, &txstats, sizeof(txstats));

	if (get_interface_ip(HOST_IF_NAME, &interface_ip_addr) < 0) {
		TRN_LOG_ERROR("bw_qos_monitor-%d: Unable to query IPv4 address for interface %s.", __LINE__, HOST_IF_NAME);
		return NULL;
	}

	while (1) {
		err = bpf_map_lookup_elem(tx_stats_map_fd, &key, &txstats);
		if (err) {
			TRN_LOG_ERROR("bw_qos_monitor-%d: Lookup BPF map for bw qos config failed. Err: %d:%s.", __LINE__, err, strerror(err));
			return NULL;
		}

		//TODO: Better algorithm for high priority (redirect) bandwidth use computation
		__u64 redirect_bytes_since_last_poll = (txstats.tx_bytes_xdp_redirect > last_poll_tx_stats.tx_bytes_xdp_redirect) ?
							(txstats.tx_bytes_xdp_redirect - last_poll_tx_stats.tx_bytes_xdp_redirect) : 0;
		float redirect_bw_bytes_per_sec = ((float)(redirect_bytes_since_last_poll)) / BANDWIDTH_POLL_INTERVAL_SEC;
		memcpy(&last_poll_tx_stats, &txstats, sizeof(txstats));

		process_bandwidth_allocation(HOST_IF_NAME, redirect_bw_bytes_per_sec);

		sleep(BANDWIDTH_POLL_INTERVAL_SEC);
	}

	pthread_exit(NULL);

	return NULL;
}
