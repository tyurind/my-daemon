#include <stdio.h>
#include <string.h>

char *match(char c, char *s);


int main(int argc, char **argv)
{
    int i;
    // char cmd[1000]="";
    char* cmd;
    char options[1000]="";

    cmd = strrchr(argv[0],'/');
    if (cmd) {
      cmd++;
    } else {
      cmd = argv[0];
    }


    strcat(options,  cmd);


    for (i=1; i<argc; i++) {
        strcat(options,  " ");
        strcat(options,  argv[i]);
    }

    printf("%s", options);

    system(options);
    return 0;
}


char *match(char c, char *s)
{
  while(c != *s && *s) {
    s++;
  }
  return(s);
}