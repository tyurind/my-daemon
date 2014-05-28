#if !defined(_GNU_SOURCE)
    #define _GNU_SOURCE
#endif


#include <stdio.h>
#include <stdlib.h>

#include "libinclude.c"

#include <signal.h>
#include <assert.h>
#include <errno.h>

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
int mainDD(int argc, char* argv[]) 
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



int main(int argc, char* argv[]) 
{
    int status;
    int pid;

    // создаем потомка
    pid = fork();

    if (pid == -1) // если не удалось запустить потомка
    {
        // выведем на экран ошибку и её описание
        PrintLog(strerror(errno));

        return -1;
    }
    else if (!pid) // если это потомок
    {
        // данный код уже выполняется в процессе потомка
        // разрешаем выставлять все биты прав на создаваемые файлы,
        // иначе у нас могут быть проблемы с правами доступа
        umask(0);

        // создаём новый сеанс, чтобы не зависеть от родителя
        pid_t sid;
        sid = setsid(); // генерируем уникальный индекс процесса
        if (sid < 0) {
            exit(1);
        }

        // переходим в корень диска, если мы этого не сделаем, то могут быть проблемы.
        // к примеру с размантированием дисков
        if ( (chdir("/") ) < 0) { //выходим в корень фс
            PrintLog("Error: Не удалось сменить коталог\n");
            exit(1);
        }

        // закрываем дискрипторы ввода/вывода/ошибок, так как нам они больше не понадобятся
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        // Данная функция будет осуществлять слежение за процессом
        status = MonitorProc();

        return status;
    }
    else // если это родитель
    {
        // завершим процес, т.к. основную свою задачу (запуск демона) мы выполнили
        return 0;
    }
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


    while (1) {
        log = getCommand(command);
        // WriteLog("Error: Процесс уже запущен\n");
        // ждем 2 секунды до следующей итерации
        WriteLog(log, LOG_FILE);
         
        sleep(2);
    }
    return 0;
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
            PrintLog(strerror(errno));
        }
        else if (!pid) // если мы потомок
        {
            // данный код выполняется в потомке

            // запустим функцию отвечающую за работу демона
            status = 1;//WorkProc();
            PrintLog("Мы ПОТОМОК ))\n");
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
                    PrintLog("[MONITOR] Childer stopped\n");

                    // прервем цикл
                    break;
                }
                else if (status == CHILD_NEED_WORK) // если требуется перезапустить потомка
                {
                    // запишем в лог данное событие
                    PrintLog("[MONITOR] Childer restart\n");
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
                PrintLog(strsignal(siginfo.si_signo));

                // убьем потомка
                kill(pid, SIGTERM);
                status = 0;
                break;
            }
        }
    }

    // запишем в лог, что мы остановились
    PrintLog("[MONITOR] Stopped\n");

    // удалим файл с PID'ом
    unlink(PID_FILE);

    return status;
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



// функция обработки сигналов
static void signal_error(int sig, siginfo_t *si, void *ptr)
{
    void*  ErrorAddr;
    void*  Trace[16];
    int    x;
    int    TraceSize;
    char** Messages;

    // запишем в лог что за сигнал пришел
    // PrintLog("[DAEMON] Signal: %s, Addr: 0x%0.16X\n", strsignal(sig), si->si_addr);
    PrintLog(strsignal(sig));


    #if __WORDSIZE == 64 // если дело имеем с 64 битной ОС
        // получим адрес инструкции которая вызвала ошибку
        ErrorAddr = (void*)((ucontext_t*)ptr)->uc_mcontext.gregs[REG_RIP];
    #else
        // получим адрес инструкции которая вызвала ошибку
        ErrorAddr = (void*)((ucontext_t*)ptr)->uc_mcontext.gregs[REG_EIP];
    #endif

    // произведем backtrace чтобы получить весь стек вызовов
    TraceSize = backtrace(Trace, 16);
    Trace[1] = ErrorAddr;

    // получим расшифровку трасировки
    Messages = (char**) backtrace_symbols(Trace, TraceSize);
    if (Messages)
    {
        PrintLog("== Backtrace ==\n");

        // запишем в лог
        for (x = 1; x < TraceSize; x++)
        {
            PrintLog(Messages[x]);
        }

        PrintLog("== End Backtrace ==\n");
        free(Messages);
    }

    PrintLog("[DAEMON] Stopped\n");

    // остановим все рабочие потоки и корректно закроем всё что надо
    DestroyWorkThread();

    // завершим процесс с кодом требующим перезапуска
    exit(CHILD_NEED_WORK);
}