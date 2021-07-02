@echo off 

set sdcc_path="C:\Program Files\SDCC"

set project_name=CH559Blink
set xram_size=0x0800
set xram_loc=0x0600
set code_size=0xEFFF
set dfreq_sys=12000000

del *.hex

%sdcc_path%\bin\sdcc -c -V -mmcs51 --model-large --xram-size %xram_size% --xram-loc %xram_loc% --code-size %code_size% -I/ -DFREQ_SYS=%dfreq_sys%  main.c

%sdcc_path%\bin\sdcc main.rel -V -mmcs51 --model-large --xram-size %xram_size% --xram-loc %xram_loc% --code-size %code_size% -I/ -DFREQ_SYS=%dfreq_sys%  -o %project_name%.ihx

%sdcc_path%\bin\packihx %project_name%.ihx > %project_name%.hex

del %project_name%.lk
del %project_name%.map
del %project_name%.mem
del %project_name%.ihx

del *.asm
del *.lst
del *.rel
del *.rst
del *.sym
