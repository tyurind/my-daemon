#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
  // if(argc != 2 ) {
  //   printf("Вы забыли ввести свое имя.\n");
  //   exit(1);
  // }

// char *match(char c, char *s)
// {
//   while(c!=*s && *s) s++;
//   return(s);
// }

    int i;
    char* options;

    if (argc > 1) {
        for (i=1; i<argc; i++) {
            strcat(options, argv[i]);
            strcat(options, " ");
        }
    }

    printf("\n%s", options);


    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return Daemon();

    // system("php -h");
    return 0;
}

/**
 * собственно наш бесконечный цикл демона
 */
int Daemon(void) {
    char *log;
    while(1) {
        // log = "process..";
        //log = getCommand("who");
        //if(strlen(log)) { //если в онлайне кто-то есть, то пишем в лог

         writeLog("process..");

        //}

        //ждем N секунд до следующей итерации
        sleep(10);
    }
    return 0;
}
