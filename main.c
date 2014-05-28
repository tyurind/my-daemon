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


#define PID_FILE "/tmp/fobia/fobiad.pid"
#define LOG_FILE "/tmp/fobia/fobiad.log"

//
//
int PrintLog(char *msg);
int main(int argc, char* argv[]);
void DestroyWorkThread();
int InitWorkThread();
int Daemon(char *command);


// 
int PrintLog(char *msg)
{
    printf(msg);
    if (!WriteLog(msg, LOG_FILE)) {
        printf("Не удалось записать в файл");
    }
}


/**
 * Точка входа
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int main(int argc, char* argv[]) 
{
    PrintLog("Daemon Start\n");

    // если параметров командной строки меньше двух, 
    // то покажем как использовать демана
    if (argc != 2) {
        printf("Usage: cmd <command>\n");
        exit(1);
    }

    char *command = argv[1];



    // System OS
    // ----------------
    if (FileExists(PID_FILE)) {
        PrintLog("Error: Процесс уже запущен\n");
        exit(1);
    }

    pid_t sid;
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
    if ( (chdir("/") ) < 0) { //выходим в корень фс
        PrintLog("Error: Не удалось сменить коталог\n");
        exit(1);
    }

    // FORK
    // ----------------
    pid_t parpid;
    parpid = fork(); // создаем дочерний процесс
    if (parpid < 0) {
        PrintLog("Error: Хуевый fork\n");
        exit(1);
    } else if (parpid != 0) {
        PrintLog("Error: Нулевой форк\n");
        exit(0);
    }


    // Start DAEMON
    // ----------------
    // закрываем дискрипторы ввода/вывода/ошибок, так как нам они больше не понадобятся
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Данная функция будет осуществлять слежение за процессом
    Daemon(argv[1]);

    exit(0);
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

    WriteLog(log, LOG_FILE);


    // while (1) {
    //     log = getCommand(command);
    //     // WriteLog("Error: Процесс уже запущен\n");
    //     // ждем 2 секунды до следующей итерации
    //     WriteLog(log, LOG_FILE);
         
    //     sleep(2);
    // }
    return 0;
}


// функция для остановки потоков и освобождения ресурсов
void DestroyWorkThread()
{
    // тут должен быть код который остановит все потоки и
    // корректно освободит ресурсы
}

// функция которая инициализирует рабочие потоки
int InitWorkThread()
{
    // код функции
    return 1;
}