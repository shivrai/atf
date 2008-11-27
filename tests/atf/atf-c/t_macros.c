/*
 * Automated Testing Framework (atf)
 *
 * Copyright (c) 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fcntl.h>
#include <regex.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <atf-c.h>

#include "atf-c/fs.h"
#include "atf-c/tcr.h"
#include "atf-c/text.h"

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

#define CE(stm) ATF_REQUIRE(!atf_is_error(stm))

static
void
create_ctl_file(const atf_tc_t *tc, const char *name)
{
    atf_fs_path_t p;

    CE(atf_fs_path_init_fmt(&p, "%s/%s",
                            atf_tc_get_config_var(tc, "ctldir"), name));
    ATF_REQUIRE(open(atf_fs_path_cstring(&p),
                   O_CREAT | O_WRONLY | O_TRUNC, 0644) != -1);
    atf_fs_path_fini(&p);
}

static
bool
exists(const char *p)
{
    bool b;
    atf_fs_path_t pp;

    CE(atf_fs_path_init_fmt(&pp, "%s", p));
    CE(atf_fs_exists(&pp, &b));
    atf_fs_path_fini(&pp);

    return b;
}

static
void
init_config(atf_map_t *config)
{
    atf_fs_path_t cwd;

    CE(atf_map_init(config));

    CE(atf_fs_getcwd(&cwd));
    CE(atf_map_insert(config, "ctldir",
                      strdup(atf_fs_path_cstring(&cwd)), true));
    atf_fs_path_fini(&cwd);
}

static
void
run_here(const atf_tc_t *tc, atf_tcr_t *tcr)
{
    atf_fs_path_t cwd;

    CE(atf_fs_getcwd(&cwd));
    CE(atf_tc_run(tc, tcr, &cwd));
    atf_fs_path_fini(&cwd);
}

static
bool
match_reason_aux(const atf_tcr_t *tcr, const char *regex)
{
    const atf_dynstr_t *reason = atf_tcr_get_reason(tcr);
    int res;
    regex_t preg;

    printf("Looking for '%s' in '%s'\n", regex, atf_dynstr_cstring(reason));
    ATF_REQUIRE(regcomp(&preg, regex, REG_EXTENDED) == 0);

    res = regexec(&preg, atf_dynstr_cstring(reason), 0, NULL, 0);
    ATF_REQUIRE(res == 0 || res == REG_NOMATCH);

    return res == 0;
}

static
bool
match_reason(const atf_tcr_t *tcr, const char *regex, ...)
{
    va_list ap;
    atf_dynstr_t formatted;

    va_start(ap, regex);
    CE(atf_dynstr_init_ap(&formatted, regex, ap));
    va_end(ap);

    return match_reason_aux(tcr, atf_dynstr_cstring(&formatted));
}

/* ---------------------------------------------------------------------
 * Helper test cases.
 * --------------------------------------------------------------------- */

#define H_REQUIRE_DEF(id, macro) \
    ATF_TC_HEAD(h_require_ ## id, tc) \
    { \
        atf_tc_set_md_var(tc, "descr", "Helper test case"); \
    } \
    ATF_TC_BODY(h_require_ ## id, tc) \
    { \
        create_ctl_file(tc, "before"); \
        macro; \
        create_ctl_file(tc, "after"); \
    }

#define H_REQUIRE_HEAD_NAME(id) ATF_TC_HEAD_NAME(h_require_ ## id)
#define H_REQUIRE_BODY_NAME(id) ATF_TC_BODY_NAME(h_require_ ## id)
#define H_REQUIRE(id, condition) H_REQUIRE_DEF(id, ATF_REQUIRE(condition))

#define H_REQUIRE_MSG_HEAD_NAME(id) ATF_TC_HEAD_NAME(h_require_msg_ ## id)
#define H_REQUIRE_MSG_BODY_NAME(id) ATF_TC_BODY_NAME(h_require_msg_ ## id)
#define H_REQUIRE_MSG(id, condition, msg) \
    H_REQUIRE_DEF(msg_ ## id, ATF_REQUIRE_MSG(condition, msg))

#define H_REQUIRE_EQ_HEAD_NAME(id) ATF_TC_HEAD_NAME(h_require_eq_ ## id)
#define H_REQUIRE_EQ_BODY_NAME(id) ATF_TC_BODY_NAME(h_require_eq_ ## id)
#define H_REQUIRE_EQ(id, v1, v2) \
    H_REQUIRE_DEF(eq_ ## id, ATF_REQUIRE_EQ(v1, v2))

#define H_REQUIRE_STREQ_HEAD_NAME(id) ATF_TC_HEAD_NAME(h_require_streq_ ## id)
#define H_REQUIRE_STREQ_BODY_NAME(id) ATF_TC_BODY_NAME(h_require_streq_ ## id)
#define H_REQUIRE_STREQ(id, v1, v2) \
    H_REQUIRE_DEF(streq_ ## id, ATF_REQUIRE_STREQ(v1, v2))

#define H_REQUIRE_EQ_MSG_HEAD_NAME(id) \
    ATF_TC_HEAD_NAME(h_require_eq_msg_ ## id)
#define H_REQUIRE_EQ_MSG_BODY_NAME(id) \
    ATF_TC_BODY_NAME(h_require_eq_msg_ ## id)
#define H_REQUIRE_EQ_MSG(id, v1, v2, msg) \
    H_REQUIRE_DEF(eq_msg_ ## id, ATF_REQUIRE_EQ_MSG(v1, v2, msg))

#define H_REQUIRE_STREQ_MSG_HEAD_NAME(id) \
    ATF_TC_HEAD_NAME(h_require_streq_msg_ ## id)
#define H_REQUIRE_STREQ_MSG_BODY_NAME(id) \
    ATF_TC_BODY_NAME(h_require_streq_msg_ ## id)
#define H_REQUIRE_STREQ_MSG(id, v1, v2, msg) \
    H_REQUIRE_DEF(streq_msg_ ## id, ATF_REQUIRE_STREQ_MSG(v1, v2, msg))

/* ---------------------------------------------------------------------
 * Test cases for the ATF_REQUIRE and ATF_REQUIRE_MSG macros.
 * --------------------------------------------------------------------- */

H_REQUIRE(0, 0);
H_REQUIRE(1, 1);
H_REQUIRE_MSG(0, 0, "expected a false value");
H_REQUIRE_MSG(1, 1, "expected a true value");

ATF_TC(require);
ATF_TC_HEAD(require, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the ATF_REQUIRE and "
                      "ATF_REQUIRE_MSG macros");
}
ATF_TC_BODY(require, tc)
{
    struct test {
        void (*head)(atf_tc_t *);
        void (*body)(const atf_tc_t *);
        bool value;
        const char *msg;
        bool ok;
    } *t, tests[] = {
        { H_REQUIRE_HEAD_NAME(0), H_REQUIRE_BODY_NAME(0), 0,
          "0 not met", false },
        { H_REQUIRE_HEAD_NAME(1), H_REQUIRE_BODY_NAME(1), 1,
          "1 not met", true },
        { H_REQUIRE_MSG_HEAD_NAME(0), H_REQUIRE_MSG_BODY_NAME(0), 0,
          "expected a false value", false },
        { H_REQUIRE_MSG_HEAD_NAME(1), H_REQUIRE_MSG_BODY_NAME(1), 1,
          "expected a true value", true },
        { NULL, NULL, false, NULL, false }
    };

    for (t = &tests[0]; t->head != NULL; t++) {
        atf_map_t config;
        atf_tc_t tcaux;
        atf_tcr_t tcr;

        init_config(&config);

        printf("Checking with a %d value\n", t->value);

        CE(atf_tc_init(&tcaux, "h_require", t->head, t->body, NULL, &config));
        run_here(&tcaux, &tcr);
        atf_tc_fini(&tcaux);

        ATF_REQUIRE(exists("before"));
        if (t->ok) {
            ATF_REQUIRE(atf_tcr_get_state(&tcr) == atf_tcr_passed_state);
            ATF_REQUIRE(exists("after"));
        } else {
            ATF_REQUIRE(atf_tcr_get_state(&tcr) == atf_tcr_failed_state);
            ATF_REQUIRE(!exists("after"));
            ATF_REQUIRE(match_reason(&tcr, "^Line [0-9]+: %s$", t->msg));
        }

        atf_tcr_fini(&tcr);
        atf_map_fini(&config);

        ATF_REQUIRE(unlink("before") != -1);
        if (t->ok)
            ATF_REQUIRE(unlink("after") != -1);
    }
}

/* ---------------------------------------------------------------------
 * Test cases for the ATF_REQUIRE_*EQ_ macros.
 * --------------------------------------------------------------------- */

struct require_eq_test {
    void (*head)(atf_tc_t *);
    void (*body)(const atf_tc_t *);
    const char *v1;
    const char *v2;
    const char *msg;
    bool ok;
};

static
void
do_require_eq_tests(const struct require_eq_test *tests)
{
    const struct require_eq_test *t;

    for (t = &tests[0]; t->head != NULL; t++) {
        atf_map_t config;
        atf_tc_t tcaux;
        atf_tcr_t tcr;

        init_config(&config);

        printf("Checking with %s, %s and expecting %s\n", t->v1, t->v2,
               t->ok ? "true" : "false");

        CE(atf_tc_init(&tcaux, "h_require", t->head, t->body, NULL, &config));
        run_here(&tcaux, &tcr);
        atf_tc_fini(&tcaux);

        ATF_REQUIRE(exists("before"));
        if (t->ok) {
            ATF_REQUIRE(atf_tcr_get_state(&tcr) == atf_tcr_passed_state);
            ATF_REQUIRE(exists("after"));
        } else {
            ATF_REQUIRE(atf_tcr_get_state(&tcr) == atf_tcr_failed_state);
            ATF_REQUIRE(!exists("after"));
            ATF_REQUIRE(match_reason(&tcr, "^Line [0-9]+: %s$", t->msg));
        }

        atf_tcr_fini(&tcr);
        atf_map_fini(&config);

        ATF_REQUIRE(unlink("before") != -1);
        if (t->ok)
            ATF_REQUIRE(unlink("after") != -1);
    }
}

H_REQUIRE_EQ(1_1, 1, 1);
H_REQUIRE_EQ(1_2, 1, 2);
H_REQUIRE_EQ(2_1, 2, 1);
H_REQUIRE_EQ(2_2, 2, 2);
H_REQUIRE_EQ_MSG(1_1, 1, 1, "1 does not match 1");
H_REQUIRE_EQ_MSG(1_2, 1, 2, "1 does not match 2");
H_REQUIRE_EQ_MSG(2_1, 2, 1, "2 does not match 1");
H_REQUIRE_EQ_MSG(2_2, 2, 2, "2 does not match 2");

ATF_TC(require_eq);
ATF_TC_HEAD(require_eq, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the ATF_REQUIRE_EQ and "
                      "ATF_REQUIRE_EQ_MSG macros");
}
ATF_TC_BODY(require_eq, tc)
{
    struct require_eq_test tests[] = {
        { H_REQUIRE_EQ_HEAD_NAME(1_1), H_REQUIRE_EQ_BODY_NAME(1_1),
          "1", "1", "1 != 1", true },
        { H_REQUIRE_EQ_HEAD_NAME(1_2), H_REQUIRE_EQ_BODY_NAME(1_2),
          "1", "2", "1 != 2", false },
        { H_REQUIRE_EQ_HEAD_NAME(2_1), H_REQUIRE_EQ_BODY_NAME(2_1),
          "2", "1", "2 != 1", false },
        { H_REQUIRE_EQ_HEAD_NAME(2_2), H_REQUIRE_EQ_BODY_NAME(2_2),
          "2", "2", "2 != 2", true },
        { H_REQUIRE_EQ_MSG_HEAD_NAME(1_1), H_REQUIRE_EQ_MSG_BODY_NAME(1_1),
          "1", "1", "1 != 1: 1 does not match 1", true },
        { H_REQUIRE_EQ_MSG_HEAD_NAME(1_2), H_REQUIRE_EQ_MSG_BODY_NAME(1_2),
          "1", "2", "1 != 2: 1 does not match 2", false },
        { H_REQUIRE_EQ_MSG_HEAD_NAME(2_1), H_REQUIRE_EQ_MSG_BODY_NAME(2_1),
          "2", "1", "2 != 1: 2 does not match 1", false },
        { H_REQUIRE_EQ_MSG_HEAD_NAME(2_2), H_REQUIRE_EQ_MSG_BODY_NAME(2_2),
          "2", "2", "2 != 2: 2 does not match 2", true },
        { NULL, NULL, 0, 0, "", false }
    };
    do_require_eq_tests(tests);
}

H_REQUIRE_STREQ(1_1, "1", "1");
H_REQUIRE_STREQ(1_2, "1", "2");
H_REQUIRE_STREQ(2_1, "2", "1");
H_REQUIRE_STREQ(2_2, "2", "2");
H_REQUIRE_STREQ_MSG(1_1, "1", "1", "1 does not match 1");
H_REQUIRE_STREQ_MSG(1_2, "1", "2", "1 does not match 2");
H_REQUIRE_STREQ_MSG(2_1, "2", "1", "2 does not match 1");
H_REQUIRE_STREQ_MSG(2_2, "2", "2", "2 does not match 2");

ATF_TC(require_streq);
ATF_TC_HEAD(require_streq, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the ATF_REQUIRE_STREQ and "
                      "ATF_REQUIRE_STREQ_MSG macros");
}
ATF_TC_BODY(require_streq, tc)
{
    struct require_eq_test tests[] = {
        { H_REQUIRE_STREQ_HEAD_NAME(1_1), H_REQUIRE_STREQ_BODY_NAME(1_1),
          "1", "1", "\"1\" != \"1\"", true },
        { H_REQUIRE_STREQ_HEAD_NAME(1_2), H_REQUIRE_STREQ_BODY_NAME(1_2),
          "1", "2", "\"1\" != \"2\"", false },
        { H_REQUIRE_STREQ_HEAD_NAME(2_1), H_REQUIRE_STREQ_BODY_NAME(2_1),
          "2", "1", "\"2\" != \"1\"", false },
        { H_REQUIRE_STREQ_HEAD_NAME(2_2), H_REQUIRE_STREQ_BODY_NAME(2_2),
          "2", "2", "\"2\" != \"2\"", true },
        { H_REQUIRE_STREQ_MSG_HEAD_NAME(1_1),
          H_REQUIRE_STREQ_MSG_BODY_NAME(1_1),
          "1", "1", "\"1\" != \"1\": 1 does not match 1", true },
        { H_REQUIRE_STREQ_MSG_HEAD_NAME(1_2),
          H_REQUIRE_STREQ_MSG_BODY_NAME(1_2),
          "1", "2", "\"1\" != \"2\": 1 does not match 2", false },
        { H_REQUIRE_STREQ_MSG_HEAD_NAME(2_1),
          H_REQUIRE_STREQ_MSG_BODY_NAME(2_1),
          "2", "1", "\"2\" != \"1\": 2 does not match 1", false },
        { H_REQUIRE_STREQ_MSG_HEAD_NAME(2_2),
          H_REQUIRE_STREQ_MSG_BODY_NAME(2_2),
          "2", "2", "\"2\" != \"2\": 2 does not match 2", true },
        { NULL, NULL, 0, 0, "", false }
    };
    do_require_eq_tests(tests);
}

/* ---------------------------------------------------------------------
 * Main.
 * --------------------------------------------------------------------- */

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, require);
    ATF_TP_ADD_TC(tp, require_eq);
    ATF_TP_ADD_TC(tp, require_streq);

    return atf_no_error();
}
