#!/bin/bash
mkdir ./build -p 
cd build
cmake ../ && make -j4
