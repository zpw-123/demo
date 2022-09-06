#ifndef __CPU_BROADWELL_H
#define __CPU_BROADWELL_H

#include <math.h>
#include "thread.h"
#include "cpu/pmc.h"
#include "debug.h"

// Perfmon2 is a library that provides a generic interface to access the PMU. It also comes with
// applications to list all available performance events with their architecture specific
// detailed description and translate them to their respective event code. 'showevtinfo' application can
// be used to list all available performance event names with detailed description and 'check_events' application
// can be used to translate the performance event to the corresponding event code.  

extern __thread int tls_hw_local_latency;
extern __thread int tls_hw_remote_latency;

extern __thread uint64_t write_latency_global;
extern __thread uint64_t read_latency_global;

extern __thread uint64_t hw_latency_global;

#ifdef MEMLAT_SUPPORT
extern __thread uint64_t tls_global_remote_dram;
extern __thread uint64_t tls_global_local_dram;
#endif

#undef FOREACH_PMC_HW_EVENT
#define FOREACH_PMC_HW_EVENT(ACTION)                                                                       \
  ACTION("CYCLE_ACTIVITY:STALLS_L2_PENDING", NULL, 0x55305a3)                                              \
  ACTION("MEM_LOAD_UOPS_L3_HIT_RETIRED:XSNP_NONE", NULL, 0x5308d2)                                        \
  ACTION("MEM_LOAD_UOPS_L3_MISS_RETIRED:REMOTE_DRAM", NULL, 0x5304d3)                                     \
  ACTION("UNC_H_REQUESTS.WRITES_REMOTE", NULL, 0x40801)//0x401) //0x500801)					
  
#undef FOREACH_PMC_EVENT
#define FOREACH_PMC_EVENT(ACTION, prefix)                                                                  \
  ACTION(ldm_stall_cycles, prefix)                                                                         \
  ACTION(remote_dram, prefix)

#define L3_FACTOR 7.0

DECLARE_ENABLE_PMC(broadwell, ldm_stall_cycles)
{
    ASSIGN_PMC_HW_EVENT_TO_ME("CYCLE_ACTIVITY:STALLS_L2_PENDING", 0);
    ASSIGN_PMC_HW_EVENT_TO_ME("MEM_LOAD_UOPS_L3_HIT_RETIRED:XSNP_NONE", 1);
    ASSIGN_PMC_HW_EVENT_TO_ME("MEM_LOAD_UOPS_L3_MISS_RETIRED:REMOTE_DRAM", 2);
    ASSIGN_PMC_HW_EVENT_TO_ME("UNC_H_REQUESTS.WRITES_REMOTE", 3);
//    ASSIGN_PMC_HW_EVENT_TO_ME("UNC_H_REQUESTS.WRITES_REMOTE", 4);
//    ASSIGN_PMC_HW_EVENT_TO_ME("UNC_H_REQUESTS.WRITES_LOCAL", 5);
	
    return E_SUCCESS;
}


DECLARE_CLEAR_PMC(broadwell, ldm_stall_cycles)
{
}

DECLARE_READ_PMC(broadwell, ldm_stall_cycles)
{
   uint64_t l2_pending_diff  = READ_MY_HW_EVENT_DIFF(0);
   uint64_t llc_hit_diff     = READ_MY_HW_EVENT_DIFF(1);
   uint64_t remote_dram_diff = READ_MY_HW_EVENT_DIFF(2);
   uint64_t local_dram_diff  = READ_MY_HW_EVENT_DIFF(3);
  // uint64_t remote_wrtie  = READ_MY_HW_EVENT_DIFF(4);
  // uint64_t remote_write1  = READ_MY_HW_EVENT_DIFF(5);

   //DBG_LOG(DEBUG, "read stall L2 cycles diff %lu; llc_hit %lu; cycles diff remote_dram %lu; local_dram %lu; remote_dram_new %lu; local_dram_new %lu\n",l2_pending_diff, llc_hit_diff, remote_dram_diff, local_dram_diff, remote_wrtie, remote_write1);

   if ((remote_dram_diff == 0) && (local_dram_diff == 0)) return 0;
#ifdef MEMLAT_SUPPORT
   tls_global_local_dram += local_dram_diff;
#endif

   // calculate stalls based on L2 stalls and LLC miss/hit
   double num = L3_FACTOR * (remote_dram_diff + local_dram_diff);
   double den = num + llc_hit_diff;
   if (den == 0) return 0;
  printf("-------------------------ldfm\n");
	 return (uint64_t) ((double)l2_pending_diff * (num / den));
}

DECLARE_ENABLE_PMC(broadwell, remote_dram)
{
    ASSIGN_PMC_HW_EVENT_TO_ME("CYCLE_ACTIVITY:STALLS_L2_PENDING", 0);
    ASSIGN_PMC_HW_EVENT_TO_ME("MEM_LOAD_UOPS_L3_HIT_RETIRED:XSNP_NONE", 1);
    ASSIGN_PMC_HW_EVENT_TO_ME("MEM_LOAD_UOPS_L3_MISS_RETIRED:REMOTE_DRAM", 2);
    ASSIGN_PMC_HW_EVENT_TO_ME("UNC_H_REQUESTS.WRITES_REMOTE", 3);
    //ASSIGN_PMC_HW_EVENT_TO_ME("UNC_H_REQUESTS.WRITES_REMOTE", 4);
    //ASSIGN_PMC_HW_EVENT_TO_ME("UNC_H_REQUESTS.WRITES_LOCAL", 5);

    return E_SUCCESS;
}

DECLARE_CLEAR_PMC(broadwell, remote_dram)
{
}

DECLARE_READ_PMC(broadwell, remote_dram)
{
   uint64_t l2_pending_diff  = READ_MY_HW_EVENT_DIFF(0);
   uint64_t llc_hit_diff     = READ_MY_HW_EVENT_DIFF(1);
   uint64_t remote_dram_diff = READ_MY_HW_EVENT_DIFF(2);
   uint64_t remote_dram_write = READ_MY_HW_EVENT_DIFF(3);

   //uint64_t remote_wrtie  = READ_MY_HW_EVENT_DIFF(4);
   //uint64_t remote_write1  = READ_MY_HW_EVENT_DIFF(5);

   //DBG_LOG(DEBUG, "read stall L2 cycles diff %lu; llc_hit %lu; cycles diff remote_dram %lu; local_dram %lu; remote_dram_new %lu; local_dram_new %lu\n",l2_pending_diff, llc_hit_diff, remote_dram_diff, local_dram_diff, remote_wrtie, remote_write1);

   if (remote_dram_diff == 0) return 0;
#ifdef MEMLAT_SUPPORT
   tls_global_remote_dram += remote_dram_diff;
#endif

   // calculate stalls based on L2 stalls and LLC miss/hit
   double num = L3_FACTOR * remote_dram_diff;
   double den = num + llc_hit_diff;
   if (den == 0) return 0;
   double stalls = (double)l2_pending_diff * (num / den);

   // calculate remote dram stalls based on total stalls and local/remote dram accesses
   // also consider the weight of remote memory access against local memory access
   den = (remote_dram_diff * tls_hw_remote_latency);
   if (den == 0) return 0;
	//printf("--------------------remote\n");

	return (uint64_t)((stalls * ((double)((remote_dram_diff) * tls_hw_remote_latency) / den))* (read_latency_global-hw_latency_global) + (double) remote_dram_write * L3_FACTOR * 6 * (write_latency_global-hw_latency_global));
}

/*
pmc_hw_event_t broadwell_known_hw_event[] = {    
    {"CYCLE_ACTIVITY:STALLS_L2_PENDING", NULL, 0x55305a3,0,0},                                              
    {"MEM_LOAD_UOPS_L3_HIT_RETIRED:XSNP_NONE", NULL, 0x5308d2,0,0},                                           
    {"MEM_LOAD_UOPS_L3_MISS_RETIRED:REMOTE_DRAM", NULL, 0x530cd3,0,0} ,                                      
    {"MEM_LOAD_UOPS_L3_MISS_RETIRED:LOCAL_DRAM", NULL, 0x5303d3,0,0},
    {NULL, NULL, 0, 0, 0}                         
  };                                              
pmc_event_t broadwell_known_event[] = {          
    {"ldm_stall_cycles", NULL, 0, 0, broadwell_create_pmc_ldm_stall_cycles, broadwell_clear_pmc_ldm_stall_cycles, broadwell_read_pmc_ldm_stall_cycles},
    {"remote_dram", NULL, 0, 0, broadwell_create_pmc_remote_dram, broadwell_clear_pmc_remote_dram, broadwell_read_pmc_remote_dram},
    {NULL, NULL, 0, 0, NULL, NULL, NULL}          
};  

pmc_events_t broadwell_pmc_events = {            
    num_hw_cntrs,                                 
    broadwell_known_hw_event,                      
    broadwell_known_event                          
};
*/
PMC_EVENTS(broadwell, 4)
#endif // __CPU_BROADWELL_H
