// Copyright 2009-2025 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2025, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// of the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _H_ARIEL_CPU
#define _H_ARIEL_CPU

#include <sst/core/sst_config.h>
#include <sst/core/interfaces/stdMem.h>
#include <sst/core/component.h>
#include <sst/core/params.h>

#include <stdint.h>
#include <unistd.h>

#include <string>
#include <map>

#include "arielmemmgr.h"
#include "arielcore.h"
#include "arielfrontend.h"
#include "ariel_shmem.h"

namespace SST {
namespace ArielComponent {

#define STRINGIZE(input) #input

class ArielCPU : public SST::Component {
    public:

    /* SST ELI */
    SST_ELI_REGISTER_COMPONENT(ArielCPU, "ariel", "ariel", SST_ELI_ELEMENT_VERSION(1,0,0), "PIN-based CPU model", COMPONENT_CATEGORY_PROCESSOR)

    SST_ELI_DOCUMENT_PARAMS(
        {"verbose", "Verbosity for debugging. Increased numbers for increased verbosity.", "0"},
        {"profilefunctions", "Profile functions for Ariel execution, 0 = none, >0 = enable", "0" },
        {"corecount", "Number of CPU cores to emulate", "1"},
        {"checkaddresses", "Verify that addresses are valid with respect to cache lines", "0"},
        {"maxissuepercycle", "Maximum number of requests to issue per cycle, per core", "1"},
        {"maxcorequeue", "Maximum queue depth per core", "64"},
        {"maxtranscore", "Maximum number of pending transactions", "16"},
        {"pipetimeout", "Read timeout between Ariel and traced application", "10"},
        {"cachelinesize", "Line size of the attached caching structure", "64"},
        {"arieltool", "Path to the Ariel PIN-tool shared library", ""},
        {"launcher", "Specify the launcher to be used for instrumentation, default is path to PIN", STRINGIZE(PINTOOL_EXECUTABLE)},
        {"executable", "Executable to trace", ""},
        {"appstdin", "Specify a file to use for the program's stdin", ""},
        {"appstdout", "Specify a file to use for the program's stdeout", ""},
        {"appstderr", "Specify a file to use for the program's stderr", ""},
        {"appstdoutappend", "If appstdout is set, set this to 1 to append the file intead of overwriting", "0"},
        {"appstderrappend", "If appstderr is set, set this to 1 to append the file intead of overwriting", "0"},
        {"launchparamcount", "Number of parameters supplied for the launch tool", "0" },
        {"launchparam%(launchparamcount)d", "Set the parameter to the launcher", "" },
        {"mpimode", "Whether to use <mpilauncher> to to launch <launcher> in order to trace MPI-enabled applications.", "0"},
        {"mpilauncher", "Specify a launcher to be used for MPI executables in conjuction with <launcher>", STRINGIZE(MPILAUNCHER_EXECUTABLE)},
        {"mpiranks", "Number of ranks to be launched by <mpilauncher>. Only <mpitracerank> will be traced by <launcher>.", "1" },
        {"mpitracerank", "Rank to be traced by <launcher>.", "0" },
        {"envparamcount", "Number of environment parameters to supply to the Ariel executable, default=-1 (use SST environment)", "-1"},
        {"envparamname%(envparamcount)d", "Sets the environment parameter name", ""},
        {"envparamval%(envparamcount)d", "Sets the environment parameter value", ""},
        {"appargcount", "Number of arguments to the traced executable", "0"},
        {"apparg%(appargcount)d", "Arguments for the traced executable", ""},
        {"arielmode", "Tool interception mode, set to 1 to trace entire program (default), set to 0 to delay tracing until ariel_enable() call., set to 2 to attempt auto-detect", "2"},
        {"arielinterceptcalls", "Toggle intercepting library calls", "0"},
        {"arielstack", "Dump stack on malloc calls (also requires enabling arielinterceptcalls). May increase overhead due to keeping a shadow stack.", "0"},
        {"mallocmapfile", "File with valid 'ariel_malloc_flag' ids", ""},
        {"tracePrefix", "Prefix when tracing is enable", ""},
        {"clock", "Clock rate at which events are generated and processed", "1GHz"},
        {"tracegen", "Select the trace generator for Ariel (which records traced memory operations", ""},
        {"memmgr", "Memory manager to use for address translation", "ariel.MemoryManagerSimple"},
        {"writepayloadtrace", "Trace write payloads and put real memory contents into the memory system", "0"},
        {"instrument_instructions", "turn on or off instruction instrumentation in fesimple", "1"})

    SST_ELI_DOCUMENT_PORTS( {"cache_link_%(corecount)d", "Each core's link to its cache", {}},
       {"rtl_link_%(corecount)d", "Each core's link to the RTL", {}})

    SST_ELI_DOCUMENT_STATISTICS(
        { "read_requests",        "Statistic counts number of read requests", "requests", 1},   // Name, Desc, Enable Level
        { "write_requests",       "Statistic counts number of write requests", "requests", 1},
        { "read_latency",         "Statistic for latency of read requests", "cycles", 1},
        { "write_latency",        "Statistic for latency of write requests", "cycles", 1},
        { "read_request_sizes",   "Statistic for size of read requests", "bytes", 1},
        { "write_request_sizes",  "Statistic for size of write requests", "bytes", 1},
        { "split_read_requests",  "Statistic counts number of split read requests (requests which come from multiple lines)", "requests", 1},
        { "split_write_requests", "Statistic counts number of split write requests (requests which are split over multiple lines)", "requests", 1},
        { "no_ops",               "Statistic counts instructions which do not execute a memory operation", "instructions", 1},
	    { "flush_requests",       "Statistic counts instructions which perform flushes", "requests", 1},
	    { "fence_requests",       "Statistic counts instructions which perform fences", "requests", 1},
        { "instruction_count",    "Statistic for counting instructions", "instructions", 1 },
        { "max_insts", "Maximum number of instructions reached by a thread",	"instructions", 0},
        { "fp_dp_ins",            "Statistic for counting DP-floating point instructions", "instructions", 1 },
        { "fp_dp_simd_ins",       "Statistic for counting DP-FP SIMD instructons", "instructions", 1 },
        { "fp_dp_scalar_ins",     "Statistic for counting DP-FP Non-SIMD instructons", "instructions", 1 },
        { "fp_dp_ops",            "Statistic for counting DP-FP operations (inst * SIMD width)", "instructions", 1 },
        { "fp_sp_ins",            "Statistic for counting SP-floating point instructions", "instructions", 1 },
        { "fp_sp_simd_ins",       "Statistic for counting SP-FP SIMD instructons", "instructions", 1 },
        { "fp_sp_scalar_ins",     "Statistic for counting SP-FP Non-SIMD instructons", "instructions", 1 },
        { "fp_sp_ops",            "Statistic for counting SP-FP operations (inst * SIMD width)", "instructions", 1 },
        { "cycles",               "Statistic for counting cycles of the Ariel core.", "cycles", 1 },
        { "active_cycles",        "Statistic for counting active cycles (cycles not idle) of the Ariel core.", "cycles", 1 })

    SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
            {"memmgr", "Memory manager to translate virtual addresses to physical, handle malloc/free, etc.", "SST::ArielComponent::ArielMemoryManager"},
            {"memory", "Interface to the memoryHierarchy (e.g., caches)", "SST::Interfaces::StandardMem" }
    )

        /* Ariel class */
        ArielCPU(ComponentId_t id, Params& params);
        ~ArielCPU();
        virtual void emergencyShutdown();
        virtual void init(unsigned int phase);
        virtual void setup() {}
        virtual void finish();
        virtual bool tick( SST::Cycle_t );

    private:
        SST::Output* output;
        ArielMemoryManager* memmgr;

        std::vector<ArielCore*> cpu_cores;
        std::vector<Interfaces::StandardMem*> cpu_to_cache_links;
        std::vector<SST::Link*> cpu_to_rtl_links;

        uint32_t core_count;

        ArielFrontend* frontend;
        ArielTunnel* tunnel;
        bool stopTicking;
};

}
}

#endif
