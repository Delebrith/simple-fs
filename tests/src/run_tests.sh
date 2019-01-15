#!/bin/bash

if ls daemon 1> /dev/null 2>&1;
then
	if ls tests/auto/* 1> /dev/null 2>&1; 
	then
		for filename in "tests/auto/*"
		do
			echo "Running $filename..."

			./daemon &
			daemon_pid=$!
			sleep 2

			echo "Started daemon"

			./$filename
	
			result=$?
			if [[ $result -eq 0 ]];
			then
				echo ">>>>>>>>[OK]"
			elif [[ $result -eq 1 ]];
			then
				echo ">>>>>>>>[FAILED]"
			else
				echo ">>>>>>>>[ERROR]"
			fi

			kill $daemon_pid
		done
	else
		echo "No test files in tests/auto/"
	fi
else
	echo "Daemon not found"
fi
