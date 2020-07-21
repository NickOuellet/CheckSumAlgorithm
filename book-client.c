#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 5432
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

int main(int argc, char * argv[])
{
	FILE *fp;
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	char buf[MAX_LINE];
	int s;
	int len;

	if (argc==2) {
		host = argv[1];
	}
	else {
		fprintf(stderr, "usage: simplex-talk host\n");
		exit(1);
	}

	/* translate host name into peer's IP address */
	hp = gethostbyname(host);
	if (!hp) {
		fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
		exit(1);
	}

	/* build address data structure */
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
	sin.sin_port = htons(SERVER_PORT);

	/* active open */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("simplex-talk: socket");
		exit(1);
	}
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		perror("simplex-talk: connect");
		close(s);
		exit(1);
	}
	/* main loop: get and send lines of text */
  printf("Send a message to the server: ");
	while (fgets(buf, sizeof(buf), stdin)) {
		int i;
		int total = 0;

		buf[strcspn(buf, "\n")] = 0;

		len = strlen(buf);
		char totalString[10];
		for (i = 0; i <= len; i++) {
	    if (buf[i] == '1') {
	      total++;
	    }
	    else {
	      //do nothing;
	     }
	  }
		sprintf(totalString, "%d", total);

		strcat(buf, ";");
		strcat(buf, totalString);
		strcat(buf, "\n");

		buf[MAX_LINE-1] = '\0';
		len = strlen(buf) + 1;

		char finalBuf[MAX_LINE];
		strcpy(finalBuf,""); //ensures that this string is empty
		strcpy(finalBuf, buf);


		int randomNumber = rand();//Section that randomizes bits to simulate errors
		if (randomNumber % 3 == 0) {
			int randomNumber2 = rand() % (len-3);
			int randomBit = finalBuf[randomNumber2] - '0'; //this line is needed to force type integers. Otherwise it spits out address
			if (randomBit == 0) {
				finalBuf[randomNumber2] = '1';
			}
			else {
				finalBuf[randomNumber2] = '0';
			}
			printf("ERROR!\nOriginal msg: %s", buf);
			printf("Compromised msg: %s", finalBuf);
		}

		send(s, finalBuf, len, 0);
    printf("Send a message to the server: ");
	}
}
