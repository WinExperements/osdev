@echo off
title Kernel debug
start gdb kernel.bin
start qemu-system-i386 -kernel kernel.bin -s -S