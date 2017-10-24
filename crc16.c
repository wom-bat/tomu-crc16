/*
 * Calculate the CRC that's reported by bootloader's `c' command
 * from a file that will be transferred with Xmodem
 *
 * Copyright 2017 Peter Chubb and Data61, CSIRO
 * Released under the Gnu Public licence, version 3.0 or later.
 */
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>


#define BOOTLOADER_SIZE (16 * 1024) /* 16kB */

uint16_t crc16(uint8_t *start, uint8_t *end, uint32_t nbytes)
{
        uint16_t crc = 0x0;
        uint8_t  *data;

        for (data = start; data < end; data++)
        {
                crc  = (crc >> 8) | (crc << 8);
                crc ^= *data;
                crc ^= (crc & 0xff) >> 4;
                crc ^= crc << 12;
                crc ^= (crc & 0xff) << 5;
        }

        nbytes -= (end - start);

        while (nbytes % 128) { /* pad to XMODEM boundary with Ctrl-Z */
                crc  = (crc >> 8) | (crc << 8);
                crc ^= 26;
                crc ^= (crc & 0xff) >> 4;
                crc ^= crc << 12;
                crc ^= (crc & 0xff) << 5;
                nbytes--;
        }
        /* rest of flash erased to 0xff */
        while (nbytes-- != 0) 
        {
                crc  = (crc >> 8) | (crc << 8);
                crc ^= 0xff;
                crc ^= (crc & 0xff) >> 4;
                crc ^= crc << 12;
                crc ^= (crc & 0xff) << 5;
        }
        return crc;
}

void Usage(const char *name)
{
        fprintf(stderr, "Usage: %s file [flashsize]\n", name);
        exit(EXIT_FAILURE);
}

/* works for power-of-two boundaries */
#define ROUNDUP(x, boundary) ((x + boundary-1) & ~(boundary - 1))

int main(int ac, char **av)
{
        unsigned int len = 64 * 1024 - BOOTLOADER_SIZE;
        int fd;
        size_t sz;
        size_t pagesize = sysconf(_SC_PAGE_SIZE);
        uint16_t crc;
        char *p;

        if (ac == 3) /* crc16 file pad */
        {
                unsigned int n;
                n = strtol(av[2], &p, 0);
                if (*p != '\0') 
                        Usage(av[0]);
                len = n;
                ac--;

        }
        if (ac != 2)
                Usage(av[0]);
        
        fd = open(av[1], O_RDONLY);
        if (fd == -1) {
                perror(av[1]);
                Usage(av[0]);
        }
        sz = lseek(fd, 0, SEEK_END);
        p = mmap(NULL, ROUNDUP(sz, pagesize), PROT_READ, MAP_PRIVATE, fd, 0);
        if (p == (char *)-1) {
                perror(av[1]);
                Usage(av[0]);
        }
        crc = crc16(p, p + sz, len);
        printf("%04X\n", crc);
        return EXIT_SUCCESS;
}
