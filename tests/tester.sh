#!/bin/bash

findCompiler() {
  if [[ -f 'ic19' ]]; then
    COMPILER='ic19'
  elif [[ -f '../cmake-build-debug/bin/ic19' ]]; then
    COMPILER='../cmake-build-debug/bin/ic19'
  fi
}

findInterpreter() {
  if [[ -f 'ic19int' ]]; then
    INTERPRETER='ic19int'
  elif [[ -f '/usr/local/bin/ic19int' ]]; then
    INTERPRETER='/usr/local/bin/ic19int'
  fi
}

findTests() {
  if [[ -d 'data/' ]]; then
    TESTS='data/'
  else
    TESTS='./'
  fi
}

runTest() {
  CODE="${1}code.ifj19"
  TAC="${1}code.ifjCode19"
  STDIN="${1}stdin"
  STDOUT="${1}stdout"
  echo "======= Running test ${1} ======="
  ${COMPILER} "${CODE}" > "${TAC}"
  COMPILER_RETVAL=$?
  echo "${COMPILER_RETVAL}" | diff "${1}compiler.retVal" - > /dev/null 2>&1
  DIFF_RETVAL=$?
  if [ "x${DIFF_RETVAL}" != "x0" ]; then
	  echo "Compiler return code difference:"
		echo "${COMPILER_RETVAL}" | diff "${1}compiler.retVal" -
	fi

  if [ "x${COMPILER_RETVAL}" != "x0" ]; then
    rm "${TAC}"
    return 1
  fi

	cat "${STDIN}" | ${INTERPRETER} "${TAC}" | diff - "${STDOUT}" > /dev/null 2>&1
	DIFF_RETVAL=$?
	if [ "x${DIFF_RETVAL}" != "x0" ]; then
	  echo "Interpret stdout difference:"
		cat "${STDIN}" | ${INTERPRETER} "${TAC}" | diff - "${STDOUT}"
	fi
  echo
}

findCompiler
findInterpreter
findTests

for dir in "${TESTS}"*/ ; do
  runTest "${dir}"
done
