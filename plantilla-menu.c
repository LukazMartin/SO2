/**
 *
 * Practica 4
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "red-black-tree.h"
#include "linked-list.h"
#include <sys/time.h>
#define MAXLINE      200
#define MAGIC_NUMBER 0x0133C8F9
#define bloc 1000
 /**
 * 
 *  Menu
 * 
 */
int num;
int menu();
pthread_mutex_t lock;
 struct afegirDades {
	FILE *fitxer;
	rb_tree *tree;
};
/*Funcions de l'opció 1 crear Arbre*/
char* getColumn(char *str, int columna){
    int count = 0,i,j=0;
    char *valor = calloc(4,sizeof(char));
    for(i = 0; i < strlen(str);i++){
		if(count == columna-1){
		    while(str[i] != ','){	
				valor[j] = str[i];
				j++;
				i++;
		    }
	    }
	    if(str[i]==','){
		    count++;
        }
    }
	return valor;
}
rb_tree *crearArbre(char * aeroports){
	int i;
	list *l;
	list_data *l_data;
	list_item *l_item;
	FILE *fp;
	char str[100];
  	node_data *n_data;
	rb_tree *tree;
 	fp = fopen(aeroports,"r");
	if(fp == NULL){
		perror("Could not open file");
		exit(-1);
	}
	if(fgets(str,100,fp) != NULL){
		num = atoi(str);
		tree = malloc(sizeof(rb_tree));
		init_tree(tree);
		for(i = 0;i<num;i++){
			fgets(str,100,fp);
			n_data = malloc(sizeof(node_data));
			n_data->key = malloc(sizeof(char)*(strlen(str)+1));
			str[strlen(str)-1] = '\0';
			strcpy(n_data->key,str);
			insert_node(tree,n_data);
			l = (list *)malloc(sizeof(list));
			n_data->list = l;
            pthread_mutex_init(&n_data->lock,NULL);
			init_list(n_data->list);
		}
	}
	fclose(fp);
	
	return tree;
}
int *afegirDades(void *arg){
	char *delay, *orig, *dest;
  	node_data *n_data;
	list_data *l_data;
	char str2[5000];
 	struct afegirDades *dades = (struct afegirDades *) arg;
	
	int linies = 0;
	while(linies < bloc){
		linies++;
		pthread_mutex_lock(&lock);
		if (fgets(str2,5000,dades->fitxer) == NULL){
			break;
		}
		pthread_mutex_unlock(&lock);
		delay = getColumn(str2,15);
		orig = getColumn(str2,17);
		dest = getColumn(str2,18);
		n_data = find_node(dades->tree,orig);
		pthread_mutex_lock(&n_data->lock);
		l_data = find_list(n_data->list, dest);
		if(l_data == NULL){
			l_data = malloc(sizeof(list_data));
			l_data->key = dest;
			if(strcmp(delay,"NA") ==0){
				l_data->delay = 0;
			}
			else{	
				l_data->delay = atof(delay);
			}
			l_data->num_vols = 1;
			insert_list(n_data->list,l_data);
		}
		else{
			if(strcmp(delay,"NA") !=0){
				l_data->delay += atof(delay);
			}	
			l_data->num_vols += 1;	
			free(dest);	
		}
		pthread_mutex_unlock(&n_data->lock);
		free(delay);
		free(orig);
		
	}
	pthread_mutex_unlock(&lock);
	return ((void*) "Dades llegides\n");
}
/*Funció de l'opció 2 guardar Arbre a disc*/
void guardarArbre(node *child,FILE *fp)
{
    if (child->right != NIL)
	guardarArbre(child->right,fp);
     if (child->left != NIL)
	guardarArbre(child->left,fp);
     fwrite(child->data->key, strlen(child->data->key), 1, fp);
    int num_destins = child->data->list->num_items;
    fwrite(&num_destins,sizeof(int),1,fp);
    if(num_destins > 0){
	list_item *l_item = child->data->list->first;
	int a;
	float f;
	while(l_item){
		fwrite(l_item->data->key,strlen(l_item->data->key),1,fp);
		a = l_item->data->num_vols;
		fwrite(&a,sizeof(int),1,fp);
		f = l_item->data->delay;
		fwrite(&f,sizeof(float),1,fp);
		l_item = l_item->next;
	}
     }
}
/*Funció per l'opció 3 carregar arbre de disc*/
rb_tree *carregarArbre(FILE *fp){
	char buffer[100];
	int i,j1,num_nodes,num_destins,maxDest = 0,magic;
	float x;
	char *aux;
	node_data *n_data;
	list_item *l_item;
	list_data *l_data;
	list *l;
	rb_tree *tree;
 	tree = malloc(sizeof(rb_tree));
	init_tree(tree);
	fread(&magic,sizeof(int),1,fp);
	if(magic != MAGIC_NUMBER){
		perror("Error en el nombre Magic");
		exit(-1);
	}
	fread(&num,sizeof(int),1,fp);
	for(num_nodes=0;num_nodes<num;num_nodes++){
 		fread(buffer, 3, 1, fp);
		buffer[3] = '\0';       
		aux = (char*)buffer;
		fread(&num_destins, sizeof(int), 1, fp); 
		n_data = malloc(sizeof(node_data));
		n_data->key = malloc(sizeof(char)*(strlen(aux)+1));
		strcpy(n_data->key,aux);
		l = (list *)malloc(sizeof(list));
		n_data->list = l;
		init_list(n_data->list);
		insert_node(tree,n_data);
		for(i=0;i<num_destins;i++){
			fread(buffer,3,1,fp);
			buffer[3] = '\0';    
			l_data = malloc(sizeof(list_data));
			aux = malloc(sizeof(char)*4);
			strcpy(aux,buffer);
			l_data->key = aux;
			fread(&j1,sizeof(int),1,fp);
			l_data->num_vols = j1;
			fread(&x,sizeof(float),1,fp);
			l_data->delay = x;
			insert_list(n_data->list,l_data);
		}
	}
	return tree;
}
/*Funcions per l'opció 4*/
 //Si s'ha introduït aeroport per teclat
void printRetard(char * IATA,rb_tree *tree){
	node_data *n_data;
	list_item *l_item;
	n_data = find_node(tree,IATA);
	if(n_data != NULL){
		l_item = n_data->list->first;
 		printf("Media de retardos para %s\n",IATA);
		printf("Retardos para el aeropuerto: %s\n",IATA);
		while(l_item){
			printf("   %s  --  %.3f minutos\n",(char*)l_item->data->key,l_item->data->delay/l_item->data->num_vols);
			l_item = l_item->next;
		}
	}
	else{
		printf("El aeropuerto %s no existe en arbol",(char *)IATA);
	}
}
//Si s'ha introduït enter
void recursion(rb_tree *t,node *child,char *res){
	
	if(child->right != NIL){
		recursion(t,child->right,res);
	}
	if(child->left != NIL){
		recursion(t,child->left,res);
	}
	if(child->data->list->num_items > find_node(t,res)->list->num_items){
		strcpy(res,child->data->key);
	}
}
char *massDestinos(rb_tree *tree){
	node *n = tree->root;
	char *r;
	r = malloc(sizeof(char)*4);
	strcpy(r,n->data->key);
	recursion(tree,n,r);
	return r;
}
 int main(int argc, char **argv)
{
    char str1[MAXLINE], str2[MAXLINE],str3[5000],strl[MAXLINE];
    int opcio;
    FILE *fp;
    rb_tree * tree = NULL;
    
    struct timeval tv1, tv2;
    clock_t t1, t2;

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    if (argc != 1)
        printf("Opcions de la linia de comandes ignorades\n");
    do {
        opcio = menu();
        printf("\n\n");
        switch (opcio) {
            case 1:

			if(tree != NULL){
				printf("Alliberant arbre\n\n");
				delete_tree(tree);
				free(tree);
			}
		        printf("Introdueix fitxer que conte llistat d'aeroports: ");
		        fgets(str1, MAXLINE, stdin);
		        str1[strlen(str1)-1]=0;
		        printf("Introdueix fitxer de dades: ");
		        fgets(str2, MAXLINE, stdin);
		        str2[strlen(str2)-1]=0;
			tree = crearArbre(str1);
			pthread_t tid[10];
			int a;
			char * b;
			/* Ara anem a llegir quantes linies te el fitxer per saber quants
			   fils haurem de crear per llegir-lo sencer */

			fp = fopen(str2,"r");
			if(fp==NULL){
				perror("Could not open file");
				exit(-1);
			}
			int lanes = 0;

			while(fgets(strl, MAXLINE, fp) != NULL){
				lanes++;
			}

			if(lanes%1000!=0){
				lanes = lanes/1000 + 1;
			}

			/*********************************************/

			fclose(fp);

			gettimeofday(&tv1, NULL);
			t1 = clock();

			fp = fopen(str2,"r");
			
			fgets(str3,5000,fp); //Llegim capçelera
			struct afegirDades aDades;
			aDades.fitxer = fp;
			aDades.tree = tree;
			int i = 0;
			while (i < lanes){
				a = pthread_create(&(tid[i]),NULL,(void*)afegirDades,(void*)&aDades);
				if(a!=0){
					printf("Error");
					exit(-1);
				}
				i++;
			}

			i=0;
			while(i<lanes){pthread_join(tid[i],(void **)&b);i++;}
			pthread_mutex_destroy(&lock);
			printf("%s",b);

			gettimeofday(&tv2, NULL);
			t2 = clock();

			printf("Temps de CPU per llegir i assignar totes les dades: %fs\n",(double)(t2-t1)/(double)CLOCKS_PER_SEC);
			printf("Temps cronologic: %fs\n",(double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +(double) (tv2.tv_sec - tv1.tv_sec));

                break;
             case 2:
				if(tree == NULL){
					printf("Primer has de crear un arbre\n");
				}
				else{
			        printf("Introdueix el nom de fitxer en el qual es desara l'arbre: ");
			        fgets(str1, MAXLINE, stdin);
			        str1[strlen(str1)-1]=0;
 			
		    		fp = fopen(str1,"w");
		    		int magic = MAGIC_NUMBER;
					fwrite(&magic,sizeof(int),1,fp);
		    		fwrite(&num,sizeof(int),1,fp);
					guardarArbre(tree->root,fp);
	    			printf("Arbre guardat\n");
		    		fclose(fp);
				}
                break;
             case 3:
                printf("Introdueix nom de fitxer que conte l'arbre: ");
                fgets(str1, MAXLINE, stdin);
                str1[strlen(str1)-1]=0;
				fp = fopen(str1,"r");
				if(tree != NULL){
					delete_tree(tree);
					free(tree);
					printf("Alliberant arbre\n");
				}
				tree = carregarArbre(fp);
				fclose(fp);
                break;
             case 4:
				if(tree == NULL){
					printf("No hi ha arbre creat.\n");
				}
				else{
			        printf("Introdueix aeroport per cercar retard o polsa enter per saber l'aeroport amb mes destins: ");
			        fgets(str1, MAXLINE, stdin);
			        str1[strlen(str1)-1]=0;
					if(strlen(str1) == 0){
						char *md;
						md = massDestinos(tree);
						printf("\nAeropuerto con mas destinos %s.\n",md);
						printf("\nCon un total de %d vuelos.\n",find_node(tree,md)->list->num_items);
						free(md);
					}
					else{
						printRetard(str1,tree);
					}
				}
                break;
             case 5:
            	if(tree != NULL){
            		delete_tree(tree);
					free(tree);
            	}
                break;
             default:
                printf("Opcio no valida\n");
         } 
    }
    while (opcio != 5);
    return 0;
}
int menu() 
{
    char str[5];
    int opcio;
     printf("\n\nMenu\n\n");
    printf(" 1 - Creacio de l'arbre\n");
    printf(" 2 - Emmagatzemar arbre a disc\n");
    printf(" 3 - Llegir arbre de disc\n");
    printf(" 4 - Consultar informacio de l'arbre\n");
    printf(" 5 - Sortir\n\n");
    printf("   Escull opcio: ");
     fgets(str, 5, stdin);
    opcio = atoi(str); 
     return opcio;
} 
