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

#ifndef _H_VANADIS_GET_THREAD_STATE_RESP
#define _H_VANADIS_GET_THREAD_STATE_RESP

#include <sst/core/event.h>
#include "os/resp/voscoreresp.h"

namespace SST {
namespace Vanadis {

class VanadisGetThreadStateResp : public VanadisCoreEventResp {
public:
    VanadisGetThreadStateResp() : VanadisCoreEventResp(), instPtr(-1), tlsPtr(-1) {}

    VanadisGetThreadStateResp( int core, int thread, uint64_t instPtr, uint64_t tlsPtr) : VanadisCoreEventResp( core, thread), instPtr(instPtr), tlsPtr(tlsPtr) {}

    ~VanadisGetThreadStateResp() {}

    uint64_t getInstPtr() { return instPtr; }
    uint64_t getTlsPtr() { return tlsPtr; }

    std::vector<uint64_t> intRegs;
    std::vector<uint64_t> fpRegs;

private:
    void serialize_order(SST::Core::Serialization::serializer& ser) override {
        VanadisCoreEventResp::serialize_order(ser);
        ser& intRegs;
        ser& fpRegs;
        ser& instPtr;
        ser& tlsPtr;
    }

    ImplementSerializable(SST::Vanadis::VanadisGetThreadStateResp);

    uint64_t instPtr;
    uint64_t tlsPtr;
};

} // namespace Vanadis
} // namespace SST

#endif
