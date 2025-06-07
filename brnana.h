/**
 * @file brnana.h
 * @brief Header for brnana: a minimal bridge with extra potassium 🍌
 *
 * This file defines the core data structures and function declarations
 * used by the brnana bridge kernel module.
 */

#include <linux/etherdevice.h> /** Ethernet-specific helpers */
#include <linux/kernel.h>      /** Core kernel definitions */
#include <linux/module.h>      /** Module macros and interfaces */
#include <linux/netdevice.h>   /** Network device structures */

/** Default bridge interface name pattern */
#define BR_NAME "brnana%d"

/**
 * struct brnana_content - Global container for all brnana bridge instances
 * @br_list: A linked list of all registered brnana bridges
 */
struct brnana_content {
    struct list_head br_list; /** List head for all bridge interfaces */
};

/**
 * struct brnana_if - Represents a brnana bridge interface
 * @lock:         Spinlock to protect concurrent access to bridge state
 * @dev:          Pointer to the associated net_device structure
 * @br_id:        Unique bridge ID
 * @mac_addr:     MAC address of the bridge
 * @port_list:    List of slave interfaces (ports) attached to this bridge
 * @link:         Link to other bridges in brnana_content.br_list
 */
struct brnana_if {
    spinlock_t lock;
    struct net_device *dev;
    int br_id;
    unsigned char mac_addr[ETH_ALEN];
    struct list_head port_list;
    struct list_head link;
};

/**
 * struct brnana_port_if - Represents a slave port attached to a brnana bridge
 * @br:   Pointer back to the parent bridge structure
 * @dev:  The net_device representing the slave port
 * @link: Link in the bridge's port_list
 */
struct brnana_port_if {
    struct brnana_if *br;
    struct net_device *dev;
    struct list_head link;
};

/**
 * brnana_add_port - Attach a port to a brnana bridge
 * @br:    Pointer to the bridge to attach to
 * @dev:   The net_device to be enslaved
 * @extack: Extended netlink ack for reporting errors to user space
 *
 * Return: 0 on success, or a negative errno value on failure.
 */
int brnana_add_port(struct brnana_if *br,
                    struct net_device *dev,
                    struct netlink_ext_ack *extack);

/**
 * brnana_del_port - Detach a port from a brnana bridge
 * @br:  Pointer to the bridge to detach from
 * @dev: The net_device to be removed
 *
 * Return: 0 on success, or a negative errno value on failure.
 */
int brnana_del_port(struct brnana_if *br, struct net_device *dev);