#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SERVER_PORT 5432
#define MAX_PENDING 1
#define MAX_LINE 256

const char *unsigned_to_binary(unsigned long msg, int size)//takes in an unsigned long int of size "size." Then it transfers it into binary (if it isnt in binary already) and then formats it into a string
{
	char* b = (char *)malloc(size + 1); //allocating space for a buffer to place the input.
	b[0] = '\0'; //Adding an end of line character

	int z; //create z
	for (z = (1<<(size-1)); z > 0; z >>= 1) //for (set z equal to the size-1 left shifted once; when z > 0; shift z to the right once and assign result to z);
	{
		strcat(b, ((msg & z) == z) ? "1" : "0"); //if msg bitwise AND z are equal to z, then add a "1" to b, otherwise add a "0"
	}

	return b; //return b
}

unsigned long crc_remainder(unsigned long msg, unsigned long check, int mlen, int clen) {//intakes an unsigned long msg and an unsigned long check (divisor) and finds the remainder of msg / check.
	unsigned long newmsg = msg << (clen-1);// newmsg is set equal to msg that has been left shifted (clen-1) times
	unsigned long n;//create n
	int i;//create i
	for (i = mlen; i > 0; i--) {//for loop that executes as long as i is not 0. i initially set to length of msg
		if ((newmsg & (1 << (i+clen-2))) != 0) { //if newmsg bitwise AND (1 left shifted (i+length of check-2)) do not equal 0, then execute "if" statement, else return newmsg bitwise AND ((1 left shifted length of check times) -1)
			n = check << (i - 1); //sets n equal to check left shifted (i-1) times
			newmsg = newmsg ^ n; //sets newmsg equal to newmsg XOR n. copies the bit if it is set in one operand but not both
		}
	}
	return newmsg & ((1 << clen) - 1);//see line 24
}

int main()
{
	struct sockaddr_in sin;
	char buf[MAX_LINE];
	int len;
  unsigned int l;
	int s, new_s;

	/* build address data structure */
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(SERVER_PORT);

	/* setup passive open */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("simplex-talk: socket");
		exit(1);
	}
	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
		perror("simplex-talk: bind");
		exit(1);
	}
	listen(s, MAX_PENDING);

	/* wait for connection, then receive and print text */
	while(1) {
		l = sizeof(sin);
		if ((new_s = accept(s, (struct sockaddr *)&sin, &l)) < 0) {
			perror("simplex-talk: accept");
			exit(1);
		}
		while ((len = recv(new_s, buf, sizeof(buf), 0))) {
			fputs("", stdout);//If I don't have this fputs, nothing prints and I have no idea why so I made it put nothing. Stubborn fputs.
			int totalServ = 0;
			int totalClient = 0;
			int i = 0;

			while (buf[i] != ';') {
				if (buf[i] == '1') {
					totalServ ++;
					i++;
				}
				else {
					i++;
				}
			}
			i++;
			totalClient = buf[i] - '0';
			printf("Client total is %d\n", totalClient);
			printf("Server total is %d\n", totalServ);
			if (totalClient != totalServ) {
				printf("Error: client data has been compromised!\n");
			}
			else {
				printf("Client data is intact.\n");
			}
		}
		close(new_s);
	}
}
#CheckSum Algorithm
