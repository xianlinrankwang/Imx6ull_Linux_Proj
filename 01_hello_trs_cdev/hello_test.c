#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>







/* 写: ./hello_test /dev/xxx 100ask
 * 读: ./hello_test /dev/xxx
 */
int main(int argc, char **argv)
{
    int fd;
    int len;
    char buf[100];

    if(argc < 2)
    {
        printf("Usage: ./hello_test /dev/");
        return -1;
    }

    fd = open(argv[1], O_RDWR);

    if(fd < 0)
    {
        printf("open file failed!\r\n");
        return -1;
    }

    if(argc == 2)
    {
        len = read(fd, buf,100);
        buf[99] = '\0';
        printf("read str : %s\n", buf);
    }
    else if(3 == argc)
    {
        len = write(fd, argv[2], strlen(argv[2])+1);

        printf("write len : %d\n", len);
    }
    close(fd);
    return 0;



}
