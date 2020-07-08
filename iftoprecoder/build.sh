#!/bin/bash
g++ -o iftoprecoder *.cpp
strip iftoprecoder
md5sum iftoprecoder
