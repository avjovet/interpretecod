#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

int main(){
    char command[256];
	while(true){ 
        cout<<"Command (one word only)> "; fflush(stdout);
        cin>>command;
		if(!strcmp(command,"exit")){
			return 0;
		} else {
			pid_t returnedValue = fork();
			if(returnedValue < 0){
				perror("error forking");
				return -1;
			} else if (returnedValue == 0){
				execlp(command, command, NULL);
				return -1;
			} else {
				if(waitpid(returnedValue, 0, 0) < 0){
					perror("error waiting for child");
					return -1;
				}
			}
		}
	}
    return 0;
}

