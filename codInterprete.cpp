#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <fcntl.h> //libreria para manejar archivos

using namespace std;

int main(){
    char comando[256];
	while(true){ 
		cout<<"Ingresar Comando> "; fflush(stdout);
		cin.getline(comando,sizeof(comando));
		
		if(!strcmp(comando,"salir")){
			return 0;
		} else {
		
			char *token;
			char *tokens[50];
			int i=0; 

			token = strtok (comando, " ");
			
			bool dirSalida=false;
			bool dirEntrada=false;
			char nombreArchSalida[200];
			char nombreArchEntrada[200];
			
			while (token !=NULL) { //mientras aun haya tokens
				if (strcmp(token, ">")== 0){ //busca simbolo de redireccion de salida
					token=strtok(NULL, " ");
					if (token != NULL){
						strcpy(nombreArchSalida, token); //se obtiene el nombre del archivo
						dirSalida=true;
						break; //si ya se encontro el nobre del archivo ya no se sigue buscando mas tokens
					}
				}
				
				if (strcmp(token, "<")== 0){ //busca simbolo de redireccion de entrada
					token=strtok(NULL, " ");
					if (token != NULL){
						strcpy(nombreArchEntrada, token); //se obtiene el nombre del archivo
						dirEntrada=true;
						break; //si ya se encontro el nobre del archivo ya no se sigue buscando mas tokens
					}
				}
				
				tokens[i]=token; //cada parte se guarda en el arreglo
				i++;
				token=strtok (NULL, " ");
			}
			
			tokens[i]=NULL; //se agrego porque execvp necesita que el ultimo elemento del arreglo sea null
			

			int copiaSalidaEstandar = dup(1); //se copia el descriptor de la salida estandar para restaurar posteriormente
			
			if(dirSalida) { //si hubo una redireccion de salida
			int fd;
				fd=open(nombreArchSalida, O_WRONLY | O_CREAT | O_TRUNC, 0777); //abre o crea el archivo 
			
				if (fd==-1){
				
					perror("error al abrir archivo de salida");
					return -1;
				}
				dup2(fd,1); //redirigi la salida estandar al archivo recien abierto
				close(fd); //cierra descriptor de archivo
					
			}
			
			int copiaEntradaEstandar = dup(0); //se copia el descriptor de la entrada estandar para restaurar posteriormente
			
			if(dirEntrada){
			int fd;
				  fd = open (nombreArchEntrada,O_RDONLY); //abre archivo de entrada
				  	if (fd==-1){
						perror("error al leer archivo de entrada");
					return -1;
				}
				
				dup2(fd,0); //redirigi la entrada estandar al archivo recien abierto
				close(fd);
			}
			pid_t returnedValue = fork(); //proceso
			
			if(returnedValue < 0){
				perror("error forking");
				return -1;
			} else if (returnedValue == 0){
			
				if (comando [0] != '/') { //las lineas siguientes son en caso de que no se especifique ruta y se asume en bin
					char rutaCompleta [256];
					strcpy (rutaCompleta, "/bin/");
					strcat(rutaCompleta, tokens [0]);
					//execvp espera un arreglo, esto lo hice para que pueda ejecutar programas con argumentos
					execvp(rutaCompleta, tokens);
				} else { //si se especifica ruta
					execvp(tokens[0], tokens);
					return -1;
				} 
			
			     cout << "Comando " << tokens[0] << " incorrecto" <<endl;
			     return -1;
			}
			else {
				if(waitpid(returnedValue, 0, 0) < 0){
					perror("error waiting for child");
					return -1;
				}
			}
				
				if(dirSalida){ //restaurar salida estandar para que no mande el promp al archivo de salida
					dup2(copiaSalidaEstandar, 1);
					close(copiaSalidaEstandar);
				}
				
				if(dirEntrada){
					dup2(copiaEntradaEstandar, 0);
					close(copiaEntradaEstandar);
				}
			
		}
	}
    return 0;
}

