/**
 * Copyright (c) 2014 Anup Patel.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * @file arch_board.c
 * @author Anup Patel (anup@brainfault.org)
 * @brief various platform specific functions
 */

#include <arch_types.h>
#include <arch_board.h>
#include <arm_plat.h>
#include <basic_stdio.h>
#include <basic_string.h>
#include <pic/gic.h>
#include <timer/generic_timer.h>
#include <serial/pl01x.h>
#include <sys/vminfo.h>
#include <display/simplefb.h>

void arch_board_reset(void)
{
	long ret;
	unsigned long func, arg0, arg1, arg2;

	/* HVC-based PSCI v0.2 SYSTEM_RESET call */
	func = 0x84000009;
	arg0 = 0;
	arg1 = 0;
	arg2 = 0;
	asm volatile(
		".arch_extension sec\n\t"
		"mov	r0, %1\n\t"
		"mov	r1, %2\n\t"
		"mov	r2, %3\n\t"
		"mov	r3, %4\n\t"
		"smc	#0    \n\t"
		"mov	%0, r0\n\t"
	: "=r" (ret)
	: "r" (func), "r" (arg0), "r" (arg1), "r" (arg2)
	: "r0", "r1", "r2", "r3", "cc", "memory");

	if (ret) {
		basic_printf("%s: PSCI SYSTEM_RESET returned %d\n",
			     __func__, (int)ret);
	}
}

void arch_board_init(void)
{
	/* Nothing to do */
}

char *arch_board_name(void)
{
	return "Virt-v7";
}

physical_addr_t arch_board_ram_start(void)
{
	return (physical_addr_t)vminfo_ram_base(VIRT_V7_VMINFO, 0);
}

physical_size_t arch_board_ram_size(void)
{
	return (physical_size_t)vminfo_ram_size(VIRT_V7_VMINFO, 0);
}

void arch_board_linux_default_cmdline(char *cmdline, u32 cmdline_sz)
{
	basic_strcpy(cmdline, "root=/dev/ram rw earlyprintk "
			      "console=ttyAMA0");
}

void arch_board_fdt_fixup(void *fdt_addr)
{
	simplefb_fdt_fixup(VIRT_V7_SIMPLEFB, fdt_addr);
}

physical_addr_t arch_board_autoexec_addr(void)
{
	return (VIRT_V7_NOR_FLASH + 0xFF000);
}

u32 arch_board_boot_delay(void)
{
	return vminfo_boot_delay(VIRT_V7_VMINFO);
}

u32 arch_board_iosection_count(void)
{
	return 8;
}

physical_addr_t arch_board_iosection_addr(int num)
{
	physical_addr_t ret = 0;

	switch (num) {
	case 0:
		/* nor-flash */
		ret = VIRT_V7_NOR_FLASH;
		break;
	case 1:
		/* gic */
		ret = VIRT_V7_GIC;
		break;
	case 2:
		/* uart0 */
		ret = VIRT_V7_UART0;
		break;
	case 3:
		/* vminfo */
		ret = VIRT_V7_VMINFO;
		break;
	case 4:
		/* simplefb */
		ret = VIRT_V7_SIMPLEFB;
		break;
	case 5:
		/* virtio-net */
		ret = VIRT_V7_VIRTIO_NET;
		break;
	case 6:
		/* virtio-blk */
		ret = VIRT_V7_VIRTIO_BLK;
		break;
	case 7:
		/* virtio-con */
		ret = VIRT_V7_VIRTIO_CON;
		break;
	default:
		while (1);
		break;
	}

	return ret;
}

u32 arch_board_pic_nr_irqs(void)
{
	return NR_IRQS_VIRT_V7;
}

int arch_board_pic_init(void)
{
	int rc;

	/*
	 * Initialize Generic Interrupt Controller
	 */
	rc = gic_dist_init(0, VIRT_V7_GIC_DIST, IRQ_VIRT_V7_GIC_START);
	if (rc) {
		return rc;
	}
	rc = gic_cpu_init(0, VIRT_V7_GIC_CPU);
	if (rc) {
		return rc;
	}

	return 0;
}

u32 arch_board_pic_active_irq(void)
{
	return gic_active_irq(0);
}

int arch_board_pic_ack_irq(u32 irq)
{
	return 0;
}

int arch_board_pic_eoi_irq(u32 irq)
{
	return gic_eoi_irq(0, irq);
}

int arch_board_pic_mask(u32 irq)
{
	return gic_mask(0, irq);
}

int arch_board_pic_unmask(u32 irq)
{
	return gic_unmask(0, irq);
}

void arch_board_timer_enable(void)
{
	return generic_timer_enable();
}

void arch_board_timer_disable(void)
{
	return generic_timer_disable();
}

u64 arch_board_timer_irqcount(void)
{
	return generic_timer_irqcount();
}

u64 arch_board_timer_irqdelay(void)
{
	return generic_timer_irqdelay();
}

u64 arch_board_timer_timestamp(void)
{
	return generic_timer_timestamp();
}

void arch_board_timer_change_period(u32 usecs)
{
	return generic_timer_change_period(usecs);
}

int arch_board_timer_init(u32 usecs)
{
	return generic_timer_init(usecs, IRQ_VIRT_V7_VIRT_TIMER);
}

int arch_board_serial_init(void)
{
	pl01x_init(VIRT_V7_UART0, PL01X_TYPE_1, 115200, 24000000);

	return 0;
}

void arch_board_serial_putc(char ch)
{
	if (ch == '\n') {
		pl01x_putc(VIRT_V7_UART0, PL01X_TYPE_1, '\r');
	}
	pl01x_putc(VIRT_V7_UART0, PL01X_TYPE_1, ch);
}

bool arch_board_serial_can_getc(void)
{
	return pl01x_can_getc(VIRT_V7_UART0, PL01X_TYPE_1);
}

char arch_board_serial_getc(void)
{
	char ch = pl01x_getc(VIRT_V7_UART0, PL01X_TYPE_1);
	if (ch == '\r') {
		ch = '\n';
	}
	arch_board_serial_putc(ch);
	return ch;
}
