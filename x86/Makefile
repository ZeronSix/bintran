all:
		nasm -f elf64 x86commands.asm -l x86commands.lst
test:
		../bin/zasm factorial.zas zvm.zo
		../bin/bintran zvm.zo
