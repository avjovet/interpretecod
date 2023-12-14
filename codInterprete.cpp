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
			
			bool ejecutarEnSegundoPlano = false;
			bool dirSalida=false;
			bool dirEntrada=false;
			bool addSalida=false;
			bool usarTuberia=false;
			char nombreArchSalida[200];
			char nombreArchEntrada[200];
			char *argumento2[50];
			
			while (token !=NULL) { //mientras aun haya tokens
				if (strcmp(token, ">")== 0){ //busca simbolo de redireccion de salida
					token=strtok(NULL, " ");
					if (token != NULL){
						strcpy(nombreArchSalida, token); //se obtiene el nombre del archivo
						dirSalida=true;
						//se quitaron los break para que se pueda hacer redireccion de entrada y 
						//salida al mismo tiempo
						break;
					}
				}
				
				 else if (strcmp(token, "<")== 0){ //busca simbolo de redireccion de entrada
					token=strtok(NULL, " ");
					if (token != NULL){
						strcpy(nombreArchEntrada, token); //se obtiene el nombre del archivo
						dirEntrada=true;

					}
					break;
				}
				
				 else if (strcmp(token, ">>")== 0){ //busca simbolo de añadir contenido a la salida
					token=strtok(NULL, " ");
					if (token != NULL){
						strcpy(nombreArchEntrada, token); //se obtiene el nombre del archivo
						addSalida=true;

					}
					break;
				}
				
				else if (strcmp(token, "|") == 0) {
				    usarTuberia = true;
				    token = strtok(NULL, " ");
				    int j = 0;
				    while (token != NULL) { //el segundo comando de la tuberia se va a almacenar en argumento2
				        argumento2[j++] = token;
				        token = strtok(NULL, " ");
				    }
				    argumento2[j] = NULL;
				    break;

				}
				else {
					tokens[i]=token; //cada parte se guarda en el arreglo
					i++;
					token=strtok (NULL, " ");
					}
				
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
			
			if(addSalida){
			int fd;
				fd=open(nombreArchSalida, O_WRONLY | O_CREAT | O_APPEND, 0777); //abre o crea el archivo 
			
				if (fd==-1){
				
					perror("error al abrir archivo de salida");
					return -1;
				}
				dup2(fd,1); //redirigi la salida estandar al archivo recien abierto
				close(fd); //cierra descriptor de archivo
			}
			
			if (usarTuberia) {
			int fd[2];
				if (pipe(fd) == -1) {
				    perror("Error al crear la tubería");
				    return -1;
				}

				pid_t hijo = fork();

				if (hijo < 0) {
				    perror("error creacion de proceso");
				    return -1;
				} else if (hijo == 0) {
				    close(fd[0]); //cerrar extremo de lectura

				    dup2(fd[1], 1); //redirigir salida estandar al extrem de lectura de tuberia
				    close(fd[1]);

				    execvp(tokens[0], tokens); //ejercutar primer comando
				    perror("Error en execvp");
				    exit(EXIT_FAILURE);
				} else {
					
				    close(fd[1]); //cerrar extremo de escritura 
				    dup2(fd[0], 0); //redirigir entrada estandar al extrem de lectura de tuberia
				    close(fd[0]);

				    pid_t returnedValue = fork(); //proceso para el segundo comando
				    if (returnedValue == -1) {
				        perror("error al crear proceso");
				        return -1;
				        
				    } else if (returnedValue == 0) {
				        execvp(argumento2[0], argumento2); //ejecutra el segundo comando de la tuberia
				        //manejo de errores
				        perror("error en execvp");
				        exit(EXIT_FAILURE);
				    } else {
				        int status;
				        waitpid(returnedValue, &status, 0);
				        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
				            cerr << "El comando no se ejecutó correctamente." << endl;
				        }
				    }
				}
			    } else {
			    	
			if (tokens[i - 1] != NULL && strcmp(tokens[i - 1], "&") == 0) {
                ejecutarEnSegundoPlano = true;
                tokens[i - 1] = NULL; // Elimina el token '&' del arreglo de comandos
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
				if (!ejecutarEnSegundoPlano) {
                    // Espera al proceso hijo si no se está ejecutando en segundo plano
                    if (waitpid(returnedValue, 0, 0) < 0) {
                        perror("error waiting for child");
                        return -1;
                    }
                } else {
                    cout << "Proceso en segundo plano ejecutando el comando." << endl;
                }
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
				
				if(addSalida){ //restaurar salida estandar para que no mande el promp al archivo de salida
					dup2(copiaSalidaEstandar, 1);
					close(copiaSalidaEstandar);
				}
				
				if (usarTuberia){
            				dup2(copiaEntradaEstandar, 0);
                        		close(copiaEntradaEstandar);
				}
			            
		}
	}
    return 0;
}
