include Makefile.inc

# Memory manager a compilar: firstfit (por defecto) o buddy
# Uso: make all MM=buddy
MM ?= firstfit
MM_OBJECTS=c/memoryManagerFF.o c/memoryManagerBuddy.o
ifeq ($(MM),buddy)
GCCFLAGS += -DUSE_BUDDY
else ifneq ($(MM),firstfit)
$(error MM invalido '$(MM)'; use firstfit|buddy)
endif

KERNEL=kernel.bin
SOURCES=$(wildcard c/*.c c/drivers/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

# Marca con que MM se compilaron los objetos: si cambia, se recompilan
$(MM_OBJECTS): .mm_$(MM)

.mm_$(MM):
	rm -f .mm_*
	touch $@

clean:
	rm -rf asm/*.o c/*.o c/drivers/*.o *.o *.bin .mm_*

.PHONY: all clean