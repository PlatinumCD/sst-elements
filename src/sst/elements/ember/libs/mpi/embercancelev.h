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


#ifndef _H_EMBER_CANCEL_EV
#define _H_EMBER_CANCEL_EV

#include "emberMPIEvent.h"

namespace SST {
namespace Ember {

class EmberCancelEvent : public EmberMPIEvent {

public:
	EmberCancelEvent( MP::Interface& api, Output* output,
                   EmberEventTimeStatistic* stat,
       		MessageRequest req ) :
       	EmberMPIEvent( api, output, stat ),
       	m_req( req )
    { }

	~EmberCancelEvent() {}

    std::string getName() { return "Cancel"; }

    void issue( uint64_t time, FOO* functor ) {

        EmberEvent::issue( time );

       	m_api.cancel( m_req, functor );
    }

private:
    MessageRequest 	m_req;
};

}
}

#endif
