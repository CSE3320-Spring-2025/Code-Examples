#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, const char *argv[]) 
{
    int fd = access("access.c", F_OK);
    if (fd == -1) 
    {
        perror("Error: ");
    } 
    else 
    {
        printf("Found it!\n");
    }
    return 0;
}
