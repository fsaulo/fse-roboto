MMCU    = atmega328p
FOSC	= 115200

AVR_GCC = avr-gcc
AVR_OBJ = avr-objcopy
GCC_ARG = -Wall -Os -mmcu=$(MMCU) -g
OBJ_ARG = -O ihex -R .eeprom
NAME    = fseroboto
STDLIB  =

LIBDIR  = lib
OBJDIR  = build
SRCDIR  = src
BINDIR  = bin

AVRDUDE = avrdude
DUDEARG = -b $(FOSC) -c arduino -D -p $(MMCU)
DEVPORT = /dev/ttyACM0
OBJHEX	= $(NAME).hex

HEADER  = serial.h
OBJ     = fseroboto.o serial.o
DEPS    = $(patsubst %, $(SRCDIR)/%,$(HEADER))
BIN     = $(patsubst %, $(OBJDIR)/%,$(OBJ))
HEX		= $(patsubst %, $(OBJDIR)/%,$(OBJHEX))

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(AVR_GCC) -c -o $@ $< $(GCC_ARG)
	
$(BINDIR)/$(NAME).elf: $(BIN)
	$(AVR_GCC) -o $@ $^ $(GCC_ARG) $(STDLIB)
	
$(OBJDIR)/$(NAME).hex: $(BINDIR)/$(NAME).elf
	$(AVR_OBJ) $(OBJ_ARG) $< $@

.PHONY: clean

all: $(DEPS) $(BINDIR) $(HEX)

hex: $(HEX)

clean:
	@rm -f $(OBJDIR)/*.o *~ core $(SRCDIR)/*~
	@rm -f $(BINDIR)/*.elf
	@rm -f $(OBJDIR)/*.hex
	
burn: $(HEX)
	$(AVRDUDE) $(DUDEARG) -P $(DEVPORT) -U flash:w:$(OBJDIR)/$(OBJHEX)
