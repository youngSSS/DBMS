#ifndef DBMS_INCLUDE_TEST_TEST_HPP_
#define DBMS_INCLUDE_TEST_TEST_HPP_

#include <cstdio>
#include <ctime>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>

#include "Api.hpp"
#include "TransactionLayer.hpp"
#include "LogLayer.hpp"
#include "test/TestUtils.hpp"

/******************************************************************************
 * single thread test (STT)
 ******************************************************************************/

void single_thread_test();

/******************************************************************************
 * s-lock test (SLT)
 * s-lock only test
 ******************************************************************************/

void slock_test();

/******************************************************************************
 * x-lock test (SLT)
 * x-lock only test without deadlock
 ******************************************************************************/

void xlock_test();

/******************************************************************************
 * mix-lock test (MLT)
 * mix-lock test without deadlock
 ******************************************************************************/

void mlock_test();

/******************************************************************************
 * deadlock test (DLT)
 * mix-lock test with deadlock
 ******************************************************************************/

void deadlock_test();

#endif //DBMS_INCLUDE_TEST_TEST_HPP_
