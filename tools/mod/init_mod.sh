#!/bin/bash

if [[ $# < 1 ]]; then
    echo "$0 <mod_name>"
    exit 1
fi

MOD_NAME=$1

BASE_DIR=$(dirname $(readlink -f $0))
PWD=$(pwd)

cp -r $BASE_DIR/Makefile $PWD/
cp -r $BASE_DIR/tpl.c $PWD/

mv $PWD/tpl.c $PWD/$MOD_NAME.c

sed -i "s/{{ MOD_NAME }}/$MOD_NAME/g" $PWD/$MOD_NAME.c
sed -i "s/{{ MOD_NAME }}/$MOD_NAME/g" $PWD/Makefile

echo "init successful!"
