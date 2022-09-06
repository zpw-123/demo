#ifndef __CPU_BROADWELL_H
#define __CPU_BROADWELL_H

#include <papi.h>
#include "debug.h"

// Perfmon2 is a library that provides a generic interface to access the PMU. It also comes with
// applications to list all available performance events with their architecture specific
// detailed description and translate them to their respective event code. 'showevtinfo' application can
// be used to list all available performance event names with detailed description and 'check_events' application
// can be used to translate the performance event to the corresponding event code.  

// These events will be initialized and started.
// Every event reading will return an array with the values for all these events.
// The array index is the same index used to define the event in the *_native_events array below
const char *broadwell_native_events[MAX_NUM_EVENTS] = {
    "CYCLE_ACTIVITY:STALLS_L2_PENDING",
    "MEM_LOAD_UOPS_L3_HIT_RETIRED:XSNP_NONE",
    "MEM_LOAD_UOPS_L3_MISS_RETIRED:REMOTE_DRAM",
    "MEM_LOAD_UOPS_L3_MISS_RETIRED:LOCAL_DRAM"
};

uint64_t broadwell_read_stall_events_local() {
    long long values[MAX_NUM_EVENTS];
    uint64_t events = 0;

    if (pmc_events_read_local_thread(values) == PAPI_OK) {
		uint64_t l2_pending = values[0];
		uint64_t llc_hit  = values[1];
		uint64_t remote_dram = values[2];
		uint64_t local_dram  = values[3];

		DBG_LOG(DEBUG, "read stall L2 cycles %lu; llc_hit %lu; remote_dram %lu; local_dram %lu\n",
			l2_pending, llc_hit, remote_dram, local_dram);

		double num = remote_dram + local_dram;
		double den = num + llc_hit;
		if (den == 0) return 0;

		events = (uint64_t)((double)l2_pending * ((double)num / den));
    } else {
        DBG_LOG(ERROR, "read stall cycles failed\n");
    }

    return events;
}

uint64_t broadwell_read_stall_events_remote() {
    long long values[MAX_NUM_EVENTS];
    uint64_t events = 0;

    if (pmc_events_read_local_thread(values) == PAPI_OK) {
		uint64_t l2_pending = values[0];
		uint64_t llc_hit  = values[1];
		uint64_t remote_dram = values[2];
		uint64_t local_dram  = values[3];

		DBG_LOG(DEBUG, "read stall L2 cycles %lu; llc_hit %lu; remote_dram %lu; local_dram %lu\n",
			l2_pending, llc_hit, remote_dram, local_dram);

		// calculate stalls based on l2 stalls and LLC miss/hit
		double num = remote_dram + local_dram;
		double den = num + llc_hit;
		if (den == 0) return 0;
		double stalls = (double)l2_pending * ((double)num / den);

		// calculate remote dram stalls based on total stalls and local/remote dram accesses
		den = remote_dram + local_dram;
		if (den == 0) return 0;
		events = (uint64_t) (stalls * ((double)remote_dram / den));
    } else {
        DBG_LOG(ERROR, "read stall cycles failed\n");
    }

    return events;
}

#endif // __CPU_BROADWELL_H