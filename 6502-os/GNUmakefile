override VASM 		:= vasm6502_oldstyle
override CROSS 		:= gcc
override LD 		:= ld

override CROSSFLAGS += 	\
	-std=c++17			\

override LDFLAGS +=		\
	-T linker.ld		\

override VASMFLAGS +=	\
	-Fbin				\
	-dotdir				\

.PHONY: boot_test
boot_test:
	$(VASM) $(VASMFLAGS) _start.S -o build/OS.bin