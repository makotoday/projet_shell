#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include  <fcntl.h>
#include <string.h>
#include "commande.h"
#include "find.h"


char *get_buf_shell(int* erreur);
void affiche();
void copie_mot(char *dest, char **src);
void initMotsCommandes(char *commande,char *mots_commande[]);
void execution_path(char *mots_commande[]);
void execution_all(char *mots_commande[]);// c'est la methode final de l'exécution des commande prenant tout en paramètre
void execution_commande(char *mots_commande[]);// méthode exécutant la commande avec prise en compte des pipes
void executionPipeBar(char *mots_commande[]);


int main()
{
    char *commande = malloc(sizeof(char)*MAX_BUFF_COMMANDE);
    char *mots_commande[MAX_COMMANDES];
    history[0] = NULL;
    indice_pipe[0] = -1;
    int erreur;
    while(1)
    {
        affiche();
        commande = get_buf_shell(&erreur);//contient la commande entiere
	if (erreur==1) {
            kill(0,SIGKILL);
        }
        initMotsCommandes(commande,mots_commande);
        enregistre_histo(commande, &nbcommandeAct);
        //execution2(mots_commande);
	execution_all(mots_commande);


    }

    return 0;
}

void initMotsCommandes(char *commande,char *mots_commande[])
{
    int i=0;
    int j = 0;
    while(*commande!='\0')
    {
        if(*commande=='>'||*commande=='<'||*commande=='|')
        {
            indice_pipe[j] = *commande;
            indice_pipe[j+1] = i;
            j+=2;
            do{
				commande = commande+1;
            }while(*commande==' ');

        }else
        {
            mots_commande[i] = malloc(sizeof(char)*MAX_PATH);
            copie_mot(mots_commande[i],&commande);
            i++;
        }

    }
    indice_pipe[j] = -1;
    mots_commande[i] = NULL;
}

void copie_mot(char *dest, char **src)
{
    //**src est le pointeur pour incrementer directement le pointeur sur caractere de commande
    while(**src!=' '&&**src!='\n')
    {
        *dest = **src;
        dest = dest+1;
        *src = (*src)+1;
    }
    *dest = '\0';
    do
    {
            *src = (*src)+1;
        }while(**src==' ');//on saute le(s) espace(s) entre 2 mots

}

void affiche()
{
    char buff[MAX_PATH];
    char* test=getcwd(buff,MAX_PATH);
    printf("%s> ",test);
}

char *get_buf_shell(int* erreur)
{
    /*char *Buffer = malloc(sizeof(char)*MAX_BUFF_COMMANDE);

        if(!(fgets(Buffer,MAX_BUFF_COMMANDE,stdin)))
        {
            while(1)
            {
                            //int d;
                //d = getchar();
                exit(EXIT_SUCCESS);
            }
        }

    return Buffer;*/

 char *Buffer = malloc(sizeof(char)*MAX_BUFF_COMMANDE);
    if(fgets(Buffer,MAX_BUFF_COMMANDE,stdin)==NULL) {
        *erreur=1;
    }
    return Buffer;

}


void execution_all(char *mots_commande[])
{
    pid_t fk;
    fk = fork();
    if(fk<0)
    {
        perror("");
        return;
    }
    if(fk==0)//processus fils
    {
        execution_commande(mots_commande);
    }
    else//processus pere
    {
        int statut;
        pid_t w;
        w = waitpid(0,&statut,0);
        if (w == -1) {
                perror("waitpid");
                return;
            }
    }
    indice_pipe[0] =-1;

}

void execution_commande(char *mots_commande[])
{

        char *fichier_redirection;
        int i=0;
        int id_stdout;
        if(indice_pipe[i]=='>')
        {
            //indicepipe[i+1] contient l'indice du mot avant le pipe
            //il y a des pipes, redirection de la sortie
            fichier_redirection =  mots_commande[indice_pipe[i+1]];
            redirection_sortie(fichier_redirection,&id_stdout);
        }
        if(indice_pipe[i]=='|')
        {
			executionPipeBar(mots_commande);

        }else {
        //execution de la commande
            if(strcmp(mots_commande[0],"cd")==0)
            {
                commande_cd(mots_commande[1]);//on envoie l'adresse a atteindre qui correspond au second argument(apres cd)
            }
            else if(strcmp(mots_commande[0],"history")==0)
            {
                commande_history(mots_commande,nbcommandeAct);
            }
            else if(strcmp(mots_commande[0],"touch")==0)
            {
                commande_touch(mots_commande);
            }
            else if(strcmp(mots_commande[0],"copy")==0)
            {
                commande_copie(mots_commande);
            }
            else if(strcmp(mots_commande[0],"cat")==0)
            {
				commande_cat(mots_commande);
			}
            else if(strcmp(mots_commande[0],"find")==0)
            {
                find(mots_commande[1]);
            }
            else
            {
                execution_path(mots_commande);
            }
            if(indice_pipe[i]=='>'||indice_pipe[i]=='<')
            {
                redirection_sortie_standard(&id_stdout);
                indice_pipe[i] = -1;
            }
        }
}


void executionPipeBar(char *mots_commande[])//ne fonctionne pas
{
           char **seconde_commande = malloc(sizeof(char*)*MAX_COMMANDES);
            int i;
            for(i=0;mots_commande[indice_pipe[1]+i]!=NULL;i++)
            {
                seconde_commande[i] = mots_commande[indice_pipe[1]+i];
            }
            seconde_commande[i] = NULL;
            mots_commande[indice_pipe[1]] = NULL;


            int fd[2];
           if (pipe(fd)==-1)
           {
            printf("pipe error \n");
            return;
           }
            pid_t fk;
            fk = fork();
            if(fk<0)
            {
                perror("");
                return;
            }

        if(fk==0)//processus fils
        {
            close(fd[0]); // on ferme le coté lecture du tuyau
            dup2(fd[1],STDOUT_FILENO);//on duplique la sortie du tuyau vers la sortie standard
            close(fd[1]);//on ferme le coté ecriture du tuyau
             execution_path(mots_commande);

        }
        else//processus pere
        {
            close(fd[1]);//fermeture du coté écriture du tuyau
            dup2(fd[0],STDIN_FILENO);//redirection de l'entree standard vers l'entree du pipe
            close(fd[0]);//fermeture du coté lecture du tuyau
            execution_path(seconde_commande);

        }
}


/*etape 3*/
void execution_path(char *mots_commande[])
{
      char buff[MAX_PATH];
      char* file=getcwd(buff,MAX_PATH + 1);//nom du fichier actuel
      char *copie_premier_mot = malloc(sizeof(char)*MAX_PATH);
      sprintf(copie_premier_mot,"%s%s","/bin/",mots_commande[0]);//ajout du /bin/ devant le premier mot
      strcpy(mots_commande[0],copie_premier_mot);
        int i=0;
      if(indice_pipe[i]!='|')
      {
        while(mots_commande[i]!=NULL) i++;
        mots_commande[i] = malloc(sizeof(char)*MAX_PATH);
        strcpy(mots_commande[i],file);//le dernier mot est le nom du fichier actuel
        mots_commande[i+1] = NULL;
      }
      int erreur = execv(mots_commande[0],mots_commande);
      if(erreur<0)
      {
            perror("execv");
            exit(1);
      }
}



