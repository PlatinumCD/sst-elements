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


#ifndef _H_EMBER_SHMEM_INIT_EVENT
#define _H_EMBER_SHMEM_INIT_EVENT

#include "emberShmemEvent.h"

namespace SST {
namespace Ember {

class EmberInitShmemEvent : public EmberShmemEvent {

public:
	EmberInitShmemEvent( Shmem::Interface& api, Output* output,
                    EmberEventTimeStatistic* stat = NULL ) :
            EmberShmemEvent( api, output, stat ){}
	~EmberInitShmemEvent() {}

    std::string getName() { return "Init"; }

    void issue( uint64_t time, Shmem::Callback callback ) {

        EmberEvent::issue( time );
        m_api.init( callback );
    }
};

}
}

#endif
