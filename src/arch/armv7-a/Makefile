OBJECTS = boot.o kernel.o
%.o: %.c
	arm-none-eabi-gcc -Wall -c -o $@ $<
%.o: %.s
	arm-none-eabi-as -o $@ $<
all: $(OBJECTS)
	echo Nothing