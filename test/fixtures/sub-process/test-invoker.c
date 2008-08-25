/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <cutter.h>
#include <gcutter.h>

#define TEST_DIR_KEY "CUTTEST_SUB_PROCESS_TEST_DIR"

void test_invoke(void);

void
test_invoke (void)
{
    CutSubProcess *sub_process;

    sub_process = cut_take_new_sub_process(g_getenv(TEST_DIR_KEY));
    cut_sub_process_run(sub_process);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
