#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include  <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include "commande.h"
#include <utime.h>
#include <time.h>


void commande_copie(char *mots_commandes[])
{
    //mots_commande[1] est la source
    DIR *isfolder = opendir(mots_commandes[1]);
    int success;
    if(isfolder!=NULL)
    {//on doit copier un repertoire
        closedir(isfolder);
        success = copie_repertoire(mots_commandes[1],mots_commandes[2]);
    }
    else
    {
        ajoutDernierMotSource(&mots_commandes[2],mots_commandes[1]);
        success = copie_unfichier(mots_commandes[1],mots_commandes[2]);
    }
    if(success<0)
    {
        printf("echec commande copy\n");
    }else printf("copy reussi\n");
}

/*Etape 1 : copie d'un seul fichier*/
int copie_unfichier(char source[MAX_PATH+1],char destination[MAX_PATH+1])
{
    mode_t mode = S_IRUSR | S_IRGRP | S_IROTH;/*droits de lecture pour tout le monde;*/
    int filesource = open(source,O_RDONLY ,mode);/*ouverture en lecture*/
   if(filesource<0)
   {
        printf("erreur d'ouverture du premier fichier\n");
        exit(1);
   }
   mode = S_IWUSR |S_IWGRP |S_IWOTH;
   /*ouverture en écriture avec effacement des données si il y en a et creation du fichier si il n'existe pas*/
   int  filedest = open(destination,O_WRONLY |O_TRUNC|O_APPEND|O_CREAT);
   if(filedest<0)
   {
        perror("erreur d'ouverture du second fichier");
        exit(1);
   }
    int nb =  copie(filesource,filedest);
    copie_droit(filesource,filedest);

    close(filesource);
    close(filedest);
    return nb;
}

int copie(int filesrc,int filecopie)
{
    char * buffer = malloc(sizeof(char)*MAX_BUFF_COMMANDE);
    ssize_t bytes_read , bytes_write;
    size_t nbytes = sizeof(buffer);
    do
    {
        bytes_read = read(filesrc,buffer,nbytes);//lecture du fichier source
        if(bytes_read<0)
        {
            printf("erreur lecture du premier fichier\n");
            return -1;
        }
        bytes_write = write(filecopie,buffer,bytes_read);//ecriture dans le nouveau fichier
        if(bytes_write<0)
        {
            printf("erreur d'écriture dans le second fichier\n");
            return -1;
        }
    }while(bytes_read>0);
    return 1;
}

/*etape 2 : copie des droits*/
int copie_droit(int filesrc,int filecopie)
{
    struct stat filestat;//info sur le fichier
   int ok = fstat(filesrc,&filestat);
   if(ok<0)
   {
        perror("erreur de lecture des droits du premier fichier");
   }
    /*copie des droits dans le second fichier*/
    mode_t mode = filestat.st_mode;
    ok = fchmod(filecopie,mode);
    if(ok<0)
    {
        perror("erreur de copie des droits dans le second fichier");
    }
    return ok;
}
/*etape 3 et 4*/
void ajoutDernierMotSource(char **repertoireDest,char *source)
{
    char *motsrc = source;
    //on cree un repertoire dans repdest avec le meme nom que src :
    while(*source!='\0')
    {
        source = source+1;
        if(*source=='/')
        {
            motsrc = source+1;
        }
    }
    sprintf(*repertoireDest,"%s/%s",*repertoireDest,motsrc);
}

int copie_repertoire(char repertoireSource[MAX_PATH+1],char repertoireDest[MAX_PATH+1])
{
    char *nom_complet_src = malloc(sizeof(char)*MAX_PATH);
    char * nom_complet_dest = malloc(sizeof(char)*MAX_PATH);
    printf("passage dest %s\n",repertoireDest);
    ajoutDernierMotSource(&repertoireDest,repertoireSource);
     int nb = creationRepertoireVide(repertoireDest,repertoireSource);
     if(nb <0){
        printf("erreur creation repertoire\n"); exit(1);
     }
    DIR *pdirSrc = opendir(repertoireSource);
    if(pdirSrc==NULL)
    {
        perror("erreur d'ouverture du premier répertoire");
        exit(1);
    }
    DIR *pdirdest = opendir(repertoireDest);
    if(pdirdest==NULL)
    {
        perror("erreur d'ouverture du répertoire a copier");
        exit(1);
    }
        struct dirent* fichierLu = NULL; //pointeur vers un des fichier du répertoire
        while ((fichierLu = readdir(pdirSrc))!= NULL)
        {
            if(strcmp(fichierLu->d_name,".")==0||strcmp(fichierLu->d_name,"..")==0||strchr(fichierLu->d_name,'~'))
            {
                continue;//on ne prend pas en compte le fichier parent et celui dans lequel on est
            }
            sprintf(nom_complet_src,"%s/%s",repertoireSource,fichierLu->d_name);
            sprintf(nom_complet_dest,"%s/%s",repertoireDest,fichierLu->d_name);
            if(fichierLu->d_type!=DT_DIR)//cas ou ce n'est pas un dossier
            {
                nb = copie_unfichier(nom_complet_src,nom_complet_dest);
                if(nb <0) exit(1);

            }else {
                sprintf(nom_complet_dest,"%s",repertoireDest);
                nb =  copie_repertoire(nom_complet_src,nom_complet_dest);//on copie tout le contenue du repertoire
                 if(nb<0)
                {
                    printf("erreur copie recurssive repertoire\n");
                    exit(1);
                }
            }
        }

        closedir(pdirdest);
        closedir(pdirSrc);
    return 1;
}

int creationRepertoireVide(char *repdest,char *repsrc)
{
    int nb;
    struct stat infofichier;
    nb = stat(repsrc,&infofichier);//recuperation des infos du repertoire source
    if(nb<0)
    {
        perror("stat");
        return -1;
    }

    nb = mkdir(repdest,infofichier.st_mode);//creation du repertoire avec le meme mode que la source
    if(nb<0)
    {
        perror("mkdir");
        return -1;
    }
    return 0;
}
//version final
/*commande cat*/
void commande_cat(char *mots_commandes[])
{

    if(strcmp(mots_commandes[1],"-n")==0)
    {
		FILE * fichier = fopen(mots_commandes[2],"r");
        if(fichier==NULL)
        {
            perror("fopen");
            return;
        }
        int nblignes = 0;//numerotation des lignes
        int caractere =0;
        printf("%2d ",nblignes);
        do
        {
            caractere = fgetc(fichier);
            printf("%c",caractere);
            if(caractere=='\n')
            {
                printf("%2d ",nblignes);
                nblignes++;
            }

        }while(caractere!=EOF);
        fclose(fichier);

    } else {
        afficheFichier(mots_commandes[1]);
        if(mots_commandes[2]!=NULL && indice_pipe[0]!='>')
        {
            afficheFichier(mots_commandes[2]);
        }
    }

}


void afficheFichier(char *motCommande)
{
    FILE *fichier = fopen(motCommande,"r");
    if(fichier==NULL)
    {
        perror("fopen");
        return;
    }
    //affichage du contenu du fichier
    int caractere =0;
    do
    {
        caractere = fgetc(fichier);
        printf("%c",caractere);
    }while(caractere!=EOF);
    fclose(fichier);
}




/*commande touch*/
void commande_touch(char *mots_commandes[])
{
    int retour;
    struct utimbuf dates;
    if(mots_commandes[1]==NULL)
    {
        printf("missing file operand\n");
        return;
    }

	FILE* fd=fopen(mots_commandes[1],"w+");
	fclose(fd);

     if(mots_commandes[2]==NULL)//sans option
    {
        time_t tpsActuel = time(NULL);
        if(tpsActuel==-1)
        {
            perror("erreur time");
        }
        //on met la date de lecture et la date de odification à la date actuel
        dates.actime = tpsActuel;
        dates.modtime = tpsActuel;
       retour  = utime(mots_commandes[1],&dates);
    }
    else if(strcmp(mots_commandes[1],"-r")==0)
    {
        //option de modification avec la date d'un fichier de référence
        struct stat buf;
        int retour = stat(mots_commandes[2],&buf);
        if(retour==-1)
        {
            perror("erreur -r");
            return;
        }
        dates.actime = buf.st_atime;//date du dernier acces
        dates.modtime = buf.st_mtime;//date de la derniere modification
	char *temp = mots_commandes[3];
	while(*(mots_commandes[3])!='\0')
	{
		//printf("%c\n",*mots_commandes[3]);
		mots_commandes[3] = (mots_commandes[3])+1;
	}
	mots_commandes[3] = temp;
        retour  = utime(mots_commandes[3],&dates);
    }
    else if(strcmp(mots_commandes[1],"-t")==0)
    {
        struct tm dateconvertie = convert_date(mots_commandes[2]);
        time_t date = mktime(&dateconvertie);
        if(date==-1)
        {
            perror("mktime");
        }else{
            dates.actime = date;
            dates.modtime = date;
            retour = utime(mots_commandes[3],&dates);
        }

    }
    else{
    printf("syntaxe erronée ou option non prise en compte\n");}
    if(retour!=0)
       {
            perror("erreur utime");
       }else{
            printf("touch reussi\n");
       }
}

struct tm convert_date(char *motdate)
{
    struct tm dateconverti;
    /* dateconverti.tm_sec = date%100;
    date/=100;*/
    char segment[2];
    char *copiedate = motdate;
    int nb=1;
    int passage=0;
    dateconverti.tm_sec = 0;//secondes en option
    while(nb!=0&&passage<6)
    {
        segment[0] = *copiedate;
        segment[1] = *(copiedate+1);
        nb = atoi(segment);
        switch(passage)
        {
            case 0 : dateconverti.tm_year = nb+100;break;//on suppose que l année est > 2000 et que le siecle n'est pas indiqué
            case 1 : dateconverti.tm_mon = nb-1;break;
            case 2 :dateconverti.tm_mday = nb;break;
            case 3 : dateconverti.tm_hour = nb;break;
            case 4 : dateconverti.tm_min = nb;break;
            case 5 : dateconverti.tm_sec = nb;break;
        }
        passage++;
        copiedate = copiedate+2;

    }

    return dateconverti;
}

/*commande history*/
void commande_history(char *mots_commande[], int nbcommandeAct)
{
    int i = nbcommandeAct;
    int nb_ligne = nbcommandeAct;
    if(mots_commande[1]!=NULL&&indice_pipe[0]!='>')
    {//option n : affiche les n dernieres lignes
        nb_ligne = atoi(mots_commande[1]);
    }
    i=i-nb_ligne;
    while(i<nbcommandeAct)
    {
        printf("%3d  %s\n",i,history[i]);
        i++;
    }
}

void enregistre_histo(char *mots_commandes,int *nbcommandeAct)
{
    int i =0;
    while(i<MAX_COMMANDES&&history[i]!=NULL)
    {
        i++;
    }
    if(i<MAX_COMMANDES-1)
    {
        history[i+1] = NULL;
    }
    else if(i==MAX_COMMANDES)
    {
    //effacement de la premiere commande si le tableau est plein
        int j;
        for(j=0;j+1<MAX_COMMANDES;j++)
        {
            history[j] = history[j+1];
        }
    }
    history[i] = mots_commandes;
    *nbcommandeAct = i+1;

}

/*commande cd*/
void commande_cd(char *adresse)
{
    int erreur;

    if(strcmp(adresse,"~")==0)
    {
        erreur = chdir("/home");
        if(erreur<0)
        {
            erreur = chdir("/HOME");
        }
    }
    else
    {
        erreur = chdir(adresse);
    }
    if(erreur<0)
    {
        perror("erreur chdir ");
        return;
    }
    printf("cd reussi\n");
}


void enleveCaractere(char *commande)
{
        if(strchr(commande,'\n')!=NULL)
        {
            *strchr(commande,'\n')= 0;
        }
}

void redirection_sortie(char* newout, int* id_stdout)
{
 *id_stdout=dup(STDOUT_FILENO);//duplication sortie standard dans is_stdout
 int id_sortie=open(newout, O_WRONLY| O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
   if(id_sortie<0)
    {
        perror("redirection");
    }
	 dup2(id_sortie,STDOUT_FILENO);//la sortie standard est connecté au fichier
	 if(close(id_sortie) < 0) perror("close");

}

void redirection_sortie_standard(int* id_sortie)
{
	dup2(*id_sortie,STDOUT_FILENO);
}

