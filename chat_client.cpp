#include <iostream>
#include<signal.h>
#include <sys/select.h>
#include <cstdio>
#include <stdio.h>

#include<unistd.h>

#include "client.h"

using namespace std;

fd_set fds_;
void signal_handler(int sig_num);
chat::client c;



int main(int argc, char *argv[]){ 
   signal(SIGINT,signal_handler);
    while(1){

        FD_ZERO(&fds_);
        FD_SET(0, &fds_);
        if(c.get_socket()!=-1)
	        FD_SET(c.get_socket(), &fds_);


        cout<<"Input/Output: "<<endl;

        switch(select(64,&fds_,NULL,NULL,NULL)){
            case -1:
                cerr<<"select error!"<<endl;
                exit(EXIT_FAILURE);
                break;

            case 0:
                cout<<"error"<<endl;
                break;

            default:
                if(c.get_socket()!=-1 && FD_ISSET(c.get_socket(),&fds_)){
                    char receive_buff[256];
                    memset(receive_buff, 0, sizeof(receive_buff));
                    int socket_length = (int)::recv(c.get_socket(), receive_buff, 256, 0);
                    if(socket_length > 0){
                        cout<<receive_buff<<endl;
                    }
                    else if(socket_length < 0 ){
                        cerr<<"Receive message error"<<endl;

                        break;
                    }else{
                        cerr<<"Server exit and message client terminate"<<endl;
                        exit(EXIT_FAILURE);
                    }
                }

                else if(FD_ISSET(0,&fds_)){
                    //cout<< "stdin"<<endl;
                    char receive_buff[256];
                    fgets(receive_buff, 256,stdin);
                    string command =receive_buff;
                    c.command(command);
                }

        }
    }

   return 0;
}

void signal_handler(int sig_num){
    shutdown(c.get_socket(),SHUT_RDWR);
    close(c.get_socket());
    cout<<endl<<"Client is terminated"<<endl;
    exit(EXIT_SUCCESS);
}