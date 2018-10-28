#!/bin/bash

if [ ! -n "$WORK_DIR" ]; then
	LDD3_WORK_DIR=~/work/ldd3
fi
echo "LDD3_WORK_DIR=$LDD3_WORK_DIR"
echo "export PATH=\$PATH:$LDD3_WORK_DIR/tools" >> ~/.bashrc
