#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xb3753869, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xb053cec, "native_write_cr4" },
	{ 0xe6278ff5, "__register_chrdev" },
	{ 0x2644500e, "pci_bus_write_config_word" },
	{ 0xb44ad4b3, "_copy_to_user" },
	{ 0xc5850110, "printk" },
	{ 0xa0eae826, "smp_call_function" },
	{ 0x3247fdeb, "pci_bus_read_config_word" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xd424be4c, "pci_find_next_bus" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x362ef408, "_copy_from_user" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "9C4BCD7BAADBFF67CF6D431");
