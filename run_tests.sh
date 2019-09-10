#!/bin/sh

TESTSDIR=tests
TESTSOUTPUTEXT=.out
TESTSEXPECTEDEXT=.exe.exp

echo "Script for run the tests in Linux."
echo "If a test doesn't display nothing, it means that has passed."
echo ""

# Obtain tests
TESTS=`ls ${TESTSDIR}/*.cpp`

for TEST in ${TESTS}; do
  # Get rid of the extension
  TEST=$(echo $TEST | cut -f 1 -d '.')

  if [ -n "$(ls ${TEST} 2> /dev/null)" ]; then
    echo "Running test ${TEST}..."
  else
    echo "Test ${TEST} not compiled or not found..."
    echo ""

    continue
  fi

  ./${TEST} > ./${TEST}${TESTSOUTPUTEXT}
  EXPECTED=`ls ./${TESTDIR}/${TEST}${TESTSEXPECTEDEXT} 2> /dev/null`

  if [ -n "${EXPECTED}" ]; then
    echo "Comparing ${TEST}${TESTSOUTPUTEXT} and ${TEST}${TESTSEXPECTEDEXT}..."
    echo "------------------------------------------------------------------------------"
    diff ${TEST}${TESTSOUTPUTEXT} ${TEST}${TESTSEXPECTEDEXT}
    echo "------------------------------------------------------------------------------"
  else
    echo "Expected file for ${TEST} not found..."
    echo ""
  fi
done
