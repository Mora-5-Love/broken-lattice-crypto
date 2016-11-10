#!/bin/bash

dim=$1

./ggh-gen-keys $dim pub.keys sec.keys
./ggh-encrypt pub.keys plain.txt enc.txt
./ggh-decrypt sec.keys enc.txt dec.txt

