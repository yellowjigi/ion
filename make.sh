#!/bin/bash

make
wait
make install
wait
ldconfig
wait
