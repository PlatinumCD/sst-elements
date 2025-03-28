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

#ifndef _H_VANADIS_UTIL_CACHE_LINE_SPLIT
#define _H_VANADIS_UTIL_CACHE_LINE_SPLIT

namespace SST {
namespace Vanadis {

static uint64_t
vanadis_line_remainder(const uint64_t start, const uint64_t line_length) {
    if (64 == line_length) {
        return line_length - (start & (uint64_t)63);
    } else {
        return line_length - (start % line_length);
    }
};

} // namespace Vanadis
} // namespace SST

#endif
