typedef __bit bool;

extern void i2c_init();

extern void i2c_start();

extern void i2c_stop();

extern void i2c_write(uint8_t data);

extern bool i2c_read_ack();

extern bool i2c_read_nak();

extern uint8_t i2c_read();

#define TW_READ		0x01
