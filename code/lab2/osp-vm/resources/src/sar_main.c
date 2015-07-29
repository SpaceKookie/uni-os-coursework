#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/list.h>
#include <linux/module.h>

#include "sar_main.h"

char procfs_buffer[PROCFS_MAX_SIZE];

static int __init sar_init(void)
{
	int result = 0;
	return result;
}

static void __exit sar_cleanup(void)
{
}

module_init(sar_init);
module_exit(sar_cleanup);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
