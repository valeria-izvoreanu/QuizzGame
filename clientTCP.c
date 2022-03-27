#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

extern int errno;

int port;

void login(int sd);
void game(int sd, int nr_quest);
void timer(int n);

int main (int argc, char *argv[])
{
    int sd;			
    struct sockaddr_in server;
    char buf[10];
    char username[100];
    int cod_game;

    if (argc != 3)
    {
        printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    port = atoi (argv[2]);

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("Eroare la socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons (port);
  
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
        perror ("[client]Eroare la connect().\n");
        return errno;
    }
    login(sd);
    printf("Te-ai logat. Pentru a iesi foloseste comanda quit\n");
    int nr_quest;
    if (read (sd, &nr_quest,sizeof(int)) < 0)
    {
        perror ("[client]Eroare la read() de la server.\n");
    }
    printf("Numar intrebari: %d\n",nr_quest);
    game(sd,nr_quest);
    close (sd);
}

void game(int sd, int nr_quest){
    char my_answer[30];
    char ok;
    char ans;
    char bye[8];
    char question[600];
    char answer[30];
    char comment[500];
    printf("Doriti sa incepeti jocul? Y|N \n");
    read (0, &ans, sizeof(ans));
    ok=1;
    if(ans=='N') {
        printf("Renunti?\n");
        fflush (stdout);
        scanf("%s", bye);
        if(strcmp(bye,"quit")==0) ok=0;
    }
    while(ans!='Y' && ok!=0){
        printf("Doriti sa incepeti jocul? Y|N \n");
        read (0, &ans, sizeof(ans));
        if(ans=='N') {
            printf("Renunti?\n");
            fflush (stdout);
            scanf("%s", bye);
            if(strcmp(bye,"quit")==0) ok=0;
        }    
    }
    if (write (sd,&ok,sizeof(ok)) <= 0)
        {
            perror ("[client]Eroare la write() spre server.\n");
        };
    if(ok==0) {
        close (sd);
        exit(0);
    }
    printf("Asteptati restul jucatorilor...\n");
     if (read (sd, &ok,sizeof(ok)) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
        }
    printf("Incepe jocul:\n");
    printf("Raspundeti exact si fara diacritice:\n");
    for(int i=1; i<=nr_quest; i++){
        memset(&question[0], 0, sizeof(question));
        memset(&answer[0], 0, sizeof(answer));
        memset(&my_answer[0], 0, sizeof(my_answer));
        memset(&comment[0], 0, sizeof(comment));

        printf("Intrebarea %d: \n", i);
        fflush(stdout);
        if (read (sd, &question,sizeof(question)) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
        } 
        printf("%s\n", question);
        timer(5);
        printf("Aveti 15 secunde sa scrieti raspunsul:\n");
        sleep(15);
        read (0, &my_answer, sizeof(my_answer));
        my_answer[strlen(my_answer)-1]='\0';
        if(my_answer[0]=='\0') strcpy(my_answer," random");
        //printf("%s\n",my_answer);
        //printf("%ld\n",strlen(my_answer));
        if (write (sd,&my_answer,strlen(my_answer)) <= 0)
        {
            perror ("[client]Eroare la write() spre server.\n");
        }
        char* substr = my_answer + 1;
        //printf("%s\n",substr);
        if(strcmp(substr,"quit")==0 || strcmp("quit",my_answer)==0) {
            close(sd);
            exit(0);
        }
        if (read (sd, &ok,sizeof(ok)) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
        }
        if(ok==1){
            printf("Raspuns corect\n");
        }else{
            printf("Raspuns gresit\n");
            printf("Raspuns corect: ");
            if (read (sd, &answer,sizeof(answer)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
            }
            printf("%s\n", answer);
        }
        printf("Comentariu: ");
        if (read (sd, &comment,sizeof(comment)) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
        }
        printf("%s\n", comment);
        sleep(5);
    }
    char win_name[100];
    int win_score;
    int my_score;
    if (read (sd, &win_name,sizeof(win_name)) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
        }
    if (read (sd, &win_score,sizeof(win_score)) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
        }
    if (read (sd, &my_score,sizeof(my_score)) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
        }
    if(my_score==win_score){
        printf("Ai castigat!\n");
    } else{
        printf("Castigatorul este:\n");
        printf("%s - %d\n", win_name, win_score);
    }
    printf("Scorul tau: %d\n", my_score);
};
void timer(int n) 
{ 
    int seconds=0;
    n++;
    while(n){
        printf("Timp: ");
        printf("%d",seconds);
        printf("\r       ");
        fflush (stdout);
        sleep(1);
        seconds++;
        n--;
    }
    printf("\n");
} 


void login(int sd){
    char username [100];
    char ok;
    printf ("Username: ");
    fflush (stdout);
    //scanf("%s",username);
    memset(&username[0], 0, sizeof(username));
    read (0, username, sizeof(username));
    if (write (sd,&username,sizeof(username)) <= 0)
    {
        perror ("[client]Eroare la write() spre server.\n");
    }
    if (read (sd, &ok,sizeof(char)) < 0)
    {
        perror ("[client]Eroare la read() de la server.\n");
    }
    
    while(!ok){
        printf ("Username luat. Username nou: ");
        fflush (stdout);
        memset(&username[0], 0, sizeof(username));
        read (0, username, sizeof(username));
        if (write (sd,&username,sizeof(username)) <= 0)
        {
            perror ("[client]Eroare la write() spre server.\n");
        }
        if (read (sd, &ok,sizeof(char)) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
        }
    }
    int ok1,ok2;
    int cod_game;
    printf("Joc nou? 1|2: ");
    fflush (stdout);
   // read (0, &ok1, sizeof(int));
    scanf("%d", &ok1);
    int nr_player, nr_quest;
    if(ok1==1){
        printf("Cati jucatori? ");
        fflush (stdout);
        scanf("%d", &nr_player);
        printf("Cate intrebari? ");
        fflush (stdout);
        scanf("%d", &nr_quest);
        if (write (sd,&ok1,sizeof(ok1)) <= 0)
            {
                perror ("[client]Eroare la write() spre server.\n");
            }
        if (write (sd,&nr_player,sizeof(int)) <= 0)
            {
                perror ("[client]Eroare la write() spre server.\n");
            }
        if (write (sd,&nr_quest,sizeof(int)) <= 0)
            {
                perror ("[client]Eroare la write() spre server.\n");
            }
        if (read (sd, &cod_game,sizeof(int)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
            }
        printf("Codul este: %d\n", cod_game);
    }else{
        printf("Insereaza cod (pana la 5 cifre): ");
        fflush (stdout);
        scanf("%d", &cod_game);
        if (write (sd,&ok1,sizeof(ok1)) <= 0)
            {
                perror ("[client]Eroare la write() spre server.\n");
            }
        if (write (sd,&cod_game,sizeof(int)) <= 0)
            {
                perror ("[client]Eroare la write() spre server.\n");
            }
        if (read (sd, &ok,sizeof(char)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
            }
                while(!ok){
                    printf("Acest joc nu exista sau este plin. \n");
                    printf("Doriti sa faceti un joc nou? 1|2");
                    fflush (stdout);
                    //read (0, &ok2, sizeof(int));
                    scanf("%d", &ok2);
                    if(ok2==1){ //era ok1???
                        printf("Cati jucatori? ");
                        fflush (stdout);
                        scanf("%d", &nr_player);
                        printf("Cate intrebari? ");
                        fflush (stdout);
                        scanf("%d", &nr_quest);
                        if (write (sd,&ok2,sizeof(ok2)) <= 0)
                            {
                                perror ("[client]Eroare la write() spre server.\n");
                            }
                        if (write (sd,&nr_player,sizeof(int)) <= 0)
                            {
                                perror ("[client]Eroare la write() spre server.\n");
                            }
                        if (write (sd,&nr_quest,sizeof(int)) <= 0)
                            {
                                perror ("[client]Eroare la write() spre server.\n");
                            }
                        if (read (sd, &cod_game,sizeof(cod_game)) < 0)
                        {
                            perror ("[client]Eroare la read() de la server.\n");
                        }
                        printf("Codul este: %d\n", cod_game);
                        ok=1;
                        } else{
                            printf("Cod nou: ");
                            fflush (stdout);
                            scanf("%d", &cod_game);
                            if (write (sd,&ok2,sizeof(ok2)) <= 0)
                                {
                                    perror ("[client]Eroare la write() spre server.\n");
                                }
                            if (write (sd,&cod_game,sizeof(int)) <= 0)
                                {
                                    perror ("[client]Eroare la write() spre server.\n");
                                }
                            //memset(&ok, 0, sizeof(ok));
                            if (read (sd, &ok,sizeof(char)) < 0)
                                {
                                    perror ("[client]Eroare la read() de la server.\n");
                                }
                        }
                }
    }
};