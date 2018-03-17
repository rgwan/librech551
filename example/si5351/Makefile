si5351.bin: si5351.ihx
	objcopy -I ihex -O binary si5351.ihx si5351.bin

si5351.ihx: si5351.c i2c.c si5351.h
	sdcc -I. -mmcs51 --model-small --xram-size 1024 --opt-code-speed -c i2c.c
	sdcc -I. -mmcs51 --model-small --xram-size 1024 --opt-code-speed si5351.c i2c.rel

clean:
	rm -f *.asm *.ihx *.lk *.lst *.bin *.map *.mem *.rel *.rst *.sym
	
flash: si5351.bin
	wchisptool -f si5351.bin -g
