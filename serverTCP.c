#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#include <pthread.h> 

#define PORT 2909

extern int errno;

typedef struct thData{
	int idThread; 
	int cl; 
}thData;

sqlite3 *db;

typedef struct info{
    int cod_game;
    int nr_quest;
    char* username;
}info;

typedef struct quest{
    char question[600];
    char answer[30];
    char comment[500];
}quest;

typedef struct d_game{
    int cod_game;
    int nr_pl;
    int nr_apl;
    int question_id[30];
    quest text;
    int answ;
    char win_name[100];
    int win_scor;
    pthread_cond_t cond1; 
    pthread_mutex_t lock; 
}d_game;

d_game games[100];
int k=0;
//pthread_mutex_t lock; 
//pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER; 

static void *treat(void *); 


int find(sqlite3 *db, char username[100]);
void insert(sqlite3 *db, char username[100]);
char check_cod(sqlite3 *db, int cod_game);
void insert_game(sqlite3 *db, int nr_player, int nr_quest, int cod_game);
void update_host(sqlite3 *db, char username[100], int cod_game);
void update_player(sqlite3 *db, char username[100], int cod_game);
int joc_nou(sqlite3 *db,char username[100],int cod_game, int, int);
int check_pl(sqlite3 *db, int cod_game);
void update_game2(sqlite3 *db, int cod_game);
void login(void *,sqlite3 *db,info* data);

void game(void *,sqlite3 *db, info* data);
char check_host(sqlite3* db,char username[100]);
void game_data(sqlite3* db,info* data);
char check_quest(int id_quest, int id, int question_id[30]);
void get_question(sqlite3* db,int question_id[30], int id, quest* text);
void quest_data(sqlite3* db, int id, quest* text);
void update_score(sqlite3* db, char username[100]);
void winner(sqlite3* db, int cod_game,char win_name[100], int* win_scor);
int pl_score(sqlite3* db,int cod_game,char username[100]);
void delete_player(sqlite3* db, char username[100]);
void delete_game(sqlite3* db, int cod_game,int n);

void find_host(sqlite3* db, int cod_game);
void quit(sqlite3* db, info* data, int n);

int main ()
{
    struct sockaddr_in server;	
    struct sockaddr_in from;	
    int sd;		
    int pid;
    pthread_t th[100];    
	int i=0;

    int rc = sqlite3_open("proiect.db", &db);
     if (rc != SQLITE_OK) {
        printf("Nu se deschide baza de date: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
    int on=1;
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));
 
    server.sin_family = AF_INET;	
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);

    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

    if (listen (sd, 10) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
    /*if (pthread_mutex_init(&lock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } */
    while (1)
        {
        int client;
        thData * td;     
        int length = sizeof (from);

        printf ("[server]Asteptam la portul %d...\n",PORT);
        fflush (stdout);

        if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	    {
	        perror ("[server]Eroare la accept().\n");
	        continue;
	    }

	    td=(struct thData*)malloc(sizeof(struct thData));	
	    td->idThread=i++;
	    td->cl=client;

	    pthread_create(&th[i], NULL, &treat, td);	      
				
	}
   // pthread_mutex_destroy(&lock); 
    sqlite3_close(db);
};
static void *treat(void * arg)
{		
		struct thData tdL; 
        info data;
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		login((struct thData*)arg, db, &data);
        game((struct thData*)arg, db, &data);
		close ((intptr_t)arg);
		return(NULL);	
  		
};

void game(void *arg,sqlite3 *db, info* data){
	struct thData tdL; 
	tdL= *((struct thData*)arg);
    game_data(db,data);
    char pl_answer[30];
    char question[600];
    char answer[30];
    char comment[500];
    char ok;
    char username[100];
    if (write (tdL.cl, &data->nr_quest, sizeof(int)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        }
    strcpy(username,data->username);
    int j=0;
    while(j<k){
        if(data->cod_game==games[j].cod_game) break;
        j++;
    }
    if (read (tdL.cl, &ok,sizeof(ok)) <= 0)
		{
			printf("[Thread %d]\n",tdL.idThread);
			perror ("Eroare la read() de la client.\n");
		}
    if(ok==0){
        quit(db,data,j);
        return;
    }
    games[j].nr_apl++;
    while(games[j].nr_apl<games[j].nr_pl){
    }
    ok=1;
    if (write (tdL.cl, &ok, sizeof(ok)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        }
    char host=0;
    //char* substr = (char*)malloc(sizeof(char)*30);
    if(check_host(db,username)==1) host=1;
    for(int i=1; i<=data->nr_quest; i++){
        if(host==1) sleep(1);
        pthread_mutex_lock(&games[j].lock);
        if(host==1){
            memset(&games[j].text.question[0], 0, sizeof(games[j].text.question));
            memset(&games[j].text.answer[0], 0, sizeof(games[j].text.answer));
            memset(&games[j].text.comment[0], 0, sizeof(games[j].text.comment));
            games[j].answ=0;
            get_question(db, games[j].question_id, i, &games[j].text);
            for(int th=0; th<games[j].nr_apl; th++) pthread_cond_signal(&games[j].cond1); 
        }else{
            /*(games[j].answ!=0)&&(games[j].text.question[0]=='\0')&& (games[j].text.answer[0]=='\0') && (games[j].text.comment[0]=='\0')*/
           // while(!games[j].x){
            //};
            //printf("waiting\n");
            pthread_cond_wait(&games[j].cond1, &games[j].lock); 
            //printf("done\n");
        }
        pthread_mutex_unlock(&games[j].lock); 
        memset(&pl_answer[0], 0, sizeof(pl_answer));
        ok=0;
        if (write (tdL.cl, &games[j].text.question, sizeof(games[j].text.question)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        }
        if (read (tdL.cl, &pl_answer,sizeof(pl_answer)) <= 0)
		{
			printf("[Thread %d]\n",tdL.idThread);
			perror ("Eroare la read() de la client.\n");
			
		}
        char * substr = pl_answer + 1;
        // substr1 = pl_answer + 1;
        //char substr[strlen(substr1) + 1];
        //strcpy(substr, substr1);
        //printf("%s\n",substr);
        if(strcmp("quit",substr)==0 || strcmp("quit",pl_answer)==0){
            quit(db,data,j);
            return;
        }
        //printf("%s %s %s\n", substr,pl_answer, games[j].text.answer);
        if(strcmp(games[j].text.answer,substr)==0 || strcmp(games[j].text.answer,pl_answer)==0){

            ok=1;
            update_score(db, username);
        }
        //free(substr);
       // substr=NULL;
        if (write (tdL.cl, &ok, sizeof(ok)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        }
        if(ok==0){
            if (write (tdL.cl, &games[j].text.answer, sizeof(games[j].text.answer)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        }
        }

        if (write (tdL.cl, &games[j].text.comment, sizeof(games[j].text.comment)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        }
        
        sleep(1);
        if(check_host(db,username)==1) host=1;
        games[j].answ++;
       // printf("%s %d %d\n", username, games[j].answ, games[j].nr_pl);
        if(host==1) while(games[j].answ<games[j].nr_pl){}

    }
    if(check_host(db,username)==1) {
        host=1;
        winner(db,data->cod_game, games[j].win_name,&games[j].win_scor);
    } else {
        while(games[j].win_name[0]=='\0'){};
    }
    int vv=pl_score(db,data->cod_game,data->username);
    if (write (tdL.cl, &games[j].win_name, sizeof(games[j].win_name)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        }
    if (write (tdL.cl, &games[j].win_scor, sizeof(games[j].win_scor)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        }
    if (write (tdL.cl, &vv, sizeof(vv)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        }
    delete_player(db, data->username);
    if(host==1){
        sleep(5);
        delete_game(db,data->cod_game,j);
    }
}

void winner(sqlite3* db, int cod_game,char win_name[100], int* win_scor){
    char sql_fetch[256];
    char sql_fetch1[256];
    char ** errmsg;
    int res;
    sqlite3_stmt *stmt;
    sqlite3_stmt *stmt1;
    sprintf(sql_fetch, "select NAME from USERS u join GAME g on u.GAME_ID=g.GAME_ID WHERE g.GAME_ID=%d order by SCORE desc limit 1;", cod_game);
    res=sqlite3_prepare_v2(db, sql_fetch, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select name winner: %s\n", sqlite3_errmsg(db));
    }//else{printf("select game");}
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
      strcpy(win_name,sqlite3_column_text(stmt, 0));
    }
    sprintf(sql_fetch1, "select SCORE from USERS u join GAME g on u.GAME_ID=g.GAME_ID WHERE g.GAME_ID=%d order by SCORE desc limit 1;", cod_game);
    res=sqlite3_prepare_v2(db, sql_fetch1, -1, &stmt1, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select score winner: %s\n", sqlite3_errmsg(db));
    }//else{printf("select player ");}
    res = sqlite3_step(stmt1);
    if (res == SQLITE_ROW) {
      *win_scor=(int)sqlite3_column_int(stmt1, 0);
    }
};
void delete_player(sqlite3* db, char username[100]){
    char sql_del[256];
    char ** errmsg;
    int res;
    sprintf(sql_del, "DELETE FROM USERS WHERE name='%s';", username);
    res=sqlite3_exec(db,sql_del,NULL, 0, errmsg);
    if(res != SQLITE_OK){
        printf("eroare update: %s\n", sqlite3_errmsg(db));
    } else printf("jucator sters\n");
}
void delete_game(sqlite3* db, int cod_game, int n){
    char sql_del[256];
    char * errmsg;
    int res;
    sprintf(sql_del, "DELETE FROM GAME WHERE game_id=%d;", cod_game);
    res=sqlite3_exec(db,sql_del,NULL, 0, &errmsg);
    if(res != SQLITE_OK){
        printf("eroare update: %s\n", sqlite3_errmsg(db));
    } else printf("joc sters\n");
    games[n].cod_game=-1;
}
int pl_score(sqlite3* db,int cod_game,char username[100]){
    char sql_fetch[256];
    char ** errmsg;
    int res;
    sqlite3_stmt *stmt;
    sprintf(sql_fetch, "select SCORE from USERS u join GAME g on u.GAME_ID=g.GAME_ID WHERE g.GAME_ID=%d and name='%s';", cod_game,username);
    res=sqlite3_prepare_v2(db, sql_fetch, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select scor player: %s\n", sqlite3_errmsg(db));
    }//else{printf("select game");}
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
      return (int)sqlite3_column_int(stmt, 0);
    }
}
char check_host(sqlite3* db,char username[100]){
    char sql_fetch[256];
    char ** errmsg;
    int res;
    sqlite3_stmt *stmt;
    int host;
    sprintf(sql_fetch, "SELECT HOST FROM USERS WHERE NAME='%s';", username);
    res=sqlite3_prepare_v2(db, sql_fetch, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select host: %s\n", sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
      host=(int)sqlite3_column_int(stmt, 0);
    }
    if(host==1) return 1; else return 0;
};
void game_data(sqlite3* db,info* data){
    char sql_fetch[256];
    char sql_fetch1[256];
    char ** errmsg;
    int res;
    sqlite3_stmt *stmt;
    sqlite3_stmt *stmt1;
    sprintf(sql_fetch, "SELECT GAME_ID FROM USERS WHERE NAME='%s';", data->username);
    res=sqlite3_prepare_v2(db, sql_fetch, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select game_id: %s\n", sqlite3_errmsg(db));
    }//else{printf("select game");}
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
      data->cod_game=(int)sqlite3_column_int(stmt, 0);
    }
    sprintf(sql_fetch1, "SELECT NR_Q FROM GAME WHERE GAME_ID=%d;", data->cod_game);
    res=sqlite3_prepare_v2(db, sql_fetch1, -1, &stmt1, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select nr_player: %s\n", sqlite3_errmsg(db));
    }//else{printf("select player ");}
    res = sqlite3_step(stmt1);
    if (res == SQLITE_ROW) {
      data->nr_quest=(int)sqlite3_column_int(stmt1, 0);
    }
};
void get_question(sqlite3* db,int question_id[30], int id, quest* text){
        int id_quest=(rand() % 22)+1;
        while(!check_quest(id_quest, id, question_id)){
            id_quest=(rand() % 2)+1;
        }
        quest_data(db,id_quest,text);
        question_id[id-1]=id_quest;
};
char check_quest(int id_quest, int id, int question_id[30]){
        for(int i=0; i<id; i++){
            if(id_quest==question_id[i]) return 0;
        }
        return 1;
};
void quest_data(sqlite3* db,int id, quest* text){
    char sql_fetch[256];
    char sql_fetch1[256];
    char sql_fetch2[256];
    char ** errmsg;
    int res;
    sqlite3_stmt *stmt;
    sqlite3_stmt *stmt1;
    sqlite3_stmt *stmt2;

    sprintf(sql_fetch, "SELECT QUESTION FROM QUESTIONS WHERE ID=%d;", id);
    res=sqlite3_prepare_v2(db, sql_fetch, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select: %s\n", sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
      strcpy(text->question,sqlite3_column_text(stmt, 0));
    }
    sprintf(sql_fetch1, "SELECT ANSWER FROM QUESTIONS WHERE ID=%d;", id);
    res=sqlite3_prepare_v2(db, sql_fetch1, -1, &stmt1, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select: %s\n", sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt1);
    if (res == SQLITE_ROW) {
      strcpy(text->answer,sqlite3_column_text(stmt1, 0));
    } 
    sprintf(sql_fetch2, "SELECT COMENTARY FROM QUESTIONS WHERE ID=%d;", id);
    res=sqlite3_prepare_v2(db, sql_fetch2, -1, &stmt2, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select: %s\n", sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt2);
    if (res == SQLITE_ROW) {
      strcpy(text->comment,sqlite3_column_text(stmt2, 0));
    } 
}
void update_score(sqlite3* db, char username[100]){
    char sql_upd[256];
    char ** errmsg;
    int res;
    sprintf(sql_upd, "UPDATE USERS SET SCORE=SCORE+1 WHERE NAME='%s';", username);
    res=sqlite3_exec(db,sql_upd,NULL, 0, errmsg);
    if(res != SQLITE_OK){
        printf("eroare update: %s\n", sqlite3_errmsg(db));
    } else printf("Update scor cu succes\n");
}

void login(void *arg,sqlite3 *db, info* data){
    int nr, i=0;
	struct thData tdL; 
    char username[100];
	tdL= *((struct thData*)arg);
    char ok=0;
    memset(&username[0], 0, sizeof(username));
    if (read (tdL.cl, &username,sizeof(username)) <= 0)
		{
			printf("[Thread %d]\n",tdL.idThread);
			perror ("Eroare la read() de la client.\n");
			
		}
    username[strlen(username)-1]='\0';	
    if(find(db, username)!=0){
        while(!ok){
            if (write (tdL.cl, &ok, sizeof(ok)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        }
	        else
		        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
            memset(&username[0], 0, sizeof(username));
            if (read (tdL.cl, &username,sizeof(username)) <= 0)
		        {
			        printf("[Thread %d]\n",tdL.idThread);
			        perror ("Eroare la read() de la client.\n");
			
		        }
                username[strlen(username)-1]='\0';	
            if(find(db, username)==0) ok=1;
        }
    };
    ok=1;
    if (write (tdL.cl, &ok, sizeof(ok)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        }
	        else
		        //printf ("[Thread %d]Mesajul a fost trasmis cu succes1.\n",tdL.idThread);
    insert(db,username);

    data->username=username;
    int ok1, ok2;
    int cod_game;
    if (read (tdL.cl, &ok1,sizeof(ok1)) <= 0)
		{
			printf("[Thread %d]\n",tdL.idThread);
			perror ("Eroare la read() de la client.\n");
		}
    int nr_player, nr_quest; 
    if(ok1==1){
        if (read (tdL.cl, &nr_player,sizeof(nr_player)) <= 0)
		{
			printf("[Thread %d]\n",tdL.idThread);
			perror ("Eroare la read() de la client.\n");
		}
        if (read (tdL.cl, &nr_quest,sizeof(nr_quest)) <= 0)
		{
			printf("[Thread %d]\n",tdL.idThread);
			perror ("Eroare la read() de la client.\n");
		}
        cod_game=joc_nou(db, username, cod_game, nr_player,nr_quest);
        if (write (tdL.cl, &cod_game, sizeof(int)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        } 
        //printf("%s",username1);
    } else{
        if (read (tdL.cl, &cod_game,sizeof(int)) <= 0)
		{
			printf("[Thread %d]\n",tdL.idThread);
			perror ("Eroare la read() de la client.\n");
		}
        char ch=0;
        char f1=check_cod(db, cod_game);
        if(f1!=0 && (check_pl(db,cod_game)==0)) f1=0;
        if (write (tdL.cl, &f1, sizeof(f1)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        } 
                
        if(!f1){ 
          while(!ch){
              if (read (tdL.cl, &ok2,sizeof(ok2)) <= 0)
		        {
			        printf("[Thread %d]\n",tdL.idThread);
			        perror ("Eroare la read() de la client.\n");
		        }
              if(ok2==1){
                if (read (tdL.cl, &nr_player,sizeof(nr_player)) <= 0)
		            {
			            printf("[Thread %d]\n",tdL.idThread);
			            perror ("Eroare la read() de la client.\n");
		            }
                if (read (tdL.cl, &nr_quest,sizeof(nr_quest)) <= 0)
		            {
			            printf("[Thread %d]\n",tdL.idThread);
			            perror ("Eroare la read() de la client.\n");
		            }
                    
                cod_game=joc_nou(db, username, cod_game, nr_player,nr_quest);
                if (write (tdL.cl, &cod_game, sizeof(cod_game)) <= 0)
		        {
		            printf("[Thread %d] ",tdL.idThread);
		            perror ("[Thread]Eroare la write() catre client.\n");
		        }
                ch=1;
              } else{
                  if (read (tdL.cl, &cod_game,sizeof(int)) <= 0)
		            {
			            printf("[Thread %d]\n",tdL.idThread);
			            perror ("Eroare la read() de la client.\n");
		            }
                  ch=check_cod(db, cod_game);
                  if(ch!=0 && (check_pl(db,cod_game)==0)) ch=0;
                  if (write (tdL.cl, &ch, sizeof(ch)) <= 0)
		            {
		                printf("[Thread %d] ",tdL.idThread);
		                perror ("[Thread]Eroare la write() catre client.\n");
		            } 
                  /*if(!f1){
                        printf("Jocul e plin sau nu exista. Cod nou:");
                        scanf("%d", &cod_game);
                    } else ch=1;*/
              }
          }
        }
        if(ok2!=1)update_player(db, username, cod_game);
        if(ch==0)update_game2(db,cod_game);
    }
};
void update_game2(sqlite3 *db, int cod_game){
    char sql_upd[256];
    char ** errmsg;
    int res;
    /*res=sqlite3_prepare_v2(db, sql_find, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select: %s\n", sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt1);
    if (res == SQLITE_ROW) {
      nr_apl=(int)sqlite3_column_int(stmt1, 0);
    } else printf("wut\n");*/
    sprintf(sql_upd, "UPDATE GAME SET NR_PL=NR_PL-1 WHERE GAME_ID=%d;", cod_game);
    int m=0;
    while(m<k){
        if(games[m].cod_game==games[k].cod_game) break;
        m++;
    }
    games[m-1].nr_pl++;
    res=sqlite3_exec(db,sql_upd,NULL, 0, errmsg);
    if(res != SQLITE_OK){
        printf("eroare update: %s\n", sqlite3_errmsg(db));
    } else printf("Update joc cu succes\n");
};
int check_pl(sqlite3 *db, int cod_game){
    char sql_find[256];
    char sql_upd[256];
    char ** errmsg;
    int res;
    int nr_pl;
    sqlite3_stmt *stmt;
    sprintf(sql_find, "SELECT NR_PL FROM GAME WHERE GAME_ID=%d;", cod_game);
    res=sqlite3_prepare_v2(db, sql_find, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select: %s\n", sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
      nr_pl=(int)sqlite3_column_int(stmt, 0);
    }
    //printf("%d", nr_pl);//??
    if(nr_pl>0){
     /* nr_pl--;
      sprintf(sql_upd, "UPDATE GAME SET NR_PL=%d WHERE GAME_ID=%d;", nr_pl, cod_game);
      res=sqlite3_exec(db,sql_upd,NULL, 0, errmsg);
      if(res != SQLITE_OK){
        printf("eroare update: %s\n", sqlite3_errmsg(db));
      } else printf("Update joc cu succes\n");*/
      return 1;
    } else return 0;
};
int joc_nou(sqlite3 *db,char username[100],int cod_game, int nr_player, int nr_quest){
        
        cod_game=(rand() % 30000)+1;
        while(check_cod(db,cod_game)){
            cod_game=(rand() % 30000)+1;
        }
        int m=k;
        k++;
        games[m].cod_game=cod_game;
        games[m].nr_pl=1;
        games[m].nr_apl=0;
        games[m].answ=0;
        if (pthread_mutex_init(&games[m].lock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
        } 
        if (pthread_cond_init(&games[m].cond1, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
        } 
       // games[m].x=0;
        insert_game(db,nr_player,nr_quest, cod_game);
        update_host(db, username, cod_game);
        return cod_game;
};
void update_player(sqlite3 *db, char username[100], int cod_game){
    char sql_upd[256];
    char ** errmsg;
    int res;
    sprintf(sql_upd, "UPDATE USERS SET GAME_ID=%d WHERE NAME='%s';", cod_game, username);
    res=sqlite3_exec(db,sql_upd,NULL, 0, errmsg);
    if(res != SQLITE_OK){
        printf("eroare update: %s\n", sqlite3_errmsg(db));
    } else printf("Update player cu succes\n");
};
void update_host(sqlite3 *db, char username[100], int cod_game){
    char sql_upd[256];
    char ** errmsg;
    int res;
    sprintf(sql_upd, "UPDATE USERS SET HOST=1, GAME_ID=%d WHERE NAME='%s';", cod_game, username);
    res=sqlite3_exec(db,sql_upd,NULL, 0, errmsg);
    if(res != SQLITE_OK){
        printf("eroare update: %s\n", sqlite3_errmsg(db));
    } else printf("Update host cu succes\n");
};
void insert_game(sqlite3 *db, int nr_player, int nr_quest, int cod_game){
    char sql_ins[256];
    char * errmsg;
    int res;
    nr_player--;
    sprintf(sql_ins, "INSERT INTO GAME(GAME_ID,NR_Q,NR_PL) VALUES(%d, %d, %d);", cod_game, nr_quest, nr_player);
    res=sqlite3_exec(db,sql_ins,NULL, 0, &errmsg);
    if(res != SQLITE_OK){
        printf("eroare inserare: %s\n", sqlite3_errmsg(db));
    } else printf("Salvare joc cu succes\n");
};
char check_cod(sqlite3 *db, int cod_game){
    char sql_find[256];
    int res;
    sqlite3_stmt *stmt;
    sprintf(sql_find, "SELECT COUNT(GAME_ID) FROM GAME WHERE GAME_ID=%d",cod_game);
    res=sqlite3_prepare_v2(db, sql_find, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select: %s\n", sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt);
    if (res != SQLITE_ROW) {
        printf("eroare : %s\n", sqlite3_errmsg(db));
    }
    return sqlite3_column_int(stmt, 0);
};
void insert(sqlite3 *db, char username[100]){
    char sql_ins[256];
    char * errmsg;
    int res;
    sprintf(sql_ins, "INSERT INTO USERS(NAME) VALUES('%s');",username);
    res=sqlite3_exec(db,sql_ins,NULL, 0, &errmsg);
    if(res != SQLITE_OK){
        printf("eroare inserare: %s\n", sqlite3_errmsg(db));
    } else printf("Logare cu succes\n");
};
int find(sqlite3 *db, char username[100]){
    char sql_find[256];
    int res;
    sqlite3_stmt *stmt;
    sprintf(sql_find, "SELECT COUNT(NAME) FROM USERS WHERE NAME='%s';",username);
    res=sqlite3_prepare_v2(db, sql_find, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select: %s\n", sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt);
    if (res != SQLITE_ROW) {
        printf("eroare : %s\n", sqlite3_errmsg(db));
    }
    int x=sqlite3_column_int(stmt, 0);
    return x;
};

void quit(sqlite3* db, info* data, int n){
    games[n].nr_pl--;
    if(games[n].nr_pl==0){
        delete_game(db,games[n].cod_game,n);
        delete_player(db, data->username);
    } else{
       if(check_host(db,data->username)==1){
           delete_player(db,data->username);
           find_host(db, games[n].cod_game);
       }  else {
           delete_player(db,data->username);
       }
    }
}

void find_host(sqlite3* db, int cod_game){
    char sql_fetch[256];
    char sql_upd[256];
    char ** errmsg;
    int res;
    sqlite3_stmt *stmt;
    sqlite3_stmt *stmt1;
    char username[100];

    sprintf(sql_fetch, "SELECT NAME FROM USERS WHERE GAME_ID=%d limit 1;", cod_game);
    res=sqlite3_prepare_v2(db, sql_fetch, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        printf("eroare select: %s\n", sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
      strcpy(username,sqlite3_column_text(stmt, 0));
    }

    sprintf(sql_upd, "UPDATE USERS SET HOST=1 WHERE NAME='%s';",username);

    printf("%s\n",sql_upd);
    res=sqlite3_exec(db,sql_upd,NULL, 0, errmsg);
    if(res != SQLITE_OK){
        printf("eroare update: %s\n", sqlite3_errmsg(db));
    } else printf("Update player cu succes\n");

}