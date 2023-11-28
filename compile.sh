#!/bin/bash

CLANG=/home/$USER/cheri/output/sdk/bin/clang
SYSROOT=/home/$USER/cheri/output/rootfs-riscv64-purecap
STUDY_ROOT=/home/$USER/study_files
SOURCE_FILES="$STUDY_ROOT/file_read_write.c"
CAP_OUTPUT_FILE=$STUDY_ROOT/cap.out
NORMAL_OUTPUT_FILE=$STUDY_ROOT/base.out
DEST_FOLDER=/root/compiled_cat/
COMPILER_LOG_FILE=~/logs/compiler.log
ARGS=$@

function compile_base_and_send(){
	print_headers
	unbuffer $CLANG -g -O2 -target riscv64-unknown-freebsd --sysroot="$SYSROOT" -fuse-ld=lld -mno-relax -march=rv64gc -mabi=lp64d -Wall -Wcheri $SOURCE_FILES  -o $NORMAL_OUTPUT_FILE 2>&1 | tee -a $COMPILER_LOG_FILE &&
	scp -P 9999 $NORMAL_OUTPUT_FILE root@localhost:$DEST_FOLDER &&
	echo "File compiled with base and sent"
}

function compile_cap_and_send(){
	print_headers
	unbuffer $CLANG -g -O2 -target riscv64-unknown-freebsd --sysroot="$SYSROOT" -fuse-ld=lld -mno-relax -march=rv64gcxcheri -mabi=l64pc128d -Wall -Wcheri $SOURCE_FILES -o $CAP_OUTPUT_FILE 2>&1 | tee -a $COMPILER_LOG_FILE &&
	scp -P 9999 $CAP_OUTPUT_FILE root@localhost:$DEST_FOLDER &&
	echo "File compiled with cap and sent"
}

function start_qemu(){
	/home/$USER/cheri/output/sdk/bin/qemu-system-riscv64cheri -M virt -m 2048 -nographic -bios bbl-riscv64cheri-virt-fw_jump.bin -kernel /home/$USER/cheri/output/rootfs-riscv64-purecap/boot/kernel/kernel -drive if=none,file=/home/$USER/cheri/output/cheribsd-riscv64-purecap.img,id=drv,format=raw -device virtio-blk-device,drive=drv -device virtio-net-device,netdev=net0 -netdev user,id=net0,hostfwd=tcp::9999-:22 -device virtio-rng-pci
}

function print_headers(){
	echo -e "-------------------------------------------------------" >> $COMPILER_LOG_FILE
	echo -e "$(date)     $0 $ARGS" >> $COMPILER_LOG_FILE
	echo -e "-------------------------------------------------------" >> $COMPILER_LOG_FILE
}

function usage(){
  echo "To compile the program use one of these flags:"
  echo "-cap to choose CHERI-RISC-V pure capability mode"
  echo "-base to choose RISC-V base mode"
  exit 1
}

function getArgs(){
  [ "$#" -lt 1 ] && usage
	case $1 in
		-r) start_qemu;;
		-cap) compile_cap_and_send;;
		-base) compile_base_and_send;;
		*) usage;;
	esac
}

getArgs $@
