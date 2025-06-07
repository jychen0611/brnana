/** 
 * @file brnana.c
 * @brief Core implementation of the brnana bridge module
 */
#include "brnana.h"

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Elian");
MODULE_DESCRIPTION(
    "C(  o  .  o  ) ╯ brnana - the minimal bridge with extra potassium!🍌 ");
MODULE_VERSION("0.2");

/**
 * num_bridge - Module parameter to control the number of bridges created
 * Default: 1. Can be set via insmod: insmod brnana.ko num_bridge=2
 */
static int num_bridge = 1;
module_param(num_bridge, int, 0444);
MODULE_PARM_DESC(num_bridge, "Number of bridges in brnana.");

/**
 * brnana - Global pointer to the top-level structure holding all bridges
 * This structure is allocated once at module init and holds the br_list.
 */
static struct brnana_content *brnana = NULL;

/**
 * dev_get_brnana_if - Helper to retrieve brnana_if from a net_device
 * @dev: Pointer to the bridge's net_device
 *
 * Returns:
 *   A pointer to the private brnana_if structure embedded in the net_device.
 *   This function assumes the device was allocated via alloc_netdev().
 */
static inline struct brnana_if *dev_get_brnana_if(struct net_device *dev)
{
    /* Bridge interface pointed to by netdev_priv() */
    return (struct brnana_if *) netdev_priv(dev);
}

/**
 * brnana_dev_open - ndo_open callback for bridge device
 * @dev: The net_device representing the brnana bridge
 *
 * This function is called when the bridge interface is brought up using:
 *   ip link set dev brnana0 up
 *
 * It starts the transmission queue and updates the interface's feature flags.
 *
 * Return:
 *   0 on success.
 */
static int brnana_dev_open(struct net_device *dev)
{
	struct brnana_if *br = dev_get_brnana_if(dev);
	pr_info("C( o . o ) ╯ brnana: bridge %d open\n", br->br_id);

	/* Refresh the device's feature flags based on current configuration */
	netdev_update_features(dev);

	/* Start the transmit queue for the interface */
	netif_start_queue(dev);	

	return 0;
}

/**
 * brnana_dev_stop - ndo_stop callback for bridge device
 * @dev: The net_device representing the brnana bridge
 *
 * This function is called when the bridge interface is brought down using:
 *   ip link set dev brnana0 down
 *
 * It stops the transmission queue to prevent packet output.
 *
 * Return:
 *   0 on success.
 */
static int brnana_dev_stop(struct net_device *dev)
{
	struct brnana_if *br = dev_get_brnana_if(dev);
	pr_info("C( o . o ) ╯ brnana: bridge %d stop\n", br->br_id);

	/* Stop the transmit queue for the interface */
	netif_stop_queue(dev);

	return 0;
}

/**
 * brnana_dev_init - ndo_init callback for bridge device
 * @dev: The net_device representing the brnana bridge
 *
 * This function is called during net_device registration. It is used
 * for driver-specific one-time initialization. In this implementation,
 * it simply logs bridge initialization.
 *
 * Return:
 *   0 on success.
 */
static int brnana_dev_init(struct net_device *dev)
{
	struct brnana_if *br = dev_get_brnana_if(dev);
	pr_info("C( o . o ) ╯ brnana: bridge %d init\n", br->br_id);

	return 0;
}

/**
 * brnana_dev_uninit - ndo_uninit callback for bridge device
 * @dev: The net_device representing the brnana bridge
 *
 * This function is called during net_device unregistration. It allows the
 * driver to clean up any resources that were initialized in ndo_init.
 * In this implementation, it simply logs the uninitialization event.
 */
static void brnana_dev_uninit(struct net_device *dev)
{
	struct brnana_if *br = dev_get_brnana_if(dev);
	pr_info("C( o . o ) ╯ brnana: bridge %d uninit\n", br->br_id);
}

/**
 * brnana_dev_xmit - ndo_start_xmit callback for bridge device
 * @skb: The socket buffer containing the packet to transmit
 * @dev: The net_device representing the brnana bridge
 *
 * This function is called when the kernel attempts to transmit a packet
 * through the brnana bridge interface. It is the primary data transmission
 * hook for net_device drivers.
 *
 * In this minimal implementation, the function does not perform actual
 * packet forwarding or transmission. It simply logs the event and drops
 * the packet.
 *
 * Note:
 *   - This function is always called with bottom halves (BH) disabled.
 *   - To forward packets to enslaved ports, logic should be added here.
 *
 * Return:
 *   NETDEV_TX_OK to indicate the packet was "successfully sent"
 *   (even if it was not processed or forwarded).
 */
static netdev_tx_t brnana_dev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct brnana_if *br = dev_get_brnana_if(dev);
	pr_info("C( o . o ) ╯ brnana: bridge %d start xmit\n", br->br_id);

	/* No forwarding implemented yet — drop or ignore the packet */

	return NETDEV_TX_OK;
}

/**
 * brnana_set_mac_address - Set the MAC address of a brnana bridge
 * @dev: The net_device representing the bridge
 * @p: Pointer to a struct sockaddr containing the new MAC address
 *
 * This function is invoked when a user attempts to change the MAC address
 * of the bridge interface, for example using:
 *   ip link set dev brnana0 address XX:XX:XX:XX:XX:XX
 *
 * It validates the new MAC address and applies it if appropriate.
 *
 * Return:
 *   0 on success, or a negative errno on failure.
 */
static int brnana_set_mac_address(struct net_device *dev, void *p)
{
	struct brnana_if *br = dev_get_brnana_if(dev);
	struct sockaddr *addr = p;

    /**
     * Validate that the provided address is a valid unicast Ethernet MAC.
     * Reject multicast or all-zero addresses.
     */
	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	/**
	 * Ensure that the interface is already registered before allowing changes.
	 * Prevents MAC changes during device setup stages.
	 */
	if (dev->reg_state != NETREG_REGISTERED)
		return -EBUSY;

	/**
	 * Protect concurrent access to the MAC address with a spinlock.
	 * This ensures safe modification even in softirq context.
	 */
	spin_lock_bh(&br->lock);

	/**
	 * Only update the address if it is different from the current one.
	 */
	if (!ether_addr_equal(dev->dev_addr, addr->sa_data)) {
        pr_info("C( o . o ) ╯ brnana: bridge %d set mac : %pM\n",
                br->br_id, addr->sa_data);
		memcpy(br->mac_addr, addr, ETH_ALEN);
        eth_hw_addr_set(dev, addr->sa_data);
	}

	spin_unlock_bh(&br->lock);

	return 0;
}

/**
 * brnana_add_slave - ndo_add_slave callback to add a slave device
 * @dev: The brnana bridge net_device (i.e., master)
 * @slave_dev: The net_device to be enslaved (i.e., slave)
 * @extack: Extended netlink acknowledgment structure
 *
 * This function is called by the kernel when a user runs:
 *   ip link set <slave_dev> master <brnana>
 *
 * It converts the bridge's net_device to its internal brnana_if structure
 * and calls brnana_add_port() to perform the actual linking.
 *
 * Return:
 *   0 on success, or a negative errno on failure.
 */
static int brnana_add_slave(struct net_device *dev,
                            struct net_device *slave_dev,
                            struct netlink_ext_ack *extack)
{
	struct brnana_if *br = dev_get_brnana_if(dev);

	return brnana_add_port(br, slave_dev, extack);
}

/**
 * brnana_del_slave - ndo_del_slave callback to remove a slave device
 * @dev: The brnana bridge net_device (i.e., master)
 * @slave_dev: The net_device to be removed from the bridge
 *
 * This function is called by the kernel when a user runs:
 *   ip link set <slave_dev> nomaster
 *
 * It converts the bridge's net_device to its internal brnana_if structure
 * and calls brnana_del_port() to handle port unlinking and cleanup.
 *
 * Return:
 *   0 on success, or a negative errno on failure.
 */
static int brnana_del_slave(struct net_device *dev,
                            struct net_device *slave_dev)
{
	struct brnana_if *br = dev_get_brnana_if(dev);

	return brnana_del_port(br, slave_dev);
}

/**
 * brnana_netdev_ops - Network device operations for brnana bridge
 *
 * This structure defines the set of callback functions that implement
 * the core behavior of the brnana bridge net_device. These callbacks
 * are invoked by the kernel networking subsystem for standard operations
 * such as interface bring-up/down, packet transmission, slave management, etc.
 */
static const struct net_device_ops brnana_netdev_ops = {
	.ndo_open            = brnana_dev_open,         /** Called when the interface is brought up (e.g., `ip link set up`) */
	.ndo_stop            = brnana_dev_stop,         /** Called when the interface is brought down */
	.ndo_init            = brnana_dev_init,         /** Called during device registration for one-time setup */
	.ndo_uninit          = brnana_dev_uninit,       /** Called during device unregistration for cleanup */
	.ndo_start_xmit      = brnana_dev_xmit,         /** Called to transmit a packet (called from upper layers) */
	.ndo_get_stats64     = dev_get_tstats64,        /** Default implementation for fetching 64-bit interface stats */
	.ndo_set_mac_address = brnana_set_mac_address,  /** Called to set a new MAC address on the bridge */
	.ndo_add_slave       = brnana_add_slave,        /** Called when a slave is attached (e.g., `ip link set dev dummy0 master brnana0`) */
	.ndo_del_slave       = brnana_del_slave,        /** Called when a slave is detached (e.g., `ip link set dev dummy0 nomaster`) */
};

/**
 * brnana_add_port - Enslave a device to the brnana bridge
 * @br: Pointer to the bridge (brnana_if) structure
 * @dev: The net_device to be enslaved as a port
 * @extack: Netlink extended acknowledgment structure
 *
 * This function is called when a user attempts to attach a network interface
 * (e.g., dummy0, eth1) to the brnana bridge (e.g., brnana0) using
 * `ip link set dev <ifname> master brnana0`.
 *
 * It dynamically allocates a port structure, links it into the bridge's port list,
 * and sets up the appropriate kernel relationships via netlink.
 *
 * The function must be called with the RTNL lock held.
 *
 * Return:
 *   0 on success, or a negative errno on failure.
 */
int brnana_add_port(struct brnana_if *br, struct net_device *dev,
                    struct netlink_ext_ack *extack)
{
    struct brnana_port_if *p;

    /**
     * Validate input: ensure slave device pointer is not NULL.
     */
    if (!dev) {
        pr_warn("C( o . o ) ╯ brnana: slave device is NULL\n");
        return -EINVAL;
    }

    /**
     * Prevent recursive bridging: disallow enslaving another brnana bridge.
     */
    if (dev->netdev_ops == &brnana_netdev_ops) {
        pr_warn("C( o . o ) ╯ brnana: refusing to enslave a brnana bridge\n");
        return -ELOOP;
    }

    /**
     * Allocate and zero-initialize a new brnana_port_if structure for this slave.
     */
    p = kzalloc(sizeof(*p), GFP_KERNEL);
    if (!p)
        return -ENOMEM;

    /**
     * Mark the device as part of a bridge. This is optional but helps in diagnostics.
     */
    dev->priv_flags |= IFF_BRIDGE_PORT;

    /**
     * Associate the port structure with the slave device using RCU-safe pointer assignment.
     */
    rcu_assign_pointer(dev->rx_handler_data, p);

    /**
     * Initialize the port's list node and store a back-reference to the device.
     */
    INIT_LIST_HEAD(&p->link);
    p->dev = dev;
    p->br = br;

    /**
     * Add the new port to the bridge's list of ports (RCU-safe insertion).
     */
    list_add_rcu(&p->link, &br->port_list);

    pr_info("C( o . o ) ╯ brnana: enslaved %s to brnana%d\n", dev->name, br->br_id);

    /**
     * Inform the kernel's net_device core that this device now has a master (the bridge).
     * This ensures that `ip link` and sysfs reflect the correct relationship.
     */
    int err = netdev_master_upper_dev_link(dev, br->dev, NULL, NULL, extack);
    if (err) {
        pr_warn("brnana: failed to link %s to %s as master: %d\n",
                dev->name, br->dev->name, err);

        /**
         * If the upper device link failed, clean up the partially added port structure.
         */
        list_del_rcu(&p->link);
        RCU_INIT_POINTER(dev->rx_handler_data, NULL);
        synchronize_rcu();
        kfree(p);
        return err;
    }

    return 0;
}

/**
 * brnana_del_port - Detach and clean up a slave device from a brnana bridge
 * @br: Pointer to the brnana bridge structure
 * @dev: The slave net_device to be removed from the bridge
 *
 * This function is called when a device is being removed from the bridge
 * (e.g., via `ip link set <dev> nomaster`). It unlinks the upper device
 * relationship, removes the port from the bridge's list, unregisters the RX
 * handler (if used), and releases associated memory.
 *
 * The function must be called with RTNL lock held.
 *
 * Return:
 *   0 on success, or a negative error code if the operation fails.
 */
int brnana_del_port(struct brnana_if *br, struct net_device *dev)
{
    struct brnana_port_if *p;

    /**
     * Sanity check: ensure the device pointer is valid.
     */
    if (!dev) {
        pr_warn("C( o . o ) ╯ brnana: attempt to del NULL device\n");
        return -EINVAL;
    }

    /**
     * Unlink the master-upper relationship from the kernel's networking core.
     * This removes `br->dev` as the upper (master) of `dev`.
     */
    netdev_upper_dev_unlink(dev, br->dev);

    /**
     * Retrieve the brnana port interface data via RCU-safe dereference.
     * If the port wasn't previously registered, abort safely.
     */
    p = rcu_dereference(dev->rx_handler_data);
    if (!p) {
        pr_warn("C( o . o ) ╯ brnana: device %s is not a brnana port\n", dev->name);
        return -ENODEV;
    }

    pr_info("C( o . o ) ╯ brnana: removing port %s from brnana%d\n",
            dev->name, br->br_id);

    /**
     * If a custom RX handler was registered to intercept packets on the slave,
     * unregister it here to restore default network stack behavior.
     */
    netdev_rx_handler_unregister(dev);

    /**
     * Remove the port entry from the bridge’s port list using RCU-safe removal.
     */
    list_del_rcu(&p->link);

    /**
     * Clear the RX handler data so that the device is no longer marked as enslaved.
     */
    RCU_INIT_POINTER(dev->rx_handler_data, NULL);

    /**
     * Ensure all concurrent RCU readers have exited before freeing memory.
     */
    synchronize_rcu();

    /**
     * Free the dynamically allocated brnana_port_if structure.
     */
    kfree(p);

    return 0;
}

/**
 * brnana_add_br - Construct and register a bridge interface
 * @idx: The index of the bridge to be created (used for ID and name generation)
 *
 * This function allocates a net_device representing a software bridge,
 * initializes its internal structures, registers it with the kernel,
 * and adds it to the global list of brnana bridges.
 *
 * Return:
 *   0 on success, negative error code on failure.
 */
static int brnana_add_br(int idx)
{
    struct net_device *dev = NULL;

    /**
     * Allocate a new Ethernet device with private data of type struct brnana_if.
     * The device name format is defined by BR_NAME ("brnana%d").
     */
    dev = alloc_netdev(sizeof(struct brnana_if), BR_NAME, NET_NAME_ENUM,
                       ether_setup);
    if (!dev) {
        pr_err("C( o . o ) ╯ brnana: Couldn't allocate space for nedv");
        return -ENOMEM;
    }

    /**
     * Assign the custom net_device_ops implementation to this bridge device.
     */
    dev->netdev_ops = &brnana_netdev_ops;

    /**
     * Register the device with the kernel networking subsystem.
     * This makes the interface visible to tools like `ip link`.
     */
    if (register_netdev(dev)) {
        pr_err("C( o . o ) ╯ brnana: Failed to register net device\n");
        free_netdev(dev);
        return -ENODEV;
    }

    /**
     * Initialize the bridge-specific context:
     * - Store device pointer and bridge ID
     * - Initialize spinlock for concurrent access
     * - Initialize list of ports connected to this bridge
     */
    struct brnana_if *br = dev_get_brnana_if(dev);
    br->dev = dev;
    br->br_id = idx;
    INIT_LIST_HEAD(&br->port_list);
    spin_lock_init(&br->lock);

    /**
     * Add this bridge to the global brnana bridge list.
     * The list is maintained for cleanup and tracking purposes.
     */
    list_add_tail(&br->link, &brnana->br_list);
    pr_info("C( o . o ) ╯ brnana: New BR %d\n", br->br_id);

    return 0;
}

/**
 * brnana_init - Module initialization function
 *
 * This function is called when the brnana module is loaded into the kernel.
 * It allocates the global bridge manager structure, initializes the internal
 * bridge list, and creates the requested number of bridge interfaces.
 *
 * Return:
 *   0 on success, negative error code on failure.
 */
static int __init brnana_init(void)
{
    pr_info("C( o . o ) ╯ brnana: %d bridge loaded\n", num_bridge);

    /**
     * Allocate memory for the global brnana_content structure,
     * which tracks all registered bridge interfaces.
     */
    brnana = kmalloc(sizeof(struct brnana_content), GFP_KERNEL);
    if (!brnana) {
        pr_err("C( o . o ) ╯ brnana: Couldn't allocate space for brnana_content\n");
        return -ENOMEM;
    }

    /**
     * Initialize the bridge list head inside the global structure.
     * This list will hold all active brnana bridge interfaces.
     */
    INIT_LIST_HEAD(&brnana->br_list);

    /**
     * Create and register each bridge interface according to the
     * `num_bridge` module parameter.
     */
    for (int i = 0; i < num_bridge; ++i) {
        brnana_add_br(i);
    }

    return 0;
}

/**
 * brnana_exit - Module cleanup function
 *
 * This function is invoked when the brnana module is removed from the kernel.
 * It performs cleanup of all dynamically allocated bridge and port structures,
 * unregisters each bridge net_device, and releases associated memory.
 */
static void __exit brnana_exit(void)
{
    pr_info("C( o . o ) ╯ brnana: %d bridge unloaded\n", num_bridge);

    /**
     * Iterate over each bridge (brnana_if) previously created.
     * For each bridge, clean up its associated ports and then
     * unregister and free the bridge device itself.
     */
    struct brnana_if *br = NULL, *safe1 = NULL;
    list_for_each_entry_safe(br, safe1, &brnana->br_list, link) {
        struct brnana_port_if *p, *safe2;

        /**
         * Iterate over each port attached to the bridge and remove it
         * from the bridge's port list. Clear its RX handler, wait for
         * any RCU readers to finish, and then free the memory.
         */
        list_for_each_entry_safe(p, safe2, &br->port_list, link) {
            list_del_rcu(&p->link);
            RCU_INIT_POINTER(p->dev->rx_handler_data, NULL);
            synchronize_rcu();  /* Ensure no readers are using p */
            kfree(p);           /* Free port memory */
        }

        /**
         * After all ports are cleaned up, unregister and free
         * the net_device representing the bridge.
         */
        unregister_netdev(br->dev);
        free_netdev(br->dev);
    }

    /* Finally, free the global brnana context structure */
    kfree(brnana);
}

module_init(brnana_init);
module_exit(brnana_exit);