#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
using namespace std;

int main(){
    char comando[256];
	while(true){ 
		cout<<"Ingresar> "; fflush(stdout);
		cin.getline(comando,sizeof(comando));
		
		if(!strcmp(comando,"salir")){
			return 0;
		} else {
		
			char *token;
			char *tokens[50];
			int i=0; 
			token = strtok (comando, " ");
			
			while (token !=NULL) { //mientras aun haya tokens
				tokens[i]=token; //cada parte se guarda en el arreglo
				i++;
				token=strtok (NULL, " ");
			}
			
			tokens[i]=NULL; //se agrego porque execvp necesita que el ultimo elemento del arreglo sea null
			pid_t returnedValue = fork(); //proceso
			
			if(returnedValue < 0){
				perror("error forking");
				return -1;
			} else if (returnedValue == 0){
			if (comando [0] != '/') { //las lineas siguientes son en caso de que no se especifique ruta y se asume que es en bin
				char rutaCompleta [256];
				strcpy (rutaCompleta, "/bin/");
				strcat(rutaCompleta, tokens [0]);
				//execvp espera un arreglo, esto lo hice para que pueda ejecutar programas con argumentos
				execvp(rutaCompleta, tokens);
			} else { //si se especifica ruta
				execvp(tokens[0], tokens);
				return -1;
			} 
			
			                cout << "Comando incorrecto: " << tokens[0] <<endl;
			                return -1;
			               }
			else {
				if(waitpid(returnedValue, 0, 0) < 0){
					perror("error waiting for child");
					return -1;
				}
			}
		}
	}
    return 0;
}

