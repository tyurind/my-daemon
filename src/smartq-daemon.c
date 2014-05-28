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

// лимит для установки максимально кол-во открытых дискрипторов
#define FD_LIMIT            1024*10

// константы для кодов завершения процесса
#define CHILD_NEED_WORK         1
#define CHILD_NEED_TERMINATE    2


#define PID_FILE "/z/srv/daemon.pid"
#define LOG_FILE "/z/srv/daemon.log"

int WriteLog(char* msg);
int LoadConfig(char* FileName);


char* getTime() { //функция возвращает форматированную дату и время
    time_t now;
    struct tm *ptr;
    static char tbuf[64];
    bzero(tbuf, 64);
    time(&now);
    ptr = localtime(&now);
    strftime(tbuf, 64, "%Y-%m-%e %H:%M:%S", ptr);
    return tbuf;
}

char* getCommand(char command[128]) { //функция возвращает результат выполнения linux команды
    FILE *pCom;
    static char comText[256];
    bzero(comText, 256);
    char buf[64];
    pCom = popen(command, "r"); //выполняем
    if (pCom == NULL) {
        WriteLog("Error Command\n");
        return "";
    }
    strcpy(comText, "");
    while (fgets(buf, 64, pCom) != NULL) { //читаем результат
        strcat(comText, buf);
    }
    pclose(pCom);
    return comText;
}

int WriteLog(char *msg) { //функция записи строки в лог
    FILE* f;

    f = fopen(LOG_FILE, "a");
    if (f) {
        fprintf(f, "%s", msg);
        fclose(f);
    } else {
        return 1;
    }


    return 0;
}

int exists(const char *fname) {
    FILE *file;
    if (file = fopen(fname, "r")) {
        fclose(file);
        return 1;
    }
    return 0;
}

void SetPidFile(char* Filename) {
    FILE* f;

    f = fopen(Filename, "w+");
    if (f) {
        fprintf(f, "%u", getpid());
        fclose(f);
    }
}

int Daemon(void) { //собственно наш бесконечный цикл демона
    char *log;
    while (1) {
        log = getCommand("php -v");
        if (strlen(log) > 5) { //если в онлайне кто-то есть, то пишем в лог
            WriteLog(log);
        }
        sleep(2); //ждем 10 минут до следующей итерации
    }
    return 0;
}

int WorkProc() {
    struct sigaction sigact;
    sigset_t sigset;
    int signo;
    int status;
    char str[312];

    // сигналы об ошибках в программе будут обрататывать более тщательно
    // указываем что хотим получать расширенную информацию об ошибках
    sigact.sa_flags = SA_SIGINFO;
    // задаем функцию обработчик сигналов
    // sigact.sa_sigaction = signal_error;

    sigemptyset(&sigact.sa_mask);

    // установим наш обработчик на сигналы

    sigaction(SIGFPE, &sigact, 0); // ошибка FPU
    sigaction(SIGILL, &sigact, 0); // ошибочная инструкция
    sigaction(SIGSEGV, &sigact, 0); // ошибка доступа к памяти
    sigaction(SIGBUS, &sigact, 0); // ошибка шины, при обращении к физической памяти

    sigemptyset(&sigset);

    // блокируем сигналы которые будем ожидать
    // сигнал остановки процесса пользователем
    sigaddset(&sigset, SIGQUIT);

    // сигнал для остановки процесса пользователем с терминала
    sigaddset(&sigset, SIGINT);

    // сигнал запроса завершения процесса
    sigaddset(&sigset, SIGTERM);

    // пользовательский сигнал который мы будем использовать для обновления конфига
    sigaddset(&sigset, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    // Установим максимальное кол-во дискрипторов которое можно открыть
    //SetFdLimit(FD_LIMIT);

    // запишем в лог, что наш демон стартовал
    WriteLog("[DAEMON] Started\n");

    // запускаем все рабочие потоки
    // status = InitWorkThread();
    status = 1;
    if (!status) {
        // цикл ожижания сообщений
        for (;;) {
            // ждем указанных сообщений
            sigwait(&sigset, &signo);

            // если то сообщение обновления конфига
            if (signo == SIGUSR1) {
                bzero(str, 312);
                strcpy(str, "[DAEMON] ");
                strcat(str, (char*) signo);
                WriteLog(str);
            } else // если какой-либо другой сигнал, то выйдим из цикла
            {
                break;
            }
        }

        // остановим все рабочеи потоки и корректно закроем всё что надо
        //DestroyWorkThread();
    } else {
        WriteLog("[DAEMON] Create work thread failed\n");
    }

    WriteLog("[DAEMON] Stopped\n");

    // вернем код не требующим перезапуска
    return CHILD_NEED_TERMINATE;
}




int MonitorProc()
{
    int       pid;
    int       status;
    int       need_start = 1;
    sigset_t  sigset;
    siginfo_t siginfo;

    // настраиваем сигналы которые будем обрабатывать
    sigemptyset(&sigset);

    // сигнал остановки процесса пользователем
    sigaddset(&sigset, SIGQUIT);

    // сигнал для остановки процесса пользователем с терминала
    sigaddset(&sigset, SIGINT);

    // сигнал запроса завершения процесса
    sigaddset(&sigset, SIGTERM);

    // сигнал посылаемый при изменении статуса дочернего процесс
    sigaddset(&sigset, SIGCHLD);

    // сигнал посылаемый при изменении статуса дочернего процесс
    sigaddset(&sigset, SIGCHLD);

    // пользовательский сигнал который мы будем использовать для обновления конфига
    sigaddset(&sigset, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    // данная функция создат файл с нашим PID'ом
    SetPidFile(PID_FILE);

    // бесконечный цикл работы
    for (;;)
    {
        // если необходимо создать потомка
        if (need_start)
        {
            // создаём потомка
            pid = fork();
        }

        need_start = 1;

        if (pid == -1) // если произошла ошибка
        {
            // запишем в лог сообщение об этом
            WriteLog("[MONITOR] Fork failed (%s)  :: ");
            WriteLog(strerror(errno));
            WriteLog("\n");
        }
        else if (!pid) // если мы потомок
        {
            // данный код выполняется в потомке

            // запустим функцию отвечающую за работу демона
            status = WorkProc();

            // завершим процесс
            exit(status);
        }
        else // если мы родитель
        {
            // данный код выполняется в родителе

            // ожидаем поступление сигнала
            sigwaitinfo(&sigset, &siginfo);

            // если пришел сигнал от потомка
            if (siginfo.si_signo == SIGCHLD)
            {
                // получаем статус завершение
                wait(&status);

                // преобразуем статус в нормальный вид
                status = WEXITSTATUS(status);

                 // если потомок завершил работу с кодом говорящем о том, что нет нужны дальше работать
                if (status == CHILD_NEED_TERMINATE)
                {
                    // запишем в лог сообщени об этом
                    WriteLog("[MONITOR] Childer stopped\n");

                    // прервем цикл
                    break;
                }
                else if (status == CHILD_NEED_WORK) // если требуется перезапустить потомка
                {
                    // запишем в лог данное событие
                    WriteLog("[MONITOR] Childer restart\n");
                }
            }
            else if (siginfo.si_signo == SIGUSR1) // если пришел сигнал что необходимо перезагрузить конфиг
            {
                kill(pid, SIGUSR1); // перешлем его потомку
                need_start = 0; // установим флаг что нам не надо запускать потомка заново
            }
            else // если пришел какой-либо другой ожидаемый сигнал
            {
                // запишем в лог информацию о пришедшем сигнале
                WriteLog("[MONITOR] Signal %s\n");
                WriteLog(strsignal(siginfo.si_signo) );
                WriteLog("\n");

                // убьем потомка
                kill(pid, SIGTERM);
                status = 0;
                break;
            }
        }
    }

    // запишем в лог, что мы остановились
    WriteLog("[MONITOR] Stopped\n");

    // удалим файл с PID'ом
    unlink(PID_FILE);

    return status;
}



/**
 * Точка входа
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int main(int argc, char* argv[]) {
    WriteLog("Daemon Start\n");

    pid_t parpid, sid;

    // если параметров командной строки меньше двух, то покажем как использовать демана
    if (argc != 2) {
        printf("Usage: smartq-daemon filename.cfg\n");
        return -1;
    }

    WriteLog("Command: ");
    WriteLog(argv[1]);
    WriteLog("\n");


    if (exists(PID_FILE)) {
        printf("Error: Процесс уже запущен\n");
        return -1;
    }

    parpid = fork(); // создаем дочерний процесс
    if (parpid < 0) {
        exit(1);
    } else if (parpid != 0) {
        exit(0);
    }

    umask(0); // даем права на работу с фс
    sid = setsid(); // генерируем уникальный индекс процесса
    if (sid < 0) {
        exit(1);
    }
    SetPidFile(PID_FILE);

    // переходим в корень диска, если мы этого не сделаем, то могут быть проблемы.
    // к примеру с размантированием дисков
    if ((chdir("/")) < 0) { //выходим в корень фс
        exit(1);
    }

    // закрываем дискрипторы ввода/вывода/ошибок, так как нам они больше не понадобятся
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Данная функция будет осуществлять слежение за процессом
    // Daemon();
    // WorkProc();

    return MonitorProc();

    // WriteLog("=========end===========\n");
    // return 0;
}


