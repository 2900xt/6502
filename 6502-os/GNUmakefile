CL65 := cl65
CA65 := ca65

TARGET := build/OS.bin
MAP := build/OS.map
LABELS := build/OS.lbl

C_SOURCES := $(wildcard *.c)
C_OBJECTS := $(patsubst %.c,build/%.o,$(C_SOURCES))
OBJECTS := build/_start.o $(C_OBJECTS)

CFLAGS := --target none --cpu 65c02 --standard c99 -Oirs -g
ASFLAGS := --cpu 65c02 -g
LDFLAGS := --target none --cpu 65c02 --no-target-lib \
	-C link.cfg -m $(MAP) -Ln $(LABELS)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS) link.cfg | build
	$(CL65) $(LDFLAGS) -o $@ $(OBJECTS)

build/%.o: %.c | build
	$(CL65) $(CFLAGS) -c -o $@ $<

build/_start.o: src/_start.S | build
	$(CA65) $(ASFLAGS) -o $@ $<

build:
	mkdir -p $@

clean:
	rm -f build/*.o build/*.s $(TARGET) $(MAP) $(LABELS)
