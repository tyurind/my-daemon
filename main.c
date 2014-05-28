#if !defined(_GNU_SOURCE)
    #define _GNU_SOURCE
#endif


#include <stdio.h>
#include <stdlib.h>

#include "libinclude.c"


// лимит для установки максимально кол-во открытых дискрипторов
#define FD_LIMIT            1024*10

// константы для кодов завершения процесса
#define CHILD_NEED_WORK         1
#define CHILD_NEED_TERMINATE    2


//
//

int PrintLog(char *msg)
{
    printf(msg);
    WriteLog(msg);
}

/**
 * Точка входа
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int main(int argc, char* argv[]) 
{
    WriteLog("Daemon Start\n");

    pid_t parpid, sid;

    // если параметров командной строки меньше двух, то покажем как использовать демана
    if (argc != 2) {
        printf("Usage: smartqd <command>\n");
        // return -1;
        exit(1);
    }

    char *command = argv[1];


    if (exists(PID_FILE)) {
        PrintLog("Error: Процесс уже запущен\n");
        exit(1);
    }
    FILE* f = fopen(PID_FILE, "a");
    if ( ! f ) {
        PrintLog("Error: Неудалось создать файл PID\n");
        exit(1);
    } else {
        fclose(f);
    }

    parpid = fork(); // создаем дочерний процесс
    if (parpid < 0) {
        PrintLog("Error: Хуевый fork\n");
        exit(1);
    } else if (parpid != 0) {
        PrintLog("Error: Нулевой форк\n");
        exit(0);
    }

    umask(0);       // даем права на работу с фс
    sid = setsid(); // генерируем уникальный индекс процесса
    if (sid < 0) {
        exit(1);
    }
    if ( ! SetPidFile(PID_FILE) ) {
        PrintLog("Error: Неудалось создать файл PID\n");
        exit(1);
    }

    // переходим в корень диска, если мы этого не сделаем, то могут быть проблемы.
    // к примеру с размантированием дисков
    if ((chdir("/")) < 0) { //выходим в корень фс
        PrintLog("Error: Не удалось сменить коталог\n");
        exit(1);
    }

    // закрываем дискрипторы ввода/вывода/ошибок, так как нам они больше не понадобятся
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Данная функция будет осуществлять слежение за процессом
    Daemon(argv[1]);
}

/**
 * собственно наш бесконечный цикл демона
 * @param  command [description]
 * @return         
 */
int Daemon(char *command) 
{
    char *log;
    strcpy(log, "Start: ");
    strcat(log, command);
    strcat(log, "\n");

    WriteLog(log);


    while (1) {
        log = getCommand(command);
        // WriteLog("Error: Процесс уже запущен\n");
        // ждем 2 секунды до следующей итерации
         WriteLog(log);
         
        sleep(2);
    }
    return 0;
}

