#!/bin/bash
# Test script for the June 2017 exam call

SOURCE_DIR="source"
GCC_OUTPUT="gcc_output.txt"
TOOLS_DIR="tools"
PORTFINDER_DIR="portfinder"
PORTFINDER="port_finder"

SERVER1="socket_server1"
SERVER2="socket_server2"
CLIENT1="socket_client1"

TESTCLIENT1_DIR="testclient1"
TESTCLIENT1="test_client1"

TESTSERVER1_DIR="testserver1"
TESTSERVER1="test_server1"

SMALL_TEST_FILE="small_file.txt"
BIG_TEST_FILE="big_file.txt"

# Command to avoid infinite loops
TIMEOUT="timeout"

# Maximum allowed running time of a standard client (to avoid infinite loops)
MAX_EXEC_TIME=20

#*******************************KILL PROCESSES**********************************
function killProcesses
{
    # Kill running servers
    for f in `ps -ef | grep $SERVER1 | awk '{print $2}'`; do kill -9 $f &> /dev/null; done
    for f in `ps -ef | grep $SERVER2 | awk '{print $2}'`; do kill -9 $f &> /dev/null; done
    for f in `ps -ef | grep $TESTSERVER1 | awk '{print $2}'`; do kill -9 $f &> /dev/null; done

    # Kill running clients
    for f in `ps -ef | grep $CLIENT1 | awk '{print $2}'`; do kill -9 $f 2>&1 &> /dev/null; done
    for f in `ps -ef | grep $TESTCLIENT1 | awk '{print $2}'`; do kill -9 $f 2>&1 &> /dev/null; done
}

#*******************************COUNT PROCESSES**********************************
# Arguments:
# $1: the pattern of the process to be counted
# Return value: the number of processes that include the pattern
function countProcesses
{
	# decrement by one not to count grep process itself
	echo $((`ps -ef | grep "$1" | wc -l` - 1 ))
}

#**********************************CLEANUP***************************************
function cleanup
{
	echo -n "Cleaning up..."
	killProcesses
	# Delete previously generated folders and files (if they exist)
    rm -r -f temp*  2>&1 &> /dev/null   # temporary file names all start with temp
	rm -f $TOOLS_DIR/$PORTFINDER 2>&1 &> /dev/null
	rm -f $SOURCE_DIR/$CLIENT1 2>&1 &> /dev/null
	rm -f $SOURCE_DIR/$SERVER1 2>&1 &> /dev/null
	rm -f $SOURCE_DIR/$SERVER2 2>&1 &> /dev/null
	echo -e "\t\t\tOk!"
}

#***************************COMPILING TESTING TOOLS******************************
function compileTools
{
    # compile tools
	echo -n "Compiling testing tools..."

	gcc -g -Wno-format-security -o $TOOLS_DIR/$PORTFINDER $TOOLS_DIR/$PORTFINDER_DIR/*.c $TOOLS_DIR/*.c -lpthread -lm 2> /dev/null
    # and check files necessary for tests are present
	echo -n "and checking all necessary files are present..."

	if [ ! -e $TOOLS_DIR/$PORTFINDER ]\
		|| [ ! -e $TOOLS_DIR/$SMALL_TEST_FILE ] || [ ! -e $TOOLS_DIR/$BIG_TEST_FILE ]\
        || [ ! -e $TOOLS_DIR/$TESTSERVER1 ] || [ ! -e $TOOLS_DIR/$TESTCLIENT1 ]; then
            echo -e "\tFail!. \n[ERROR] Unable to compile testing tools or missing test files. Test aborted!"
            exit -1
    else
        echo -e "\tOk!"
    fi

}

#********************************COMPILING SOURCES******************************
function compileSource
{
	cd $SOURCE_DIR
	rm -f $GCC_OUTPUT

    echo -n "Test 0.1 (compiling server1)...";
    gcc -g -o $SERVER1 server1/*.c *.c -Iserver1 -lpthread -lm >> $GCC_OUTPUT 2>&1
    if [ ! -e $SERVER1 ] ; then
        TEST_01_PASSED=false
        echo -e "\tFail! \n\t[ERROR] Unable to compile source code for server1."
        echo -e "\tGCC log is available in $SOURCE_DIR/$GCC_OUTPUT"
    else
        TEST_01_PASSED=true
        echo -e "\tOk!"
    fi

	echo -n "Test 0.2 (compiling client1)...";
	gcc -g -o $CLIENT1 client1/*.c *.c -Iclient1 -lpthread -lm >> $GCC_OUTPUT 2>&1 
	if [ ! -e $CLIENT1 ] ; then
		TEST_02_PASSED=false
		echo -e "\tFail! \n\t[ERROR] Unable to compile source code for client1."
		echo -e "\tGCC log is available in $SOURCE_DIR/$GCC_OUTPUT"
	else
		TEST_02_PASSED=true
		echo -e "\tOk!"
	fi

	echo -n "Test 0.3 (compiling server2)...";
	gcc -g -o $SERVER2 server2/*.c *.c -Iserver2 -lpthread -lm >> $GCC_OUTPUT 2>&1 
	if [ ! -e $SERVER2 ] ; then
		TEST_03_PASSED=false
		echo -e "\tFail! \n\t[ERROR] Unable to compile source code for server2."
		echo -e "\tGCC log is available in $SOURCE_DIR/$GCC_OUTPUT"
	else
		TEST_03_PASSED=true
		echo -e "\tOk!"
	fi

	cd ..
}

#*************************************SETUP TEMP DIRECTORY FOR SERVER**************************************
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
    cp -f $TOOLS_DIR/$SMALL_TEST_FILE $2 2>&1 &> /dev/null
    cp -f $TOOLS_DIR/$BIG_TEST_FILE $2 2>&1 &> /dev/null

    # Check if we are testing the student server
    if [[ $1 == $SERVER1 ]] || [[ $1 == $SERVER2 ]] ; then
        testStudentServer=true
    else
        testStudentServer=false
    fi

    # Copying the server executable file to the temp directory
    if [[ $testStudentServer == true ]] ; then
        cp -f $SOURCE_DIR/$1 $2 2>&1 &> /dev/null
    else
        cp -f $TOOLS_DIR/$1 $2 2>&1 &> /dev/null
fi
}

#*************************************SETUP TEMP DIRECTORY FOR CLIENT**************************************
# Prepares the temp directory of the client
# Arguments:
# $1: the name of the client executable file
# $2: the directory to be used for the client
function setupTempClientDir
{
    # Creating empty temp directory for client
    rm -r -f $2
    mkdir $2 2>&1 &> /dev/null

    # Copying test-related files to temp directory
    # in this case, no file is necessary

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


#************************************RUN SERVER*************************************
# Runs the specified server in the specified directory
# Arguments:
# $1: the server to be run (including path relative to server directory)
# $2: the directory of the server to be run
# $3: the file for recording the server standard output and error
# Return value: the listening port
function runServer
{
    pushd $2 >> /dev/null
        local FREE_PORT=`./$PORTFINDER`		# This will find a free port
        $1 $FREE_PORT &> $3 &						# Launch the server
        ensureServerStarted $FREE_PORT
    popd >> /dev/null
	echo $FREE_PORT	# Returning to the caller the port on which the server is listening
}

#************************************RUN CLIENT*************************************
# Runs the specified client in the current directory
# Arguments:
# $1: the client to be run (with path relative to client directory)
# $2: the directory of the client to be run
# $3: the address or name of the server
# $4: the port number of the server
# $5: the file name argument
# $6: -t3 (optional)
# Global variables used by this function:
# $TIMEOUT:          timeout command
# $MAX_EXEC_TIME:    timeout value
# $client_run:       progressive number of client run
# $test_suite:       progressive number of test suite
# $testStudentClient:true if we are testing the student client
function runClient
{
	echo -n "Running client $1 (server $3, port $4, file $5) ..."
    pushd $2 >> /dev/null

    if [[ $1 == "$DUMMYCLIENT" ]] ; then
        local tout=$MAX_DUMMY_EXEC_TIME
    else
        local tout=$MAX_EXEC_TIME
    fi
    # run the client
    if [[ $6 == "-t3" ]] ; then
        output=$($TIMEOUT $tout $1 "$3" "$4" "$5" "$6")
        rc=$?
    else
        output=$($TIMEOUT $tout $1 "$3" "$4" "$5")
        rc=$?
    fi
    client_run=$(($client_run + 1 ))
    # store std out and return value
    outputname="testclientoutput$test_suite$client_run"
    eval ${outputname}="'$output'"
    if [[ $1 == "./$TESTCLIENTNODATA" ]] ; then
        returnname="testclient_nodatareturn"
    else
        returnname="testclientreturn$test_suite$client_run"
    fi 
    eval ${returnname}="'$rc'"
    # give message about exit code of client
    if [[ $testStudentClient == false ]] && (( $rc == 0 )) ; then
        echo -e "\t$1 could not connect to $3 at port $4"
    elif [[ $testStudentClient == false ]] && (( $rc == 1 )) ; then
	echo -e "\tBad or no response received from $3"
    elif [[ $testStudentClient == false ]] && (( $rc == 2 )) ; then
	echo -e "\tTest client run terminated successfully"
    else
        echo -e "\tClient run terminated"
    fi

    popd >> /dev/null
}


#************************************ENSURE SERVER STARTED*************************************
# Check a server is listening on the specified port. Wait for at most WAITSEC seconds (hardcoded)
# Arguments:
# $1: the port to be checked
#
function ensureServerStarted
{
	local WAITSEC=5				# Maximum waiting time in seconds
	# Ensure the server has started
	for (( i=1; i<=$WAITSEC; i++ ))	# Maximum WAITSEC tries
	do
		#fuser $1/tcp &> /dev/null
		rm -f temp
		netstat -ln | grep "tcp" | grep ":$1 " > temp
		if [[ -s temp ]] ; then
			# Server started
			break
		else
			# Server didn't start
			sleep 1
		fi
	done
	rm -f temp
}

#*************************************TEST CLIENT-SERVER INTERACTION**************************************
# Runs the specified server and client and performs a number of tests
# Arguments:
# $1: server to be run (name of executable file)
# $2: client to be run (name of executable file)
# $3: extra argument of client (optional)
#
function testClientServerInteraction
{
    # Setup temp directory for server
    setupTempServerDir "$1" "temp_server_dir"

    # Setup temp directory for client
    setupTempClientDir "$2" "temp_client_dir"

    # Run server
    echo -e "Running server $1 ...\n\n"
    server_port=$(runServer "./$1" "temp_server_dir" "$1$test_suite")
    echo -e "\t $1 [PORT $server_port] Ok!"

    # Run client with small file
    local fname=$SMALL_TEST_FILE
    runClient "./$2" "temp_client_dir" "127.0.0.1" "$server_port" "$fname" "$3"

    # TEST return value of test client
    # TEST 1.1, 3.1
	if [[ $testStudentClient == false ]] ; then
        testClientReturnValue "2"
        if [[ $result == "failed" ]] ; then
            echo -e "\tSkipping the next tests of this part"
            return
        fi
    fi

    # TEST the requested file is present in the client dir and identical to the original one
	# TEST 1.2, 2.1, 2.3, 3.2
    testFileEquality "temp_client_dir" "$fname"

    # Run client with big file
    local fname=$BIG_TEST_FILE
    runClient "./$2" "temp_client_dir" "127.0.0.1" "$server_port" "$fname"

    # TEST the requested file is present in the client dir and identical to the original one
    # TEST 1.3, 2.2, 2.4, 3.3
    testFileEquality "temp_client_dir" "$fname"

}

#************************************Get return value of test client*************************************
# returns (to the standard output) the client return value
# Arguments:
#
function getClientReturnValue
{
    retvalname="testclientreturn$test_suite$client_run"
    echo "${!retvalname}"
}


#************************************Test return value of test client*************************************
# Check the client return value is greater than or equal to the expected one
# Stores result ("passed" or "failed") in global var $result
# Arguments:
# $1: the expected return value
#
function testClientReturnValue
{
    test_number=$(($test_number + 1))
    rc=$(getClientReturnValue)
    echo -e "\n\n[TEST $test_suite.$test_number] Checking result of interaction of test client with server ..."
    local tname="TEST_$test_suite$test_number"
    local tname+="_PASSED"
    if (( $rc >= $1 )); then
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
# $1: the client directory
# $2: the expected file name
#
function testFileEquality
{
    test_number=$(($test_number + 1))
    echo -e "\n\n[TEST $test_suite.$test_number] Checking the transferred file exists in client directory and equals the original one"
    local tname="TEST_$test_suite$test_number"
    local tname+="_PASSED"
    local fequality=$(getFileEquality "$1" "$2")
    echo "DEBUG equality: $fequality"
    if [[ $fequality == "0" ]] ; then
        echo -e "\n\t[++TEST $test_suite.$test_number PASSED++] Ok!"
        eval ${tname}=true
        result=passed
    elif [[ $fequality == "1" ]] ; then
        echo -e "\n\t[--TEST $test_suite.$test_number FAILED--] File is not present in the client directory."
        eval ${tname}=false
        result=failed
    else
        echo -e "\n\t[--TEST $test_suite.$test_number FAILED--] Files are not equal."
        eval ${tname}=false
        result=failed
    fi
}

#************************************Test if file present and identical*************************************
# Test if the expected file is present in the client directory and equals the expected one
# (with same name in $TOOLS_DIR directory
# Arguments:
# $1: the client directory
# $2: the expected file name
#
# Returns 0 for test success, 1 if file not present, 2 if file not equal
function getFileEquality
{
    if [ -e $1/$2 ] ; then
        diff "$1/$2" "$TOOLS_DIR/$2" 2>&1 &> /dev/null
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



#********************************** TEST INITIALIZATION ****************************************

cleanup
compileTools
compileSource


#********************************** TEST SUITE 1 ****************************************
echo -e "\n************* TESTING PART 1 *************"
test_suite=1
test_number=0
client_run=0

if [[ $TEST_01_PASSED == true ]] ; then

	testClientServerInteraction "$SERVER1" "$TESTCLIENT1"
    killProcesses
else
	echo -e "---Skipping test of Part 1 because your server1 didn't compile---"
fi
echo -e "************* END OF TESTING PART 1 *************"

#********************************** TEST SUITE 2 ****************************************
echo -e "\n\n************* TESTING PART 2 *************"
test_suite=2
test_number=0
client_run=0

if [[ $TEST_02_PASSED == true ]] ; then

    testClientServerInteraction "$SERVER1" "$CLIENT1"
    killProcesses
    testClientServerInteraction "$TESTSERVER1" "$CLIENT1"
    killProcesses
else
	echo -e "---Skipping test of Part 2 because your client1 didn't compile---"
fi
echo "************* END OF TESTING PART 2 *************"

#********************************** TEST SUITE 3 ****************************************
echo -e "\n\n************* TESTING PART 3 *************"
test_suite=3
test_number=0
client_run=0

if [[ $TEST_03_PASSED == true ]] ; then
    testClientServerInteraction "$SERVER2" "$TESTCLIENT1" "-t3"
    killProcesses
else
	echo -e "---Skipping test of Part 3 because your server2 didn't compile---"
fi
echo "************* END OF TESTING PART 3 *************"

#****************************** SUMMARY *********************************
# Checking minimum requirements
#((testclientreturn11 == 2)) || ((testclientreturn11 >= 1) && (test 2.1 and and test 2.2 passed))
messages=0
passed=false

if [[ $TEST_11_PASSED == true ]] ; then     #testclientreturn11==2 means TEST_11_PASSED
	passed=true
else
	((messages++))
	echo -e "\n \t Your server did not interact correclty with the test client"
fi

if [[ $testclientreturn11 -ge 1 ]] && [[ $TEST_21_PASSED == true ]] && [[ $TEST_22_PASSED == true ]]; then
	passed=true
else 
	((messages++))
	echo -e "\n \t The interaction between your server1 and client1 did not work correctly"
fi
if [[ $passed == false ]] ; then 
	echo -e "\n----- FAIL: You MAY NOT have met the minimum requirements to pass the exam!!! -----\n"
else 
	echo -e "\n+++++ OK: You may have met the minimum requirements to pass the exam!!! +++++\n"
fi
if [[ "$messages" == 2 ]] ; then
	echo -e "\n### Fix at least one of the two items above to meet the minimum requirements ###\n "	
fi

#**************************************** CLEANUP ************************************
cleanup


