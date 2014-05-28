#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>

int Daemon(char command[128]);
// int writeLog(char msg[256]);

int main(int argc, char *argv[])
{
    int i;
    char* command;
    pid_t parpid, sid;

    if (argc > 1) {
        for (i=1; i<argc; i++) {
            strcat(command, argv[i]);
            strcat(command, " ");
        }
    }

    printf("Запуск скрипта: ");
    // printf("\n  %s", command);


    // создаем дочерний процесс
    parpid = fork();
    if(parpid < 0) {
        printf("\nparpid < 0; Exit 1");
        exit(1);
    } else if(parpid == 0) {
        printf("\nparpid != 0; Exit 0");
        exit(0);
    }

    // даем права на работу с фс
    umask(0);

    // генерируем уникальный индекс процесса
    sid = setsid();
    if(sid < 0) {
        printf("sid < 0; Exit 0");
        exit(1);
    }

    // выходим в корень фс
    if((chdir("/")) < 0) {
        printf("Нет выхода");
        exit(1);
    }

    // закрываем доступ к стандартным потокам ввода-вывода
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // system("php -h");
    return Daemon(command);


}

/**
 * собственно наш бесконечный цикл демона
 */
int Daemon(char command[128]) {
    char *log;
    while(1) {
        system("php -v");
        // log = "process..";
        //log = getCommand("who");
        //if(strlen(log)) { //если в онлайне кто-то есть, то пишем в лог

        // writeLog("process..");

        //}

        // system(command);

        // ждем N секунд до следующей итерации
        sleep(2);
    }
    return 0;
}
