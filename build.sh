#!/bin/bash

if [[ "$1" == "-w" || "$1" == "--web" ]]; then
	if [[ ! -d "buildweb" ]]; then
		echo "ERROR: buildweb directory not found! Set up with ./build-init.sh --web"
		exit 1
	fi

	cd buildweb
else
	if [[ ! -d "build" ]]; then
		echo "ERROR: build directory not found! Set up with ./build-init.sh"
		exit 1
	fi

	cd build
fi

ninja
