//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007 The NetBSD Foundation, Inc.
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

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <errno.h>
#include <stdlib.h>

#include "env.h"
#include "sanity.h"

const char *
atf_env_get(const char *name)
{
    const char* val = getenv(name);
    PRE(val != NULL);
    return val;
}

bool
atf_env_has(const char *name)
{
    return getenv(name) != NULL;
}

int
atf_env_set(const char *name, const char *val)
{
#if defined(HAVE_SETENV)
    int ret = 0;

    if (setenv(name, val, 1) == -1)
        ret = errno;

    return ret;
#elif defined(HAVE_PUTENV)
    int ret = 0;
    char *buf;

    buf = (char *)malloc(strlen(name) + 1 + strlen(val) + 1);
    if (buf == NULL) {
        ret = ENOMEM;
        goto out;
    }
    strcpy(buf, name);
    strcat(buf, "=");
    strcat(buf, val);

    if (putenv(buf) == -1)
        ret = errno;

    free(buf);

out:
    return ret;
#else
#   error "Don't know how to set an environment variable."
#endif
}

int
atf_env_unset(const char *name)
{
#if defined(HAVE_UNSETENV)
    unsetenv(name);
    return 0;
#elif defined(HAVE_PUTENV)
    int ret = 0;
    char *buf;

    buf = (char *)malloc(strlen(name) + 2);
    if (buf == NULL) {
        ret = ENOMEM;
        goto out;
    }
    strcpy(buf, name);
    strcat(buf, "=");

    if (putenv(buf) == -1)
        ret = errno;

    free(buf);

out:
    return ret;
#else
#   error "Don't know how to unset an environment variable."
#endif
}
