#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
int main(void)
{
    int fd = -1;
    unsigned char i;
    unsigned char  buf[8];
    fd = open("/dev/ds18b20_reader", 0);
    if(fd < 0)
    {
        perror("Can't open /dev/ds18b20_reader \n");
        exit(1);
    }
    printf("ID:");
    read(fd,buf,8);
    for(i=0;i<8;i++)
    {
	printf("0x%02x ",buf[i]);
    }
    printf("\n");
    close(fd);
    return 0;
} 
