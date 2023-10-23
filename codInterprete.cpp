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
			int fd;
			token = strtok (comando, " ");
			
			bool dirSalida=false;
			char nombreArchSalida[200];
			
			while (token !=NULL) { //mientras aun haya tokens
				if (strcmp(token, ">")== 0){ //busca simbolo de redireccion de salida
				token=strtok(NULL, " ");
				if (token != NULL){
					strcpy(nombreArchSalida, token); //se obtiene el nombre del archivo
					dirSalida=true;
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
			fd=open(nombreArchSalida, O_WRONLY | O_CREAT | O_TRUNC, 0777); //abre o crea el archivo 
			
			dup2(fd,1); //redirigi la salida estandar al archivo recien abierto
			close(fd); //cierra descriptor de archivo
				
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
				
				if(dirSalida){
				dup2(copiaSalidaEstandar, 1);
				close(copiaSalidaEstandar);
				}
			}
		}
	}
    return 0;
}

