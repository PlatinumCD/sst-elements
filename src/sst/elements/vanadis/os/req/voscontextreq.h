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

#ifndef _H_VANADIS_CONTEXT_SAVE_REQ
#define _H_VANADIS_CONTEXT_SAVE_REQ

#include "os/req/voscorereq.h"

namespace SST {
namespace Vanadis {

class VanadisContextSaveReq : public VanadisCoreEventReq {
public:
    VanadisContextSaveReq() : VanadisCoreEventReq() {}

    VanadisContextSaveReq( int core, int thread, uint32_t tid ) :
        VanadisCoreEventReq( core, thread ), tid(tid) {}

    ~VanadisContextSaveReq() {}

    uint64_t getTid() { return tid; }

private:
    void serialize_order(SST::Core::Serialization::serializer& ser) override {
        VanadisCoreEventReq::serialize_order(ser);
        ser& tid;
    }

    ImplementSerializable(SST::Vanadis::VanadisContextSaveReq);
    uint32_t tid;
};

class VanadisContextLoadReq : public VanadisCoreEventReq {
public:
    VanadisContextLoadReq() : VanadisCoreEventReq() {}

    VanadisContextLoadReq( int core, int thread, uint32_t tid, uint64_t instPtr, uint64_t tlsPtr ) : 
        VanadisCoreEventReq( core, thread ), tid(tid), instPtr(instPtr), tlsPtr(tlsPtr), isFork(false) {}

    ~VanadisContextLoadReq() {}

    uint32_t getTid() { return tid; }
    uint64_t getInstPtr() { return instPtr; }
    uint64_t getTlsPtr() { return tlsPtr; }

    bool isFork;
    std::vector<uint64_t> intRegs;
    std::vector<uint64_t> fpRegs;

private:
    void serialize_order(SST::Core::Serialization::serializer& ser) override {
        VanadisCoreEventReq::serialize_order(ser);
        ser& isFork;
        ser& tid;
        ser& intRegs;
        ser& fpRegs;
        ser& instPtr;
        ser& tlsPtr;
    }

    ImplementSerializable(SST::Vanadis::VanadisContextLoadReq);
    uint32_t tid;
    uint64_t instPtr;
    uint64_t tlsPtr;
};

}
}

#endif
