#!/usr/bin/env bash
sudo losetup -Pf --show filesystem.img
sudo mount /dev/loop0p1 ./fsconst/esp
sudo mount /dev/loop0p2 ./fsconst/primary