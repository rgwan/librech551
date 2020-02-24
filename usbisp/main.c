#ifdef _MSC_VER
#include "..\msvc\stdafx.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#ifdef _MSC_VER
// MSVC

#include "..\msvc\libusb\include\libusb-1.0\libusb.h"
#ifdef _WIN64
#pragma comment(lib, "libusb\\MS64\\dll\\libusb-1.0.lib")
#else
#pragma comment(lib, "libusb\\MS32\\dll\\libusb-1.0.lib")
#endif

// POSIX getopt()
#include "..\msvc\getopt.h"
#else
// Linux GCC
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#endif

#define VENDOR_ID	0x4348
#define PRODUCT_ID	0x55e0 /* CH551 Bootloader VID-PID pair */



static struct libusb_device_handle *devh = NULL;


static int ep_in_addr  = 0x82;
static int ep_out_addr = 0x02;

/* 命令格式：操作-1byte|长度-1byte|数据 */

static char detect_chip_cmd_v1[64] = {0xa2, 0x13, 0x55, 0x53, 0x42, 0x20, 0x44, 0x42,
				   0x47, 0x20, 0x43, 0x48, 0x35, 0x35, 0x39, 0x20,
				   0x26, 0x20, 0x49, 0x53, 0x50, 0x00}; /* 检测型号 */

static char use_interface_cmd_v1[2] = {0xbb, 0x00}; /* 使用IF */

static char erase_page_cmd_v1[4] = {0xa9, 0x02, 0x00, 0x00}; /* 1KB/Page? */

static char key_input_cmd_v1[6] = {0xa6, 0x04, 0x00, 0x00, 0x00, 0x00};

static char run_cmd_v1[4] = {0xa5, 0x02, 0x01, 0x00};

static char write_cmd_v1[64] = {0xa8, 0x00, 0x00, 0x00}; /* cmd, length, addrl, addrh, data */
static char verify_cmd_v1[64] = {0xa7, 0x00, 0x00, 0x00}; /* cmd, length, addrl, addrh, data */

static char read_df_cmd_v1[2] = {0xb7, 0x00};
static char claim_device_cmd_v1[4] = {0xa5, 0x02, 0x00, 0x00};
static char erase_df_cmd_v1[4] ={0xb5, 0x02, 0x00, 0xc0};
static char write_df_cmd_v1[64] = {0xb6, 0x00, 0x00, 0x00}; /* cmd, length, addrl, addrh, data */

static char detect_chip_cmd_v2[] = { //cmd, len, ?, model, series?
				0xa1, 0x12, 0x00, 0x52, 0x11, 0x4d, 0x43, 0x55, 0x20, 0x49, 0x53,
				0x50, 0x20, 0x26, 0x20, 0x57, 0x43, 0x48, 0x2e, 0x43, 0x4e
									};
static char read_cfg_cmd_v2[] = {
				0xa7, 0x02, 0x00, 0x1f, 0x00
};
static char erase_chip_cmd_v2[] = { //2KB per page?
				0xa4, 0x01, 0x00, 0x08
};
static char write_cmd_v2[64] = {0xa5, 0x00, 0x00, 0x00}; /* cmd, length, zero, addrl,
															addrh, z,    z,    random, data */

static char verify_cmd_v2[64] = {0xa6, 0x00, 0x00, 0x00}; /* cmd, length, zero, addrl, */


int write_to_device(unsigned char *data, int size)
{
	int actual_length;
	int rc = libusb_bulk_transfer(devh, ep_out_addr, data, size,
							 &actual_length, 4000);
	if (rc < 0) {
		fprintf(stderr, "Error while bulking out: %s\n",  libusb_error_name(rc));
	}
	return actual_length;
}

int read_from_device(unsigned char *data, int size)
{
	int actual_length;
	int rc = libusb_bulk_transfer(devh, ep_in_addr, data, size, &actual_length,
								  400000);
	if (rc == LIBUSB_ERROR_TIMEOUT) {
		printf("timeout (%d)\n", actual_length);
		return -1;
	} else if (rc < 0) {
		fprintf(stderr, "Error while bulking in: %s\n",  libusb_error_name(rc));
		return -1;
	}

	return actual_length;
}

void hexdump(unsigned char *data, int size)
{
	int i;
	for(i = 0; i < size; i++)
	{
		printf("0x%02X ", data[i]);
		if((i + 1) % 8 == 0)
			putchar('\n');
	}
	putchar('\n');
}

void printusage(char *selfname)
{
	printf("Usage %s: [OPTIONS]\n", selfname);
	printf("\t-f <binary_file>\tSpecify your program file to be flash to the chip\n");
	printf("\t-d <binary_file>\tSpecify data flash file to be flash to the chip\n");
	printf("\t-D <binary_file>\tSpecify data flash file to be read from the chip\n");
	printf("\t-e\t\t\tErase chip\n");
	printf("\t-w\t\t\tErase data flash\n");
	printf("\t-h\t\t\tShow this message and exit\n");
	printf("\t-g\t\t\tRun your program after the device has been programmed (Not supported in v2 mode)\n");
	printf("\t-n\t\t\tUse new ISP protocol for BL version > 2.30, only tested on CH552T\n\n");
	printf("Copyright 2018\tZhiyuan Wan, GPL v3 License, all operations must to be based on the basic law, 0..0\n");
}

uint8_t lastkey;

void key_input_v2()
{
	uint8_t inbuf[64];
	uint8_t outbuf[64] = {0xa3, 0x30, 0x00};
	int i;
    write_to_device(read_cfg_cmd_v2, sizeof(read_cfg_cmd_v2));
	read_from_device(inbuf, 30);
	uint8_t checksum = inbuf[22];
	checksum += inbuf[23];
	checksum += inbuf[24];
	checksum += inbuf[25];

	for(i = 0; i < 0x30; i++)
		outbuf[i + 3] = checksum ^ 0; //给全零密钥

	lastkey = checksum ^ 0 + 0x52; //坑爹协议

    write_to_device(outbuf, 0x30 + 3);
    read_from_device(inbuf, 6); //TODO

}

int main(int argc, char **argv)
{
	int rc;
	int i;

	char inbuffer[64];

	char *file_buffer = 0;

	FILE *fp = 0;
	int file_length;

	char file_name[255];

	int require_download = 0;
	int require_exec = 0;
	int require_erase = 0;
	int require_dataflash_read = 0;
	int require_dataflash_write = 0;
	int require_dataflash_erase = 0;
	int require_protocol_v2 = 0;

	char df_read_file_name[255];
	char df_write_file_name[255];

	int device_flash_size = 0;
	int device_dataflash_size = 0;
	char ch;

	while ((ch = getopt(argc,argv,"f:gewnhD:d:")) != -1)
	{
		switch(ch)
		{
			case 'f':/* 指定文件 */
				strncpy(file_name, optarg, 255);
				require_download = 1;
				break;
			case 'g':/* 下载后执行 */
				require_exec = 1;
				break;
			case 'e':/* 仅擦除芯片 */
				require_erase = 1;
				break;
			case 'D':/* 读DataFlash */
				require_dataflash_read = 1;
				strncpy(df_read_file_name, optarg, 255);
				break;
			case 'd':/* 写DataFlash */
				require_dataflash_write = 1;
				strncpy(df_write_file_name, optarg, 255);
				break;
			case 'w':/* 擦DataFlash */
				require_dataflash_erase = 1;
				break;
			case 'n':
				require_protocol_v2 = 1;
				break;
			default:
				printusage(argv[0]);
				exit(0);
				break;

		}
	}

	/* Initialize libusb
	 */
	rc = libusb_init(NULL);
	if (rc < 0) {
		fprintf(stderr, "Error initializing libusb: %s\n", libusb_error_name(rc));
		exit(-1);
	}

	/* Set debugging output to max level.
	 */
	libusb_set_debug(NULL, 3);

	/* Look for a specific device and open it.
	 */
	devh = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
	if (!devh) {
		fprintf(stderr, "Can't find any suitable USB device!\n");
		goto out;
	}

	libusb_claim_interface(devh, 0);

	if(require_protocol_v2)
	{
		write_to_device(detect_chip_cmd_v2, sizeof(detect_chip_cmd_v2));
		read_from_device(inbuffer, 6); //TODO: 其他支持
		if(require_exec || require_dataflash_erase || require_dataflash_write || require_dataflash_read)
		{
			printf("Those feature currently not supported");
			exit(-1);
		}
	}
	else
	{
		write_to_device(detect_chip_cmd_v1, 63);
		read_from_device(inbuffer, 2);
		//hexdump(inbuffer, 2);
		printf("Libre CH551 Flasher 2018\n");

		if(inbuffer[0] == 0x51)
		{
			printf("Detected device CH551\n");
			device_flash_size = 10240;
			device_dataflash_size = 128;
		}
		else if(inbuffer[0] == 0x52)
		{
			printf("Detected device CH552\n");
			device_flash_size = 16384;
			device_dataflash_size = 128;
		}
		else if(inbuffer[0] == 0x53)
		{
			printf("Detected device CH553\n");
			device_flash_size = 10240;
			device_dataflash_size = 128;
		}
		else if(inbuffer[0] == 0x54)
		{
			printf("Detected device CH554\n");
			device_flash_size = 14336;
			device_dataflash_size = 128;
		}
		else if(inbuffer[0] == 0x59)
		{
			printf("Detected device CH559\n");
			device_flash_size = 61440;
			device_dataflash_size = 1024;
		}
		else
		{
			fprintf(stderr, "The chip id 0x%02X is currently not support in this program\n", inbuffer[0]);
			goto out;
		}
		printf("Device rom capacity %d, data flash capacity %d\n",
			device_flash_size, device_dataflash_size);

	}

	if(require_download)
	{
		fp = fopen(file_name, "rb");
		if(!fp)
		{
			fprintf(stderr, "Can't open file '%s' !\n", file_name);
			exit(1);
		}

		fseek(fp, 0, SEEK_END);
		file_length = ftell(fp);
		file_buffer = malloc(file_length);

		if(file_length > device_flash_size)
		{
			fprintf(stderr, "File size exceeded device's capacity!\n");
			exit(1);
		}

		if(file_buffer == 0)
		{
			fprintf(stderr, "Can't allocate memory!\n");
			exit(1);
		}
		printf("Binary length %d\n", file_length);

		fseek(fp, 0, SEEK_SET);

		fread(file_buffer, 1, file_length, fp);

		fclose(fp);
		fp = 0;
	}

	if(require_protocol_v2)
	{
		key_input_v2();
	}
	else
	{
		write_to_device(use_interface_cmd_v1, 2);
		read_from_device(inbuffer, 2);
		printf("Device bootloader version: %d.%d\n\n", inbuffer[0] >> 4, inbuffer[0] & 0xf);
		//hexdump(inbuffer, 2);

		write_to_device(key_input_cmd_v1, 6); /* Input a dummy key that let we doesn't need to 'encrypt' */
		read_from_device(inbuffer, 2);
		//hexdump(inbuffer, 2);
	}

	if(require_erase || require_download)
	{
		printf("Now performing erase...\n");
		if(require_protocol_v2)
		{
			write_to_device(erase_chip_cmd_v2, sizeof(erase_chip_cmd_v2));
			read_from_device(inbuffer, 6);
		}
		else
		{
			for(i = 0; i < (device_flash_size / 1024); i++)
			{
				erase_page_cmd_v1[3] = i * 4;
				write_to_device(erase_page_cmd_v1, 4);
				read_from_device(inbuffer, 2);
				//hexdump(erase_page_cmd_v1, 4);
				if(inbuffer[0] != 0x00)
				{
					fprintf(stderr, "Erase failed!\n");
					goto out;
				}
			}
		}
		printf("Erase done\n");
	}

	if(require_download)
	{
		i = file_length;

		int curr_addr = 0;
		int pkt_length;

		printf("Writing Flash\n");
		if(require_protocol_v2)
		{
			while(curr_addr < file_length)
			{
				pkt_length = i >= 0x38? 0x38: i;
				write_cmd_v2[1] = pkt_length + 5;
				write_cmd_v2[3] = curr_addr & 0xff;
				write_cmd_v2[4] = (curr_addr >> 8) & 0xff;
				write_cmd_v2[7] = i;

				memcpy(write_cmd_v2 + 8, file_buffer + curr_addr, pkt_length);
				int j;
				for(j = 0; j < 0x38; j++)
				{ //该死的最后一个字节的XOR加密
					if(j % 8 == 7)
						write_cmd_v2[j] ^= lastkey;
				}
				write_to_device(write_cmd_v2, 64);
				read_from_device(inbuffer, 6);
				//hexdump(inbuffer, 2);

				//printf("Write to addr %d, pkt_length %d\n", curr_addr, pkt_length);

				curr_addr += pkt_length;
				i -= pkt_length;

				if(inbuffer[4] != 0x00)
				{
					fprintf(stderr, "Write failed!\n");
					goto out;
				}

			}
		}
		else
		{
			while(curr_addr < file_length)
			{
				pkt_length = i >= 0x3c? 0x3c: i;
				write_cmd_v1[1] = pkt_length;
				write_cmd_v1[2] = curr_addr & 0xff;
				write_cmd_v1[3] = (curr_addr >> 8) & 0xff;

				memcpy(write_cmd_v1 + 4, file_buffer + curr_addr, pkt_length);
				write_to_device(write_cmd_v1, 64);
				read_from_device(inbuffer, 2);
				//hexdump(inbuffer, 2);

				//printf("Write to addr %d, pkt_length %d\n", curr_addr, pkt_length);

				curr_addr += pkt_length;
				i -= pkt_length;

				if(inbuffer[0] != 0x00)
				{
					fprintf(stderr, "Write failed!\n");
					goto out;
				}

			}
		}

		printf("Write done\n");

		i = file_length;

		curr_addr = 0;

		printf("Verifying Flash\n");
		if(require_protocol_v2)
		{
			while(curr_addr < file_length)
			{
				pkt_length = i >= 0x38? 0x38: i;
				verify_cmd_v2[1] = pkt_length + 5;
				verify_cmd_v2[3] = curr_addr & 0xff;
				verify_cmd_v2[4] = (curr_addr >> 8) & 0xff;
				verify_cmd_v2[7] = i;

				memcpy(verify_cmd_v2 + 8, file_buffer + curr_addr, pkt_length);
				int j;
				for(j = 0; j < 0x38; j++)
				{ //该死的最后一个字节的XOR加密
					if(j % 8 == 7)
						verify_cmd_v2[j] ^= lastkey;
				}
				write_to_device(write_cmd_v1, 64);
				read_from_device(inbuffer, 6);
				//hexdump(inbuffer, 2);

				//printf("Write to addr %d, pkt_length %d\n", curr_addr, pkt_length);

				curr_addr += pkt_length;
				i -= pkt_length;

				if(inbuffer[4] != 0x00)
				{
					fprintf(stderr, "Write failed!\n");
					goto out;
				}

			}
		}
		else
		{
			while(curr_addr < file_length)
			{
				pkt_length = i >= 0x3c? 0x3c: i;
				verify_cmd_v1[1] = pkt_length;
				verify_cmd_v1[2] = curr_addr & 0xff;
				verify_cmd_v1[3] = (curr_addr >> 8) & 0xff;

				memcpy(verify_cmd_v1 + 4, file_buffer + curr_addr, pkt_length);
				//memset(verify_cmd_v1 + 4, 0xff, pkt_length);

				write_to_device(verify_cmd_v1, 64);
				read_from_device(inbuffer, 2);
				//hexdump(inbuffer, 2);

				//printf("Verify addr %d\n", curr_addr);

				curr_addr += pkt_length;
				i -= pkt_length;

				if(inbuffer[0] != 0x00)
				{
					fprintf(stderr, "Verify failed!\n");
					goto out;
				}

			}
		}
		printf("Verify done\n");
	}
	if(require_dataflash_read)
	{
		printf("Read from chip data flash\n");

		fp = fopen(df_read_file_name, "wb+");
		if(!fp)
		{
			fprintf(stderr, "Can't create read back file!\n");
			goto out;
		}
		write_to_device(read_df_cmd_v1, 2);

		read_from_device(inbuffer, 64);
		fwrite(inbuffer, 1, 64, fp);
		read_from_device(inbuffer, 64);
		fwrite(inbuffer, 1, 64, fp);
		fclose(fp);
		fp = 0;

		printf("Read done\n");

	}
	if(require_dataflash_erase || require_dataflash_write)
	{
		write_to_device(claim_device_cmd_v1, 4);
		read_from_device(inbuffer, 2);
		write_to_device(key_input_cmd_v1, 6); /* Input a dummy key that let we doesn't need to 'encrypt' */
		read_from_device(inbuffer, 2);

		printf("Erasing device's data flash\n");
		write_to_device(erase_df_cmd_v1, 4); /* Erase DF */
		read_from_device(inbuffer, 2);

		if(inbuffer[0] != 0x00)
		{
			fprintf(stderr, "Erase data flash failed!\n");
			goto out;
		}

		printf("Erase done\n");
	}

	if(require_dataflash_write)
	{

		fp = fopen(df_write_file_name, "rb");
		if(!fp)
		{
			fprintf(stderr, "Can't open data flash file '%s'!\n", df_write_file_name);
			goto out;
		}
		fseek(fp, 0, SEEK_END);
		file_length = ftell(fp);

		if(file_length > device_dataflash_size)
		{
			fprintf(stderr, "File size exceeded device's capacity!\n");
			goto out;
		}

		if(file_buffer) free(file_buffer);
		file_buffer = malloc(file_length);

		if(file_buffer == 0)
		{
			fprintf(stderr, "Can't allocate memory!\n");
			exit(1);
		}
		printf("Data flash Binary length %d\n", file_length);
		fseek(fp, 0, SEEK_SET);

		fread(file_buffer, 1, file_length, fp);
		fclose(fp);
		fp = 0;

		printf("Flashing device's data flash\n");

		i = file_length;

		int curr_addr = 0;
		int pkt_length;

		while(curr_addr < file_length)
		{
			pkt_length = i >= 0x3c? 0x3c: i;
			write_df_cmd_v1[1] = pkt_length;
			write_df_cmd_v1[2] = curr_addr & 0xff;
			write_df_cmd_v1[3] = (curr_addr >> 8) & 0xff;

			memcpy(write_df_cmd_v1 + 4, file_buffer + curr_addr, pkt_length);
			write_to_device(write_df_cmd_v1, 64);
			read_from_device(inbuffer, 2);
			//hexdump(inbuffer, 2);

			//printf("Write to addr %d, pkt_length %d\n", curr_addr, pkt_length);

			curr_addr += pkt_length;
			i -= pkt_length;

			if(inbuffer[0] != 0x00)
			{
				fprintf(stderr, "Write data flash failed!\n");
				goto out;
			}

		}
		printf("Write data flash done\n");
		printf("Verifing data flash\n");

		write_to_device(read_df_cmd_v1, 2);
		read_from_device(inbuffer, 64);
		if(memcmp(inbuffer, file_buffer, file_length > 64? 64: file_length) != 0)
		{
			fprintf(stderr, "Verify data flash failed!\n");
			goto out;
		}
		read_from_device(inbuffer, 64);
		if(file_length > 64)
		{
			if(memcmp(inbuffer, file_buffer + 64, file_length > 64? file_length - 64: 0) != 0)
			{
				fprintf(stderr, "Verify data flash failed!\n");
				goto out;
			}
		}
		printf("Verify data flash done\n");
	}

	if(require_exec)
	{
		printf("Let target run, keep it at Hong Kong reporter speed!\n");
		write_to_device(run_cmd_v1, 4);
	}

	printf("\nExcited! 0..0\n");

out:
	if(file_buffer)
		free(file_buffer);
	if(fp)
		fclose(fp);

	if (devh)
	{
		libusb_release_interface(devh, 0);
		libusb_close(devh);
	}
	libusb_exit(NULL);
	return rc;
}


