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

#ifndef _H_VANADIS_CORE_RESP
#define _H_VANADIS_CORE_RESP

#include <sst/core/event.h>

namespace SST {
namespace Vanadis {

class VanadisCoreEventResp : public SST::Event {
public:
    VanadisCoreEventResp() : SST::Event(), thread(-1), core(-1) {}
    VanadisCoreEventResp( int thread ) : SST::Event(), core(-1), thread(thread) {}
    VanadisCoreEventResp( int core, int thread ) : SST::Event(), core(core), thread(thread) {}
    int     getThread() { return thread; }
    int     getCore() { return core; }

protected:
    int     thread;
    int     core;

    void serialize_order(SST::Core::Serialization::serializer& ser) override {
        Event::serialize_order(ser);
        ser& thread;
        ser& core;
    }

    ImplementSerializable(SST::Vanadis::VanadisCoreEventResp);
};

}
}

#endif
