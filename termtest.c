#include <stdio.h>
#include <string.h>


#define ASCII_ESC (27)
#define MAX_SINPUT_LEN	40
#define EXIT_STRING "q"			// string to type to exit program

int send(char *s)
{
	// prepend the ESC char to the control string that was entered and send it
	fprintf(stdout, "%c%s", (char)ASCII_ESC, s);
}

int main(int argc, char *argv[])
{
	char sinput[MAX_SINPUT_LEN];

	while(1) {
		// get string
		if(fscanf(stdin,"%s", sinput) > 0){
	
			if (strcmp(EXIT_STRING, sinput) == 0){
				// quit
				return 0;
			}
			else{
				// send string to terminal
				send(sinput);
			}
		}
	}

}



