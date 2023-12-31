#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <fcntl.h> //libreria para manejar archivos
#include <cstdlib>

using namespace std;

void ejecutarmakefile() {
    char makefilePath[256];

    cout << "Ingrese la ruta completa del Makefile: ";
    cin.getline(makefilePath, sizeof(makefilePath));

    // Construye el comando para ejecutar el makefile
    string makeCommand = "make -f " + string(makefilePath);

    // Ejecuta el comando
    int status = system(makeCommand.c_str());

    if (status == -1) {
        perror("Error al ejecutar make");
        exit(EXIT_FAILURE);
    } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        cout << "Error: El comando make devolvió un código de salida no exitoso: " << WEXITSTATUS(status) << endl;
        exit(EXIT_FAILURE);
    }
}

int main(){
    char comando[256];
	while(true){ 
		cout<<"Ingresar Comando> "; fflush(stdout);
		cin.getline(comando,sizeof(comando));
		
		if(!strcmp(comando,"salir")){
			return 0;
		} else if(!strcmp(comando,"make")){
			ejecutarmakefile();	
			
			//se ejecuta el make file
			
		} else {
			
			char *token;
			char *tokens[50];
			int i=0; 

			token = strtok (comando, " ");
			

			bool dirSalida=false;
			bool dirEntrada=false;
			bool addSalida=false;
			bool usarTuberia=false;
			char nombreArchSalida[200];
			char nombreArchEntrada[200];
			char *argumento2[50];
			bool ejecutarEnSegundoPlano = false;
						
			while (token !=NULL) { //mientras aun haya tokens
				if (strcmp(token, ">") == 0) { // busca símbolo de redirección de salida
				    token = strtok(NULL, " ");
				    if (token != NULL) { // obtiene el siguiente token (nombre del archivo de salida)
				        strcpy(nombreArchSalida, token); // se obtiene el nombre del archivo
				        dirSalida = true;
				    }
				} else if (strcmp(token, "<") == 0) { // busca símbolo de redirección de entrada
				    token = strtok(NULL, " ");
				    if (token != NULL) {
				        strcpy(nombreArchEntrada, token); // se obtiene el nombre del archivo
				        dirEntrada = true;
				    }
				}				
				 else if (strcmp(token, ">>")== 0){ //busca simbolo de añadir contenido a la salida
					token=strtok(NULL, " ");
					if (token != NULL){
						strcpy(nombreArchEntrada, token); //se obtiene el nombre del archivo
						addSalida=true;
					}

				}
				
				else if (strcmp(token, "|") == 0) {
				    usarTuberia = true;
				    token = strtok(NULL, " "); // obtiene el siguiente token (primer argumento del segundo comando)
				    int j = 0;
				    while (token != NULL) { //el segundo comando de la tuberia se va a almacenar en argumento2
				        argumento2[j++] = token; 
				        token = strtok(NULL, " ");
				    }
				    argumento2[j] = NULL; // establece el último elemento del arreglo en NULL (necesario para execvp)
				}
				

				else {
				    tokens[i] = token; // se guarda en el arreglo
				    i++;
				}
				token=strtok (NULL, " ");

				
			}
			
			tokens[i]=NULL; //se agrego porque execvp necesita que el ultimo elemento del arreglo sea null
			

			int copiaSalidaEstandar = dup(1); //se copia el descriptor de la salida estandar para restaurar posteriormente
			
			if(dirSalida) { //si hubo una redireccion de salida
			int fd;
				fd=open(nombreArchSalida, O_WRONLY | O_CREAT | O_TRUNC, 0777); //abre o crea el archivo, modo escritura
			
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
				fd = open (nombreArchEntrada,O_RDONLY); //abre archivo de entrada, modo lectura
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

						dup2(fd[1], 1); //redirigir salida estandar al extrem de escritura de tuberia
						//la salida del primer comando se escriba en la tubería para que el proceso padre pueda leerla
						close(fd[1]);

						execvp(tokens[0], tokens); //ejercutar primer comando
						perror("Error en execvp");
						exit(EXIT_FAILURE);
					} else {
						
						close(fd[1]); //cerrar extremo de escritura 
						//solo va a leer datos del proceso hijo y no escribirá en la tubería
						dup2(fd[0], 0); //redirigir entrada estandar al extremo de lectura de tuberia
						//proceso padre podrá leer la salida del primer comando que se escribió en la tubería por el primer proceso hijo.
						close(fd[0]);

						pid_t returnedValue = fork(); //proceso para el segundo comando
						if (returnedValue == -1) {
						    perror("error al crear proceso");
						    return -1;
						    
						} else if (returnedValue == 0) {
						    execvp(argumento2[0], argumento2); //ejecuta el segundo comando de la tuberia
						    //manejo de errores
						    perror("error en execvp");
						    exit(EXIT_FAILURE);
						} else {
						    int status;
						    waitpid(returnedValue, &status, 0); // espera a que el segundo comando termine
						    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
						        cerr << "El comando no se ejecutó correctamente." << endl;
						    }
						}
					}
			    } 
			    else {
			    	
				if (tokens[i - 1] != NULL && strcmp(tokens[i - 1], "&") == 0) {
					ejecutarEnSegundoPlano = true;
				        tokens[i - 1] = NULL; // elimina el token '&' del arreglo de comandos
				}
					
				pid_t returnedValue = fork(); //creacion de proceso
			
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
