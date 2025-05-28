#include "brnana.h"

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Elian");
MODULE_DESCRIPTION("C(  o  .  o  ) ╯ brnana - the minimal bridge with extra potassium!🍌 ");
MODULE_VERSION("0.1");

/* Customize the number of bridges constructed by brnana */
static int num_bridge = 1;
module_param(num_bridge, int, 0444);
MODULE_PARM_DESC(num_bridge, "Number of bridges in brnana.");

static int __init brnana_init(void) {   
    pr_info("C( o . o ) ╯ brnana: %d bridge loaded\n", num_bridge);


    return 0;
}

static void __exit brnana_exit(void) {
    pr_info("C( o . o ) ╯ brnana: %d bridge unloaded\n", num_bridge);
    
}

module_init(brnana_init);
module_exit(brnana_exit);