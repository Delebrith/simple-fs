#!/bin/bash

if ls daemon 1> /dev/null 2>&1;
then
	if ls tests/auto/* 1> /dev/null 2>&1; 
	then
		for filename in `ls tests/auto`
		do
			echo "Running $filename..."

			./daemon 1> /dev/null &
			daemon_pid=$!
			sleep 1

			echo "Started daemon"

			./tests/auto/$filename
	
			result=$?
			if [[ $result -eq 0 ]];
			then
				echo -e "\e[32m>>>>>>>>[OK]\e[0m"
			elif [[ $result -eq -1 ]];
			then
				echo -e "\e[43m>>>>>>>>[FAILED]\e[0m"
			else
				echo -e "\e[31m>>>>>>>>[ERROR]\e[0m"
			fi

			kill $daemon_pid
		done
	else
		echo "No test files in tests/auto/"
	fi
else
	echo "Daemon not found"
fi
