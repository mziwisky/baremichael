/* BareMichael SCC baremetal framework.
 * Copyright (C) 2012.  All rights reserved. */

#include <stddef.h>
#include <stdio.h>
#include <scc.h>
#include <apic.h>
#include <interrupt.h>
#include <clock.h>

void dumbhandler0(void) {
	printf("I've been interrupted!\n");
#if RTCLOCK
	printf("(SCC has been booted for %d seconds)\n", clktime);
#endif
}

void dumbhandler1(void) {
	printf("Another interruption!\n");
#if RTCLOCK
	printf("(SCC has been booted for %d seconds)\n", clktime);
#endif
}

void dumbsleep(int time) {
	// dumb sleep for a while
	unsigned int i, j;
	for (j = 0; j < time; j++)
		for (i = 0; i < 100000; i++) ;
}

void main() {
	printf("Hello, World -- I'm core %d!\n", get_my_coreid());

#ifndef RCCE_SUPPORT
	if (get_my_coreid() == 47) {
		// Setup APIC LINT0 pin to trigger IRQ3 (which is IDT vector 0x23), 
		// and configure IRQ3 to run dumbhandler0()
		setupLINT(0, IRQ3, dumbhandler0);
		// Setup APIC LINT1 pin to trigger IRQ4 (IDT vector 0x24), and 
		// configure IRQ4 to run dumbhandler1()
		setupLINT(1, IRQ4, dumbhandler1);

		enable(); // Enable interrupts for this core
	}

	if (get_my_coreid() == 0) {
		dumbsleep(2000);
		printf("I'm going to trigger core 47's LINT0 now.\n");
		interrupt_core(47, 0);

		dumbsleep(2000);
		printf("Now I'm diddling core 47's LINT1.\n");
		interrupt_core(47, 1);
	}
#else
	if (get_my_coreid() == 0 || get_my_coreid() == 1)
	{
		dumbsleep(500);
		RCCE_pingpong();
	}
#endif /* RCCE_SUPPORT */

}
