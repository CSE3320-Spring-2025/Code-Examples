#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

int main( int argc, char * argv[] )
{
   pid_t pid = fork( );

   if( pid == 0 )
   {
      // Iterate over the provided command and see if there is a redirect
      // operator in it.  If there is then open a pipe between 
      int i;
      for( i=1; i<argc; i++ )
      {
         if( strcmp( argv[i], ">" ) == 0 )
         {
            int fd = open( argv[i+1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
            if( fd < 0 )
            {
                perror( "Can't open output file." );
                exit( 0 );                    
            }
            dup2( fd, 1 );
            close( fd );
            
            // Trim off the > output part of the command
            argv[i] = NULL;
         }
      }
      execvp( argv[1], &argv[1] );
   }
  else if( pid > 0 )
  {
    wait( NULL );
  }
  else
  {
    perror( "Fork failed." );
  }
}
