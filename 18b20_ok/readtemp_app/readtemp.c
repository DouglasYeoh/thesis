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
#define K 0.0625

unsigned char  buf1[8]={0x28,0x4a,0x62,0x11,0x05,0x00,0x00,0xb4};
unsigned char  buf2[8]={0x28,0xaa,0xde,0x10,0x05,0x00,0x00,0x7c};


int main(void)
{
    int fd = -1;
    unsigned char i;

    
    unsigned int tmp = 0;
    float res=0;
    fd = open("/dev/ds18b20_sensor", O_RDWR);
    if(fd < 0)
    {
        perror("Can't open /dev/ds18b20_sensor \n");
        exit(1);
    }
    

    for(i=0;i<100;i++)
    {
        write(fd,buf1,8);	
		read(fd, &tmp , sizeof(tmp));
    	res=tmp*K;
    	printf("TMP1: %0.2f     ",res);	
        sleep(1);
		
		write(fd,buf2,8);	
		read(fd, &tmp , sizeof(tmp));
    	res=tmp*K;
    	printf("TMP2: %0.2f\n",res);	
        sleep(1);
    }
    close(fd);
    return 0;
} 
