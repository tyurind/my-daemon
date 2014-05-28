#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>

// rlimit
// #include <sys/time.h> 
#include <sys/resource.h> 
// #include <unistd.h>

/**
 * функция возвращает форматированную дату и время
 * @return формат времени
 */
char* getTime()
{
    time_t now;
    struct tm *ptr;
    static char tbuf[64];
    
    bzero(tbuf, 64);
    time(&now);
    
    ptr = localtime(&now);
    
    strftime(tbuf, 64, "%e-%b %H:%M:%S", ptr);

    return tbuf;
}



/**
 * Функция возвращает результат выполнения linux команды
 * 
 * @param  command      команда
 * 
 * @return char
 */ 
char* getCommand(char command[128])
{
    FILE *pCom;
    static char comText[256];

    bzero(comText, 256);
    char buf[64];

    pCom = popen(command, "r"); //выполняем
    if (pCom == NULL) {
        strcpy(comText, "");
        strcat(comText, command);
        strcat(comText, "\n");
        WriteLog(comText);


        return "";
    }
    strcpy(comText, "");
    while (fgets(buf, 64, pCom) != NULL) { //читаем результат
        strcat(comText, buf);
    }
    pclose(pCom);

    return comText;
}

int mainExit(int code)
{
    exit(code);
    // return c;
    return 0;
}


/**
 * функция записи строки в лог
 * 
 * @param  msg
 *
 * @return int
 */
int WriteLog(char *msg, const char* fname)
{
    FILE* f;
    char* time;

    f = fopen(fname, "a");
    if (f) {
        time = getTime();
        fprintf(f, "[%s] [DAEMON] %s", time, msg);
        fclose(f);
        return 1;
    } else {
        return 0;
    }
}

/**
 * Проверка существования файла
 *
 * @param fname char файл
 * 
 * @return int
 */
int FileExists(const char *fname) 
{
    FILE *file;

    if (file = fopen(fname, "r")) {
        fclose(file);
        return 1;
    }

    return 0;
}

/**
 * Запись ID текущего процесса в файл
 *
 * @param fname char файл
 * 
 * @return  int
 */
int SetPidFile(const char* fname) 
{
    FILE* f;

    f = fopen(fname, "w+");
    if (f) {
        fprintf(f, "%u", getpid());
        fclose(f);
        return 1;
    } else {
        return 0;
    }
}





/**
 * функция установки максимального кол-во дескрипторов которое может быть открыто 
 * @param  MaxFd максимум форков
 * @return       [description]
 */
int SetFdLimit(int MaxFd)
{
    struct rlimit lim;
    int           status;

    // зададим текущий лимит на кол-во открытых дискриптеров
    lim.rlim_cur = MaxFd;
    // зададим максимальный лимит на кол-во открытых дискриптеров
    lim.rlim_max = MaxFd;

    // установим указанное кол-во
    status = setrlimit(RLIMIT_NOFILE, &lim);

    return status;
}