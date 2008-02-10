//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007, 2008 The NetBSD Foundation, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this
//    software must display the following acknowledgement:
//        This product includes software developed by the NetBSD
//        Foundation, Inc. and its contributors.
// 4. Neither the name of The NetBSD Foundation nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
// CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

extern "C" {
#include "atf-c/env.h"
}

#include "atf/env.hpp"
#include "atf/exceptions.hpp"
#include "atf/sanity.hpp"

namespace impl = atf::env;
#define IMPL_NAME "atf::env"

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

std::string
impl::get(const std::string& name)
{
    return atf_env_get(name.c_str());
}

bool
impl::has(const std::string& name)
{
    return atf_env_has(name.c_str());
}

void
impl::set(const std::string& name, const std::string& val)
{
    int ret = atf_env_set(name.c_str(), val.c_str());
    if (ret != 0)
        throw atf::system_error(IMPL_NAME "::set(" + name + ", " +
                                val + ")", "failed", ret);
}

void
impl::unset(const std::string& name)
{
    int ret = atf_env_unset(name.c_str());
    if (ret != 0)
        throw atf::system_error(IMPL_NAME "::unset(" + name + ")",
                                "failed", ret);
}
