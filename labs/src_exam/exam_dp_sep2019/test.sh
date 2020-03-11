#!/bin/bash
# Script for testing socket final test 25-06-2019

SOURCE_DIR="source"
GCC_OUTPUT="gcc_output.txt"
TOOLS_DIR="tools"

# These names should include only alphanumeric characters and underscores
PORTFINDER="port_finder"

SERVER1="server1_sol"
SERVER1_DIR="server1"
CLIENT1="client1_sol"
CLIENT1_DIR="client1"
SERVER2="server2_sol"
SERVER2_DIR="server2"

TESTCLIENT1="client1_ref"
TESTSERVER1="server1_ref"

TEST_FILE="small_file1.txt"
CONNECTION_FILE_LOG="connections.log"

# Command to avoid infinite loops
TIMEOUT="timeout"

# Maximum allowed running time of a standard client
MAX_CLIENT_TIME=20

# Maximum allowed running time of a standard server
MAX_SERVER_TIME=60

# Maximum allowed running time in seconds of a standard client (to avoid infinite loops)
MAX_EXEC_TIME=20

# Maximum allowed allocated memory of a standard client (KB)
MAX_ALLOC_MEM=524288

# maximum time in seconds that the script will allow for servers to start
MAX_WAITING_TIME=5	# This parameter must be a positive integer

# if the folder containing the server2 is present then it tries to compile and
# test the solution
SERVER2_IS_PRESENT=false


#*******************************KILL PROCESSES***********************************************************
# Kill al the client and server processes that are still running
function killProcesses
{
    killall -9 $SERVER1 $TESTSERVER1 $CLIENT1 $TESTCLIENT1 $SERVER2 2>&1 &> /dev/null
}


#**********************************CLEANUP***************************************************************
function cleanup
{
    echo -n "Cleaning up..."

    # Kill pending processes
    killProcesses

    # Delete previously generated folders and files (if they exist)
    rm -r -f temp*  /tmp/temp* 2>&1 &> /dev/null   # temporary file names start all with temp
    rm -f $SOURCE_DIR/$SERVER1 2>&1 &> /dev/null
    rm -f $SOURCE_DIR/$CLIENT1 2>&1 &> /dev/null
    rm -f $SOURCE_DIR/$SERVER2 2>&1 &> /dev/null

    echo -e "\t\t\tOk!"
}

#***************************CHECK TESTING TOOLS**********************************************************
# check files necessary for tests are present
function checkTools
{
    echo -n "checking all necessary files are present in tools folder..."

    if [ ! -e $TOOLS_DIR/$PORTFINDER ]\
	|| [ ! -e $TOOLS_DIR/$TEST_FILE ]\
        || [ ! -e $TOOLS_DIR/$TESTSERVER1 ]\
	|| [ ! -e $TOOLS_DIR/$TESTCLIENT1 ]\
        ; then
            echo -e "\tFail!. \n[ERROR] Could not find testing tools or missing test files. Test aborted!"
            exit -1
    else
        echo -e "\tOk!"
    fi

}

#*******************************SINGLE COMPILATION*******************************************************
# Performs a single compilation command
# Arguments:
# $1: the executable name
# $2: the folder where the program to compile is
function gcc_compile
{
    gcc -std=gnu99 -o $1 $2/*.c *.c -I$2 -lpthread -lm >> $GCC_OUTPUT 2>&1
}



#********************************COMPILE SOURCES*********************************************************
# compiles all sources
function compileSource
{
    cd $SOURCE_DIR
    rm -f $GCC_OUTPUT

    echo -n "Test 0.1 (compiling $SERVER1)...";
    gcc_compile $SERVER1 $SERVER1_DIR
    if [ ! -e $SERVER1 ] ; then
        TEST_01_PASSED=false
        echo -e "\tFail! \n\t[ERROR] Unable to compile source code for $SERVER1."
        echo -e "\tGCC log is available in $SOURCE_DIR/$GCC_OUTPUT"
    else
        TEST_01_PASSED=true
        echo -e "\tOk!"
    fi

    echo -n "Test 0.2 (compiling $CLIENT1)...";
    gcc_compile $CLIENT1 $CLIENT1_DIR
    if [ ! -e $CLIENT1 ] ; then
	TEST_02_PASSED=false
	echo -e "\tFail! \n\t[ERROR] Unable to compile source code for $CLIENT1."
	echo -e "\tGCC log is available in $SOURCE_DIR/$GCC_OUTPUT"
    else
	TEST_02_PASSED=true
	echo -e "\tOk!"
    fi

    if [ -e $SERVER2_DIR ] ; then
	echo -n "Test 0.3 (compiling $SERVER2)...";
	gcc_compile $SERVER2 $SERVER2_DIR
    	if [ ! -e $SERVER2 ] ; then
        	TEST_03_PASSED=false
        	echo -e "\tFail! \n\t[ERROR] Unable to compile source code for $SERVER2."
        	echo -e "\tGCC log is available in $SOURCE_DIR/$GCC_OUTPUT"
    	else
        	TEST_03_PASSED=true
            SERVER2_IS_PRESENT=true
        	echo -e "\tOk!"
    	fi
    fi

    cd ..
}

#*************************************GET MEMORY USAGE***************************************************
# Get the total memory usage of all processes that match a given pattern
# Arguments:
# $1: the pattern of the processes to check
function getMemUsage
{
    tot_mem=0
    for f in `ps -ef | grep $1 | awk '{print $2}'`;
    do
            mem=$(pmap -x $f | tail -1 | awk '{print $3}')
            if [[ ! -z "$mem" ]] ; then
                    tot_mem=$(($tot_mem+$mem))
            fi
    done
    echo $tot_mem
}

#*************************************WATCHDOG**************************************
# Kill a process after the timeout or the memory threshold
# Arguments:
# $1: the name of the process to watch
# $2: the timeout - mandatory
# $3: the memory threshold - observed if greater than 0
# $4: the maximum number of processes - observed if greater than 0
function watchdog
{
    echo -e "started watchdog on process name: $1"
    started=false
    for (( i=0; i<=$(($2*10)); i++ ))
    do
        sleep 0.1
        num_processes=$(pgrep -c $1)
        if (($num_processes > 0)) ; then
            started=true
            # check memory size
            if (( $3 > 0 )) ; then
                # kill if memory threshold has been reached
                mem_usage=$(getMemUsage $1)
                if (( $mem_usage>$3 )) ; then
                    echo "killing $1 as it allocated $mem_usage KB of memory"
                    for f in `ps -ef | grep $1 | awk '{print $2}'`; do kill -9 $f 2>&1 &> /dev/null; done
                    killall -9 $1 2>&1 &> /dev/null
                    return
                fi
            fi
            # check number of processes
            if (( $4 > 0 )) ; then
                if (( $num_processes>$4 )) ; then
                    echo "killing $1 as it spawned $num_processes processes"
                    for f in `ps -ef | grep $1 | awk '{print $2}'`; do kill -9 $f 2>&1 &> /dev/null; done
                    killall -9 $1 2>&1 &> /dev/null
                    return
                fi
            fi
        elif [ $started = true ] ; then
            echo "watchdog: $1 has finished"
            return
        fi
    done
    echo "killing $1 as it exceded execution timeout"
    for f in `ps -ef | grep $1 | awk '{print $2}'`; do kill -9 $f 2>&1 &> /dev/null; done
    killall -9 $1 2>&1 &> /dev/null
}

#*************************************SETUP TEMP DIRECTORY FOR SERVER************************************
# Prepares the temp directory of the server
# Arguments:
# $1: the name of the server executable file
# $2: the directory to be used for the server
function setupTempServerDir
{
    # Creating empty temp directory for server
    rm -r -f $2
    mkdir $2 2>&1 &> /dev/null

    # Copying test-related files to temp directory
    cp -f $TOOLS_DIR/$PORTFINDER $2 2>&1 &> /dev/null
    cp -f $TOOLS_DIR/$TEST_FILE $2 2>&1 &> /dev/null

    # Check if we are testing the student server
    if [[ $1 == $SERVER1 ]] || [[ $1 == $SERVER2 ]]; then
        testStudentServer=true
    else
        testStudentServer=false
    fi

    if [[ $1 == $SERVER2 ]]; then
      testServer2=true
    else
      testServer2=false
    fi

    # Copying the server executable file to the temp directory
    if [[ $testStudentServer == true ]] ; then
        cp -f $SOURCE_DIR/$1 $2 2>&1 &> /dev/null
    else
        cp -f $TOOLS_DIR/$1 $2 2>&1 &> /dev/null
fi
}

#*************************************SETUP TEMP DIRECTORY FOR CLIENT************************************
# Prepares the temp directory of the client
# Arguments:
# $1: the name of the client executable file
# $2: the directory to be used for the client
function setupTempClientDir
{
    # Creating empty temp directory for client
    rm -r -f $2
    mkdir $2 2>&1 &> /dev/null


    # Check if we are testing the student client
    if [[ $1 == $CLIENT1 ]] ; then
        testStudentClient=true
    else
        testStudentClient=false
    fi

    # Copying the client executable file to the temp directory
    if [[ $testStudentClient == true ]] ; then
        cp -f $SOURCE_DIR/$1 $2 2>&1 &> /dev/null
    else
        cp -f $TOOLS_DIR/$1 $2 2>&1 &> /dev/null
    fi
}


#************************************RUN SERVER**********************************************************
# Runs the specified server in the specified directory
# Arguments:
# $1: the server to be run (including path relative to server directory)
# $2: the directory of the server to be run
# $3: the file for recording the server standard output and error
# $4...: additional parameters accepted by the server
# Return value: the listening port or -1 in case of error
function runServer
{
    pushd $2 >> /dev/null
    local FREE_PORT=`./$PORTFINDER`     # This will find a free port
    $1 $FREE_PORT "${@:4}" &> $3 &               # Launch server with free port on cmd line
    res=$(ensureServerStarted $FREE_PORT)
    if [[ $res != "0" ]] ; then
        echo "-1"
        return
    fi
    popd >> /dev/null
    echo $FREE_PORT     # Returning to the caller the port on which the server is listening
}


#************************************RUN CLIENT**********************************************************
# Runs the specified client in the current directory
# Assumption: the client accepts server name or address and port number as first params
# Arguments:
# $1: the client to be run (with path relative to client directory). Assumption: starts with "./"
# $2: the directory of the client to be run
# $3: the address or name of the server
# $4: the port number of the server
# $5...: the last parameters accepted by the client
# Global variables used by this function:
# $TIMEOUT:             timeout command
# $MAX_CLIENT_TIME:     timeout value
# $client_run:          progressive number of client run
# $test_suite:          progressive number of test suite
# $testStudentClient:   true if we are testing the student client
function runClient
{
    local cli=$1
    local cli_exe=${1:2}	# eliminate leading "./" to get client exec name only
    local cli_dir=$2
    local srv=$3
    local port=$4
    shift 4
    echo -ne "\nRunning client $cli (server $srv, port $port, next arguments: $@) ..."
    pushd $cli_dir >> /dev/null

    # start the watchdog for the client to run
    if [[ $testStudentClient == true ]] ; then
        echo -e "\nStarting watchdog for ${cli_exe} ..."
        watchdog ${cli_exe} $MAX_CLIENT_TIME $MAX_ALLOC_MEM 0 &
        wd_pid=$!
    fi

    # run the client
    $TIMEOUT $MAX_CLIENT_TIME $cli "$srv" "$port" ${@} &> output_tmp${cli_exe} &

    # wait for client to finish
    wait $!
    # get return value
    rc=$?
    # get output
    #read output < output_tmp${cli_exe}
    #rm output_tmp${cli_exe}

    # kill the watchdog
    if [[ $testStudentClient == true ]] ; then
        echo "killing watchdog"
        kill -9 $wd_pid
	      wait $wd_pid 2>&1 &> /dev/null 	# just to suppress "Killed" message
    fi

    echo -e "\nExecution of client $cli completed."

    # perform test
    client_run=$(($client_run + 1 ))
    # store std out and return value
    #outputname=${cli_exe}"output$test_suite$client_run"
    #eval ${outputname}="'$output'"
    mv output_tmp${cli_exe} ${cli_exe}output${test_suite}${client_run}.txt
    returnname=${cli_exe}"return$test_suite$client_run"
    eval ${returnname}="'$rc'"

    if [[ $testStudentClient == false ]] ; then
      # Store port used by the client
      portname=${cli_exe}"port$test_suite$client_run"
      port_value="$(<port.txt)"
      eval ${portname}="'$port_value'"
    fi

    # give message about exit code of client
    if [[ $testStudentClient == false ]] && (( $rc == 1 )) ; then
        echo -e "\t$cli could not connect to $srv at port $port"
    elif [[ $testStudentClient == false ]] && (( $rc == 2 )) ; then
        echo -e "\tBad response received from $srv (or timeout)"
    elif [[ $testStudentClient == false ]] && (( $rc == 3 )) ; then
	    echo -e "\tError code received from $srv"
    elif [[ $testStudentClient == false ]] && (( $rc == 4 )) ; then
	    echo -e "\tTimeout while receiving file from $srv"
    elif [[ $testStudentClient == false ]] && (( $rc == 5 )) ; then
        echo -e "\tTest client run terminated successfully"
    else
        echo -e "\tClient run terminated with exit code $rc"
    fi

    popd >> /dev/null
}


#************************************ENSURE SERVER STARTED***********************************************
# Check a server is listening on the specified port. Wait for at most MAX_WAITING_TIME seconds
# (hardcoded in the script)
# Arguments:
# $1: the port to be checked
#
# Returns 0 for success, -1 otherwise.
# Global variables used by this function:
# $MAX_WAITING_TIME: max waiting time for server to start

function ensureServerStarted
{
	# Ensure the server has started
	for (( i=1; i<=$MAX_WAITING_TIME; i++ ))	# Maximum MAX_WAITING_TIME tries
	do
		#fuser $1/tcp &> /dev/null
		rm -f temp
		netstat -ln | grep "tcp" | grep ":$1 " > temp
		if [[ -s temp ]] ; then
			# Server started
			rm -f temp
			echo "0"
			return
		else
			# Server didn't start
			sleep 1
		fi
	done
	rm -f temp
	echo "-1"
}

#*************************************TEST CLIENT-SERVER INTERACTION*************************************
# Runs the specified server and client and performs a number of tests
# Arguments:
# $1: server to be run (name of executable file)
# $2: client to be run (name of executable file)
# $3: flag to indicate if the test suite is the 3bis or not
# $4: additional server parameter
#
function testClientServerInteraction
{
    local is_addititional_test=$3

    # Setup temp directory for server
    server_dir="/tmp/temp_server_dir_$$_$test_suite$(($client_run+1))"     # we append pid ($$) to fixed dir name
    setupTempServerDir "$1" "$server_dir"
    # Setup temp directory for client
    client_dir="/tmp/temp_client_dir_$$_$test_suite$(($client_run+1))"
    setupTempClientDir "$2" "$client_dir"

    # Run server
    echo -e "Running server $1 ...\n\n"
    server_port=$(runServer "./$1" "$server_dir" "$1output$test_suite.txt" "${@:4}")
    if [[ $server_port -eq "-1" ]] ; then
        echo -e "Server did not start."
        echo -e "Skipping the next tests of this part"
        skipping=1
        return
    else
        echo -e "\t $1 [PORT $server_port] Ok!"
    fi

    # Run client with small file
    local fname=$TEST_FILE
    runClient "./$2" "$client_dir" "127.0.0.1" "$server_port" "$fname"

    # TEST return value of test client (>=5)
    # TEST 1.1/3.1
    if [[ $testStudentClient == false ]] ; then
         testClientReturnValue $2 "5" "-ge"
         if [[ $result == "failed" ]] ; then
           echo -e "\tSkipping the next tests of this part"
           skipping=1
           return
         fi
    fi

    # TEST the requested file is present in the client dir and is identical to the expected one
    # TEST 1.2/2.1/3.2
    testFilesEquality "$server_dir" "$client_dir" "$fname"


    if [[ $testStudentServer == true ]] && [[ $testServer2 == true ]]; then
      if [[ $is_addititional_test == false ]]; then
        # TEST check if connction file log exists and contains the exact number of lines == 3
        # TEST 3.7
        testConnectionFilePresent "$server_dir" $CONNECTION_FILE_LOG 1
        # TEST check if connction file log exists and contains the same client ports used before
        # TEST 3.8
        testConnectionFileClients "$server_dir" $CONNECTION_FILE_LOG "$2"
      else
        sleep 10
        # TEST check if connction file log exists and contains the exact number of lines == 3
        # TEST 3.20
        testConnectionFilePresent2 "$server_dir" $CONNECTION_FILE_LOG 4 5

      fi
    fi

    killProcesses
}

#************************************Test correctness of connection file*************************************
# Check if connction file log exists and contains the exact number of line
# Stores result ("passed" or "failed") in global var $result
# Arguments:
# $1: the server directory
# $2: the name of the file to test
# $3: the expected number of lines in the file
#
function testConnectionFilePresent
{
  local expected_lines=$3
  test_number=$(($test_number + 1))
  echo -e "\n\n[TEST $test_suite.$test_number] Checking if connction file log exists and contains the exact number of line"
  local tname="TEST_$test_suite$test_number"
  local tname+="_PASSED"

  if [[ -f "$1/$2" ]]; then
    echo "File $2 exists"
  else
    echo -e "\n\t[--TEST $test_suite.$test_number FAILED--] File $2 does NOT exist in the server directory."
    eval ${tname}=false
    result=failed
    return
  fi

  # Let's count the number of lines in the file
  lines="$(wc -l "$1/$2" | awk '{ print $1 }')"

  if [[ "$lines" -eq "$expected_lines" ]]; then
    echo -e "\n\t[++TEST $test_suite.$test_number PASSED++] Ok!"
    eval ${tname}=true
    result=passed
  else
    echo -e "\n\t[--TEST $test_suite.$test_number FAILED--] File $2 contains an unexpected number of lines: $lines."
    eval ${tname}=false
    result=failed
  fi
}

#************************************Test correctness of connection file*************************************
# Check if connction file log exists and contains the exact number of line
# Stores result ("passed" or "failed") in global var $result
# Arguments:
# $1: the server directory
# $2: the name of the file to test
# $3: the expected number of lines in the file
# $4: the second expected number of lines in the file
#
function testConnectionFilePresent2
{
  local expected_lines1=$3
  local expected_lines2=$3
  test_number=$(($test_number + 1))
  echo -e "\n\n[TEST $test_suite.$test_number] Checking if connction file log exists and contains the exact number of line"
  local tname="TEST_$test_suite$test_number"
  local tname+="_PASSED"

  if [[ -f "$1/$2" ]]; then
    echo "File $2 exists"
  else
    echo -e "\n\t[--TEST $test_suite.$test_number FAILED--] File $2 does NOT exist in the server directory."
    eval ${tname}=false
    result=failed
    return
  fi

  # Let's count the number of lines in the file
  lines="$(wc -l "$1/$2" | awk '{ print $1 }')"

  if [[ "$lines" -eq "$expected_lines1" ]] || [[ "$lines" -eq "$expected_lines2" ]]; then
    echo -e "\n\t[++TEST $test_suite.$test_number PASSED++] Ok!"
    eval ${tname}=true
    result=passed
  else
    echo -e "\n\t[--TEST $test_suite.$test_number FAILED--] File $2 contains an unexpected number of lines: $lines."
    eval ${tname}=false
    result=failed
  fi
}

#************************************Test correctness of connection file*************************************
# Check if connction file log exists and contains the same client ports used before
# Stores result ("passed" or "failed") in global var $result
# Arguments:
# $1: the server directory
# $2: the name of the file to test
# $3: the name of the server
#
function testConnectionFileClients
{
  test_number=$(($test_number + 1))
  echo -e "\n\n[TEST $test_suite.$test_number] Checking if connction file log exists and contains the same client ports used before"
  local tname="TEST_$test_suite$test_number"
  local tname+="_PASSED"

  if [[ -f "$1/$2" ]]; then
    echo "File $2 exists"
  else
    echo -e "\n\t[--TEST $test_suite.$test_number FAILED--] File $2 does NOT exist in the server directory."
    eval ${tname}=false
    result=failed
    return
  fi

  local run=1
  local var_name=$3"port${test_suite}$run"

  while true ; do
    #echo "${!var_name}"
    if grep -q "${!var_name}" "$1/$2"; then
      result=passed
    else
      result=failed
      break
    fi

    run=$(( run + 1 ))
    var_name=$3"port${test_suite}$run"

    [[ -z "${!var_name}" ]] && break
  done

  if [[ $result == "passed" ]]; then
    echo -e "\n\t[++TEST $test_suite.$test_number PASSED++] Ok!"
    eval ${tname}=true
    result=passed
  else
    echo -e "\n\t[--TEST $test_suite.$test_number FAILED--] File $2 does not contain the expected client ports."
    eval ${tname}=false
    result=failed
  fi

}

#************************************Test if timestamps are equal*************************************
# Check the two timestamps are the same
# Stores result ("passed" or "failed") in global var $result
# Arguments:
# $1: the first timestamp
# $2: the second timestamp
#
function testClientTimestamps
{
    test_number=$(($test_number + 1))
    echo -e "\n\n[TEST $test_suite.$test_number] Checking if the timestamp of the file is the same"
    local tname="TEST_$test_suite$test_number"
    local tname+="_PASSED"
    if [[ "$1" -eq "$2" ]]; then
        echo -e "\n\t[++TEST $test_suite.$test_number PASSED++] Ok!"
        eval ${tname}=true
        result=passed
    else
        echo -e "\n\t[--TEST $test_suite.$test_number FAILED--] The timestamps are different between two consecutive runs."
        eval ${tname}=false
        result=failed
    fi
}

#************************************Get return value of test client*************************************
# returns (to the standard output) the client return value
# Arguments:
# $1: the client executable
#
function getClientReturnValue
{
    retvalname="$1return$test_suite$client_run"
    echo "${!retvalname}"
}

#************************************Test return value of test client************************************
# Check the client return value is equal to the expected one
# Stores result ("passed" or "failed") in global var $result
# Arguments:
# $1: the client executable
# $2: the expected return value
# $3: if given and equal to "-ge", passes also for values higher than the expected one
#
function testClientReturnValue
{
    test_number=$(($test_number + 1))
    rc=$(getClientReturnValue $1)
    if [[ $2 == 2 ]] ; then
        echo -e "\n\n[TEST $test_suite.$test_number] Checking if the interaction between client and server has been started correctly ..."
    elif [[ $2 == 3 ]] ; then
        echo -e "\n\n[TEST $test_suite.$test_number] Checking if the interaction between client and server has been completed correctly ..."
    elif [[ $2 == 1 ]] ; then
        echo -e "\n\n[TEST $test_suite.$test_number] Checking if the client recognized the timeout ..."
    elif [[ $2 == 4 ]] ; then
        echo -e "\n\n[TEST $test_suite.$test_number] Checking if the file has been transferred correctly ..."
    elif [[ $2 == 5 ]] ; then
        echo -e "\n\n[TEST $test_suite.$test_number] Checking if the server behaved as expected ..."
    fi
    local tname="TEST_$test_suite$test_number"
    local tname+="_PASSED"
    if [[ ( $# == 2 && $rc == $2 ) || ( $# == 3 && $3 == "-ge" && $rc -ge $2 ) ]]; then
        echo -e "\t[++TEST $test_suite.$test_number PASSED++] "
        eval ${tname}=true
        result=passed
    else
        echo -e "\t[--TEST $test_suite.$test_number FAILED--]"
        eval ${tname}=false
        result=failed
    fi
}

#************************************Test file present and identical*************************************
# Check the expected file is present in the client directory and equals the expected one
# Stores result ("passed" or "failed") in global var $result
# Arguments:
# $1: the server directory
# $2: the client directory
# $3: the name of the file to test
#
function testFilesEquality
{
    test_number=$(($test_number + 1))
    echo -e "\n\n[TEST $test_suite.$test_number] Checking the transferred file exists in client directory and equals the original ones ..."
    local tname="TEST_$test_suite$test_number"
    local tname+="_PASSED"
    fequality=$(getFileEquality "$1" "$2" "$3")
    if [[ $fequality == "0" ]] ; then
        echo -e "\n\t[++TEST $test_suite.$test_number PASSED++] Ok!"
        eval ${tname}=true
        result=passed
    elif [[ $fequality == "1" ]] ; then
        echo -e "\n\t[--TEST $test_suite.$test_number FAILED--] File is not present in the client directory."
        eval ${tname}=false
        result=failed
    elif [[ $fequality == "2" ]] ; then
        echo -e "\n\t[--TEST $test_suite.$test_number FAILED--] Files are not equal."
        eval ${tname}=false
        result=failed
    fi
}

#************************************Get info about file present and identical***************************
# Test if the expected file is present in the client directory and equals the expected one
# (with same name in $TOOLS_DIR directory
# Arguments:
# $1: the server directory
# $2: the client directory
# $3: the name of the file to test
#
# Returns 0 for test success, 1 if file not present, 2 if file not equal
function getFileEquality
{
    if [ -e $2/$3 ] ; then
        diff "$1/$3" "$2/$3" 2>&1 &> /dev/null
        rc=$?
        if [ $rc -eq 0 ] ; then
            echo "0"
        else
            echo "2"
        fi
    else
        echo "1"
    fi
}


#********************************** TEST INITIALIZATION *************************************************

cleanup
checkTools
compileSource


#********************************** TEST SUITE 1 ********************************************************
echo -e "\n*** PART 1: TESTS ON THE STUDENT'S SERVER WITH A REFERENCE CLIENT *************"
test_suite=1
test_number=0
client_run=0

if [[ $TEST_01_PASSED == true ]] ; then

    testClientServerInteraction "$SERVER1" "$TESTCLIENT1" false
    killProcesses
else
	echo -e "---Skipping test of Part 1 because the student's server didn't compile---"
fi
echo -e "*** END OF TESTING PART 1 *************"


#********************************** TEST SUITE 2 ********************************************************
echo -e "\n\n*** PART 2: TESTS ON THE STUDENT'S CLIENT WITH THE STUDENT'S SERVER *************"
test_suite=2
test_number=0
client_run=0

if [[ $TEST_01_PASSED == true ]] && [[ $TEST_02_PASSED == true ]] ; then

    testClientServerInteraction "$TESTSERVER1" "$CLIENT1" false
    killProcesses
else
	echo -e "---Skipping test of Part 2 because the student's server or client didn't compile---"
fi
echo "*** END OF TESTING PART 2 *************"


#********************************** TEST SUITE 3 ********************************************************
if [ "$SERVER2_IS_PRESENT" = true ] ; then
    echo -e "\n\n*** PART 3: TESTS ON THE STUDENT'S SERVER2 WITH A REFERENCE CLIENT (20seconds) *************"
    test_suite=3
    test_number=0
    client_run=0

    if [[ $TEST_03_PASSED == true ]] ; then

        testClientServerInteraction "$SERVER2" "$TESTCLIENT1" false "20"

        killProcesses
    else
	echo -e "---Skipping test of Part 3 because the student's SERVER2 didn't compile---"
    fi
    echo "*** END OF TESTING PART 3 *************"
fi

#********************************** TEST SUITE 3bis ********************************************************
if [ "$SERVER2_IS_PRESENT" = true ] ; then
    echo -e "\n\n*** PART 3: TESTS ON THE STUDENT'S SERVER2 WITH A REFERENCE CLIENT (3seconds) *************"
    test_suite=3
    test_number=4
    client_run=0

    if [[ $TEST_03_PASSED == true ]] ; then

        testClientServerInteraction "$SERVER2" "$TESTCLIENT1" true "3"

        killProcesses
    else
	echo -e "---Skipping test of Part 3bis because the student's SERVER2 didn't compile---"
    fi
    echo "*** END OF TESTING PART 3bis *************"
fi

#****************************** SUMMARY *****************************************************************


# recap print
echo ""
echo " ----- Tests recap ------"
echo " - test 0.1:  "${TEST_01_PASSED:-skipped}	#prints "skipped" if variable was not set
echo " - test 0.2:  "${TEST_02_PASSED:-skipped}
echo " - test 0.3:  "${TEST_03_PASSED:-skipped}
echo ""
echo " - test 1.1:   "${TEST_11_PASSED:-skipped}
echo " - test 1.2:   "${TEST_12_PASSED:-skipped}
echo ""
echo " - test 2.1:   "${TEST_21_PASSED:-skipped}
echo ""
echo " - test 3.1:   "${TEST_31_PASSED:-skipped}
echo " - test 3.2:   "${TEST_32_PASSED:-skipped}
echo " - test 3.3:   "${TEST_33_PASSED:-skipped}
echo " - test 3.4:   "${TEST_34_PASSED:-skipped}
echo " - test 3.5:   "${TEST_35_PASSED:-skipped}
echo " - test 3.6:   "${TEST_36_PASSED:-skipped}
echo " - test 3.7:   "${TEST_37_PASSED:-skipped}

echo " ------------------------"

# Checking minimum requirements

npassed=0
nmessages=0
passed=false

if [[ $TEST_11_PASSED == true ]] ; then
	((npassed++))
elif [[ $TEST_11_PASSED == false ]]; then
	((nmessages++))
	echo -e "\n \t The student's server could not be contacted or did not respond correctly"
fi

if [[ $TEST_12_PASSED == true ]]; then
	((npassed++))
elif [[ $TEST_12_PASSED == false ]]; then
	((nmessages++))
	echo -e "\n \t The student's server failed to deliver the file as expected"
fi

if [[ $TEST_21_PASSED == true ]]; then
	((npassed++))
elif [[ $TEST_21_PASSED == false ]]; then
	((nmessages++))
	echo -e "\n \t The interaction between the student's client and server did not work correctly (files are not the same)"
fi

if [[ $TEST_31_PASSED == true ]]; then
	((npassed++))
elif [[ $TEST_31_PASSED == false ]]; then
	((nmessages++))
	echo -e "\n \t The interaction between the reference client and the server2 did not work correctly (connection time 20s)"
fi

if [[ $TEST_32_PASSED == true ]]; then
	((npassed++))
elif [[ $TEST_32_PASSED == false ]]; then
	((nmessages++))
	echo -e "\n \t The student's server2 failed to deliver the file as expected (connection time 20s)"
fi

if [[ $TEST_33_PASSED == true ]]; then
	((npassed++))
elif [[ $TEST_33_PASSED == false ]]; then
	((nmessages++))
	echo -e "\n \t The connections log file in the server2 does not exist or is not correctly formatted"
fi

if [[ $TEST_34_PASSED == true ]]; then
	((npassed++))
elif [[ $TEST_34_PASSED == false ]]; then
	((nmessages++))
	echo -e "\n \t The connections log file in the server2 does not exist or does not contain the right clients' port"
fi

if [[ $TEST_35_PASSED == true ]]; then
	((npassed++))
elif [[ $TEST_35_PASSED == false ]]; then
	((nmessages++))
	echo -e "\n \t The interaction between the reference client and the server2 did not work correctly (connection time 3s)"
fi

if [[ $TEST_36_PASSED == true ]]; then
	((npassed++))
elif [[ $TEST_36_PASSED == false ]]; then
	((nmessages++))
	echo -e "\n \t The student's server2 failed to deliver the file as expected (connection time 3s)"
fi

if [[ $TEST_37_PASSED == true ]]; then
	((npassed++))
elif [[ $TEST_37_PASSED == false ]]; then
	((nmessages++))
	echo -e "\n \t The connections log file in the server2 does not exist or is not correctly formatted"
fi

if [[ $TEST_11_PASSED == true && $TEST_12_PASSED == true ]] ; then
	passed=true
else
	passed=false
fi

if [[ $passed == false ]] ; then
	echo -e "\n----- FAIL: You have not met the minimum requirements for submission!!! -----\n"
else
	echo -e "\n+++++ OK: You may have met the minimum requirements for submission!!! +++++\n"
fi

if [[ "$nmessages" -ge 1 ]] ; then
	echo -e "\n### Fix the reported errors to meet the minimum requirements ###\n "
fi

#**************************************** CLEANUP *******************************************************
cleanup

#**************************************** EXIT *******************************************************
if [[ $passed == "true" ]] ; then
	exit 0
else
	exit 1
fi
