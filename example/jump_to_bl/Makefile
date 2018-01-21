jump_to_bl.bin: jump_to_bl.ihx
	objcopy -I ihex -O binary jump_to_bl.ihx jump_to_bl.bin

jump_to_bl.ihx: jump_to_bl.c
	sdcc -I. -mmcs51 --model-small --opt-code-speed jump_to_bl.c

clean:
	rm -f *.asm *.ihx *.lk *.lst *.bin *.map *.mem *.rel *.rst *.sym
	
flash: jump_to_bl.bin
	wchisptool -f jump_to_bl.bin -g
