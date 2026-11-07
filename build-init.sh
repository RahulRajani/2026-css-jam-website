#!/bin/bash

if [[ "$1" == "-w" || "$1" == "--web" ]]; then
	if [[ -d "emsdk" ]]; then
		echo " -- Using local emsdk --"
		echo
		source emsdk/emsdk_env.sh
	fi

	if [[ -z "$EMSDK" ]]; then
		echo "ERROR: emsdk not found!"
		echo "If not installed, use ./emsdk-init.sh to download locally for this project"
		exit 1
	fi

	emcmake cmake -S . -B buildweb -G Ninja
	cp buildweb/compile_commands.json .
else
	cmake -S . -B build -G Ninja
	cp build/compile_commands.json .
fi
