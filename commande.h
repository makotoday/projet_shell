#define MAX_PATH 200  //longueur du nom d'un fichier

#define MAX_BUFF 300 //copie des caracteres d'un fichier
#define MAX_BUFF_COMMANDE 300
#define MAX_COMMANDES 30
#define MAXPIPE 21 //nombre maximum de pipes ou redirections

#ifndef COMMANDE_H_INCLUDED
#define COMMANDE_H_INCLUDED

char *history[MAX_COMMANDES];
int nbcommandeAct;

int indice_pipe[MAXPIPE];// tableau d'entier indicant la présence de pipe ou de >  et leur indice

void enleveCaractere(char *commande);
void commande_cd(char *mots);
char *tronquage(char *mots);
void enregistre_histo(char *mots_commandes, int *nbcommandeAct);
void commande_history(char *mots_commande[], int nbcommandeAct);

void commande_touch(char *mots_commandes[]);
struct tm convert_date(char* date);

//fonctions commande copie :
void commande_copie(char *mots_commandes[]);
/*Etape 1 : copie d'un seul fichier*/
int copie_unfichier(char path[MAX_PATH+1],char fichier_copie[MAX_PATH+1]);
int copie(int file1,int filecopie);
/*etape 2 : copie des droits*/
int copie_droit(int file1,int filecopie);
/*etape 3 : copie des répertoires*/
void ajoutDernierMotSource(char **repertoireDest,char *source);
int copie_repertoire(char repertoire1[MAX_PATH+1],char repertoire_existe[MAX_PATH+1]);
int creationRepertoireVide(char *repdest,char *repsrc);
void redirection_sortie(char* newout, int* id_stdout); // pour les commande externe
void redirection_sortie_standard(int* id_sortie);

void commande_cat(char *mots_commandes[]);
void afficheFichier(char *motCommande);
#endif // COMMANDE_H_INCLUDED

