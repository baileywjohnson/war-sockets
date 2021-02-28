#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>

#define SOCKET_PATH "WAR_socket"

//"Draws" a rank that represents a card value/suit
int draw(int drawtype){
	if(drawtype == 0){
		int value = (rand() % (13 - 1 + 1)) + 1;
		return value;
	}
	else{
		int value = (rand() % (17 - 14 + 1)) + 14;
		return value;
	}
}


//Matches a rank to a given string value for output
char *getValue(int rank){
	char *value = malloc(10);
	switch(rank){
		case 1: value = "Ace"; break;
		case 11: value = "Jack"; break;
		case 12: value = "Queen"; break;
		case 13: value = "King"; break;
		case 14: value = "Spades"; break;
		case 15: value = "Hearts"; break;
		case 16: value = "Diamonds"; break;
		case 17: value = "Clubs"; break;
		default: sprintf(value, "%d" ,rank); break;
	}
	return value;
}


void do_parent(long rounds, int child1_pid, int child2_pid){
	//Create Parent Socket
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("socket error\n");
        exit(-1);
    }

    //Setup Socket Address
    struct sockaddr_un serv_addr;
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, SOCKET_PATH);

    //"clear out old garbage"
    unlink(SOCKET_PATH);

    //Bind Socket
    int size = sizeof(serv_addr);
    if(bind(sockfd, (struct sockaddr *) &serv_addr, size) < 0 ){
        perror("bind error\n");
        exit(-1);
    }

    //Start Listening on Socket
    if(listen(sockfd, 1) < 0){
        perror("listen error\n");
        exit(-1);
    }

    //Answer Children (This Blocks)
    int c1 = accept(sockfd, NULL, NULL);
    if( c1 < -1){
        perror("accept error\n");
        exit(-1);
    }
    int c2 = accept(sockfd, NULL, NULL);
    if( c2 < -1){
        perror("accept error\n");
        exit(-1);
    }

	printf("Child 1 PID: %i\n", child1_pid);
	printf("Child 2 PID: %i\n", child2_pid);
    printf("Beginning %ld rounds... (Ace is LOW CARD)\n(Spades > Hearts > Diamonds > Clubs)\nFight!\n---------------------------\n", rounds);

    int child1_wins = 0;
	int child2_wins = 0;

	//Main Round Loop
	int i, signal;
	for(i=0; i < rounds; i++){
		printf("Round %i:\n", i);

		signal = 0;
		write(c1, &signal, sizeof(int));
		write(c2, &signal, sizeof(int));

		int result1[2], result2[2];
		read(c1, result1, sizeof(int)*2);
		read(c2, result2, sizeof(int)*2);

		if(result1[1] == child1_pid){
			//Determine Win/Tie
			printf("Child 1 draws %s\n", getValue(result1[0]));
			printf("Child 2 draws %s\n", getValue(result2[0]));
			if(result1[0] > result2[0]){
				printf("Child 1 Wins!\n");
				child1_wins += 1;	
			}
			else if(result2[0] > result1[0]){
				printf("Child 2 Wins!\n");
				child2_wins += 1;
			}
			//Tie Occurs- Call to Children to Retrieve Suit
			else{
				printf("Checking suit...\n");
				signal = 1;
				write(c1, &signal, sizeof(int));
				write(c2, &signal, sizeof(int));
				
				int result3[2], result4[2];
				read(c1, result3, sizeof(int) * 2);
				read(c2, result4, sizeof(int) * 2);

				//If the First Return Was Child 1
				if(result3[1] == child1_pid){
					printf("Child 1 draws %s %s\n", getValue(result1[0]), getValue(result3[0]));
					printf("Child 2 draws %s %s\n", getValue(result2[0]), getValue(result4[0]));
					if(result3[0] < result4[0]){
						printf("Child 1 Wins!\n");
						child1_wins += 1;	
					}
					else if(result4[0] < result3[0]){
						printf("Child 2 Wins!\n");
						child2_wins += 1;
					}
					else{
						printf("It's a Tie!\n");
					}
				}

				//If the First Return Was Child 2
				else{
					printf("Child 1 draws %s %s\n", getValue(result2[0]), getValue(result4[0]));
					printf("Child 2 draws %s %s\n", getValue(result1[0]), getValue(result3[0]));
					if(result4[0] < result3[0]){
						printf("Child 1 Wins!\n");
						child1_wins += 1;
					}
					else if(result3[0] < result4[0]){
						printf("Child 2 Wins!\n");
						child2_wins += 1;
					}
					else{
						printf("It's a Tie!\n");
					}
				}
			}
		}
		else{
			//Determine Win/Tie
			printf("Child 1 draws %s\n", getValue(result2[0]));
			printf("Child 2 draws %s\n", getValue(result1[0]));
			if(result2[0] > result1[0]){
				printf("Child 1 Wins!\n");
				child1_wins += 1;
			}
			else if(result1[0] > result2[0]){
				printf("Child 2 Wins!\n");
				child2_wins += 1;
			}
			//Tie Occurs- Call to Children to Retrieve Suit
			else{
				printf("Checking suit...\n");
				signal = 1;
				write(c1, &signal, sizeof(int));
				write(c2, &signal, sizeof(int));
				
				int result3[2], result4[2];
				read(c1, result3, sizeof(int) * 2);
				read(c2, result4, sizeof(int) * 2);

				//If the First Return Was Child 1
				if(result3[1] == child1_pid){
					printf("Child 1 draws %s %s\n", getValue(result2[0]), getValue(result3[0]));
					printf("Child 2 draws %s %s\n", getValue(result1[0]), getValue(result4[0]));
					
					if(result3[0] < result4[0]){
						printf("Child 1 Wins!\n");
						child1_wins += 1;	
					}
					else if(result4[0] < result3[0]){
						printf("Child 2 Wins!\n");
						child2_wins += 1;
					}
					else{
						printf("It's a Tie!\n");
					}
				}

				//If the First Return Was Child 2
				else{
					printf("Child 1 draws %s %s\n", getValue(result1[0]), getValue(result4[0]));
					printf("Child 2 draws %s %s\n", getValue(result2[0]), getValue(result3[0]));
					if(result4[0] < result3[0]){
						printf("Child 1 Wins!\n");
						child1_wins += 1;
					}
					else if(result3[0] < result4[0]){
						printf("Child 2 Wins!\n");
						child2_wins += 1;
					}
					else{
						printf("It's a Tie!\n");
					}
				}
			}
		}

		printf("---------------------------\n");
	}

	//Signal Children to Exit
	signal = 2;
	write(c1, &signal, sizeof(int));
	write(c2, &signal, sizeof(int));


	//Display Match Results & Determine Winner
	printf("Results:\n");
	printf("Child 1: %i\n", child1_wins);
	printf("Child 2: %i\n", child2_wins);
	if(child1_wins > child2_wins){
		printf("Child 1 Wins!\n");
	}
	else if(child2_wins > child1_wins){
		printf("Child 2 Wins!\n");
	}
	else{
		printf("It's a tie!\n");
	}

    //reap children
    wait(NULL);
        
    //close socket
    close(sockfd);
}

void do_child(int pid){
	//Create Child Socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("socket error\n");
        exit(-1);
    }

    //Setup Socket Address
    struct sockaddr_un sa;
    sa.sun_family = AF_UNIX;
    //strncpy(sa.sun_path, SOCKET_PATH, sizeof(sa.sun_path)-1);
    strcpy(sa.sun_path, SOCKET_PATH);

    //length = strlen(sa.sun_path) + sizeof(sa.sun_family);
    int length = sizeof(sa);

    int i, connect_check;
    for(i=0; i<5; i++){
        connect_check = connect(sockfd, (struct sockaddr *) &sa, length);
        if(connect_check < 0) {
            usleep(10000);
            //perror("do_child connect failed, about to try again...\n");
        }
        else{
            break; //leave loop if successful
        }
    }

    if(connect_check == -1){
            perror("connect error: unable to connect\n");
            exit(-1);
    }

    srand(time(NULL)*pid);
    while(1){
    	int signal;
		read(sockfd, &signal, sizeof(int));
		if(signal != 2){
			int result = draw(signal);
			int buffer[2] = {result, pid};
			write(sockfd, buffer, sizeof(int)*2);
		}
		else{
    		close(sockfd);
    		exit(0);
		}
    }
}

int main(int argc, char* argv[]){
	int parentPID = getpid();

	//User Input for Number of Rounds
	if((argc == 1) || (argc > 2)){
		printf("USAGE: 'war <# of rounds to play (int)>'");
		exit(1);
	}
	long rounds = strtol(argv[1], NULL, 0);

	//Process Forking
	int i, pid, child1_pid, child2_pid;
	for(i=0; i < 2; i++){
		pid = fork();
		if(pid > 0){
			if(i == 0){
				child1_pid = pid;
			}
			else{
				child2_pid = pid;
			}
			continue;
		}
		else if(pid == 0){
			break;
		}
		else{
			printf("Error Forking a New Process\n");
			exit(1);
		}
	}

	pid = getpid();
	//Parent Behavior
	if(pid == parentPID){
		do_parent(rounds, child1_pid, child2_pid);
	}
	//Child Behavior
	else{
		do_child(pid);
	}
}
