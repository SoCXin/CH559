CC := sdcc

CODELOC := 
STACKLOC := # Stack Location; IRAM? val?
XSTACKLOC := 
XRAMLOC := 0x0000
DATALOC := 
IDATALOC := 

IRAMSIZE := 256 # 256 Bytes; but 0x80 and higher is bad...
XRAMSIZE := 0x1800 # 6KB XRAM
CODESIZE := 0xF000 # ~ 61K
STACKSIZE := 

SIZEFLAGS := --iram-size $(IRAMSIZE) --xram-size $(XRAMSIZE) --code-size $(CODESIZE) --model-small
LOCFLAGS := --xram-loc $(XRAMLOC) 
CFLAGS := -V -mmcs51 $(SIZEFLAGS) --std-c11



