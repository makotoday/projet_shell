#include "find.h"
#include "commande.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>

int premierefois=0;
char* full_path;

void find (char * dir_name)
{
  // on ouvre le repertoire dir_name
    DIR * d;
  	if(premierefois==0){
		char original_path[] = "/home/sebastien";
		full_path = malloc( MAX_PATH*sizeof(char));
		strcpy(full_path,original_path);
		//strcat(full_path,dir_name);//On ajoute le nom du dossier à la fin du path
		printf("%s\n",full_path);
        premierefois=1;
		d = opendir (full_path);
		}
		else {
            d = opendir (dir_name);
		}
		  if(d==NULL)
            {
            // printf("impossible d'ouvrir %s\n",full_path);
                return;
            }
    while (1) {

        struct dirent * pdir = readdir (d);
        if(pdir==NULL){
             break;//repertoire vide
             }

			char *nom_fichier = pdir->d_name;
			if (pdir->d_type & DT_DIR&&nom_fichier[0]!='.') {
				 //readdir lit d'abord le fichier parent et le fichier actuel.
						char path[2*MAX_PATH];
                        strcpy(path,full_path);
						strcat(path,"/");
						strcat(path,nom_fichier);
						printf("%s\n",nom_fichier);
						find(path); // appel récursive de la fonction find
			}
			else
				printf("%s\n",dir_name);
    }//end while
     closedir (d); // fermeture du repertoire
}
