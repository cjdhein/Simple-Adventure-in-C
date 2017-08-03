#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

pthread_mutex_t mtx1 = PTHREAD_MUTEX_INITIALIZER;


// Room struct
typedef struct temp{
	char* name;
	char* type;

	// hold names of room's connections
	char *connectionNames[6]; 

	// index of the next empty connection
	int nextConnIndex;

	// number of active connections
	int connectionCount;

} Room;


// Holds the 7 rooms representing the map
static Room roomArray[7];

//constants for the number of rooms and char length of their names
static const int numOfRooms = 10;
static const int nameLength = 20;

// allocates memory for elements' name, type and connectionNames
// along with initializing other variables to proper initial values
void initRoomArray()
{
	// for loop counters
	int i, j;

	// loop through each element of roomArray
	for(i = 0; i < 7; i++)
	{
		// alloc memory for name and type
		roomArray[i].name = malloc(sizeof(char)*nameLength);
		roomArray[i].type = malloc(sizeof(char)*nameLength);

		// initialize to starting values
		roomArray[i].connectionCount = 0;
		roomArray[i].nextConnIndex = 0;

		// loop through each potential connection
		// and allocate memory for the name
		for(j = 0; j < 6; j++)
		{
			roomArray[i].connectionNames[j] = malloc(sizeof(char)*nameLength);
		}
	}
}

// finds and assigns most recent room directory to newestDirName
// *borrowed from example in block 2*
void findDir(char* newestDirName, int stringSize)
{
	// initialize to -1 to ensure no issues in comparison
	int newestDirTime = -1;

	// the target prefix for the directory we are seeking
	char targetDirPrefix[32] = "dheinc.rooms.";

	// used to check and analyze each potential directory
	DIR* dirToCheck;
	struct dirent *fileInDir;
	struct stat dirAttributes;

	// start with checking / opening root directory
	dirToCheck = opendir(".");

	// confirm root was opened
	if (dirToCheck > 0)
	{
		// while there is no error and end of directory is not reached
		while ((fileInDir = readdir(dirToCheck)) != NULL)
		{
			// if file/dir's name contains the target Prefix
			if (strstr(fileInDir->d_name, targetDirPrefix) != NULL)
			{
				// pull attributes into dirAttributes
				stat(fileInDir->d_name, &dirAttributes);

				// check if last modification time is more recent that the prior newest time
				if ((int)dirAttributes.st_mtime > newestDirTime)
				{
					// assign this dir's time to newest time
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(256));
					strcpy(newestDirName, fileInDir->d_name);
				}
			}
		}
	}

	// free the directory pointer
	free(dirToCheck);
}

// loads correct file for a room into roomArray
// accepts directory name and number of room/room file
void loadRoom(char* dirName, int roomNum)
{
	// buffer for full file name
	char buf[256];

	// combines dirName and roomNum into full string and 
	// stuffs this into buf
	snprintf(buf, sizeof(buf), "%s/room%d", dirName, roomNum);

	// prefixes for each line of room files
	// (used to check where to store that lines data)
	char prefixName[16] = "ROOM NAME: ";
	char prefixConn[16] = "CONNECTION ";
	char prefixType[16] = "ROOM TYPE: ";

	// pointer used to open file
	FILE *fp = fopen(buf,"r");

	// if open was successful
   	if (fp != NULL)
	{
		// holds full line of input from file
		char line[60];

		// holds data we are pulling from file (room name, type, connection name, etc)
		char temp[32];
		
		// clears out temp
		memset(temp,'\0', sizeof(temp));


		// iterator to track which character we are pulling from line
		int	fromIter;	

		// iterator to track where we are placing the character into temp
		int toIter;

		// while there are still lines to read
		while(fgets(line,60,fp) != NULL)
		{
			// If line contains prefix for room name
			// Load in room name
			if(strstr(line,prefixName) != NULL)
			{
				// 11 is location the actual room name starts
				fromIter = 11;	
				// start placing into temp at 0
				toIter = 0;

				// clears out temp
				memset(temp,'\0', sizeof(temp));


				// loops through and copies name from line into temp
				while(line[fromIter] != '\n')
				{
					temp[toIter] = line[fromIter];
					toIter++;
					fromIter++;
				}
				// copies temp into proper room's name
				strcpy(roomArray[roomNum].name, temp);

			}

			// If line contains prefix for connection
			// Load in connections
			if(strstr(line,prefixConn) != NULL)
			{
				// connection name starts at 14
				fromIter = 14;	
				toIter = 0;

				// clears out temp
				memset(temp,'\0', sizeof(temp));


				// loop and copy into temp
				while(line[fromIter] != '\n')
				{
					temp[toIter] = line[fromIter];
					toIter++;
					fromIter++;
				}

				// copy connection name into proper room and connectionName index
				strcpy(roomArray[roomNum].connectionNames[roomArray[roomNum].nextConnIndex],temp); 

				// increment connection count and index
				roomArray[roomNum].nextConnIndex++;
				roomArray[roomNum].connectionCount++;
			}

			// If line contains prefix for room type
			// Load in type
			if(strstr(line,prefixType) != NULL)
			{
				// type name starts at 11
				fromIter = 11;	
				toIter = 0;

				// clears out temp
				memset(temp,'\0', sizeof(temp));


				// copy type into temp
				while(line[fromIter] != '\n')
				{
					temp[toIter] = line[fromIter];
					toIter++;
					fromIter++;
				}

				// copy temp into room's type attribute
				strcpy(roomArray[roomNum].type,temp); 
			}

		}
	}

	// close file pointer
	fclose(fp);

}


// Prints player's current location and possible connections
void printLocation(Room* currLoc)
{
	// used to build string for each output line
	char buffer[512];

	// clear out to remove potential garbage characters
	memset(buffer, '\0', sizeof(buffer));

	// stuff string and location name into the buffer
	snprintf(buffer, sizeof(buffer), "\nCURRENT LOCATION: %s\nPOSSIBLE CONNECTIONS: ",currLoc->name);

	// loop to go through current location's connections and stuff
	// their names and proper delimiting character into bufer
	int i;
	for(i = 0; i < currLoc->connectionCount; i++)
	{
		// stuff name
		strcat(buffer, currLoc->connectionNames[i]);

		// if this is the last connection, stuff a period and newline
		if (i == currLoc->connectionCount - 1)
		{
			strcat(buffer, ".\n");
		}
		// otherwise stuff a comma and space
		else
		{
			strcat(buffer, ", ");
		}
	}

	// print fully generated output string
	printf("%s",buffer);
}

// trim the newline character from the end of a line
void trimNewline(char* line)
{
	// hold temporary line
	char tmpLine[60];
	// clear to ensure no garbage
	memset(tmpLine, '\0', sizeof(tmpLine));

	// tracks which character we are copying
	int index = 0;

	// continue copying characters until the newline is hit
	while(line[index] != '\n')
	{
		tmpLine[index] = line[index];
		index++;
	}

	// copy the string over to the original (now minus newline)
	strcpy(line, tmpLine);
}
	
// opens new thread and writes time into currentTime.txt file
void writeTime()
{
	// lock thread
	pthread_mutex_lock(&mtx1);

	// string for time
	char timeString[128];
	memset(timeString,'\0',sizeof(timeString));

	// create / open currentTime.txt for writing
	FILE* fp;
	fp = fopen("currentTime.txt","w+");

	// load time into time_t tme
	time_t tme = time(NULL);
	
	// tm struct to facilitate easier access / formatting of time
	struct tm *tminfo;
	// copy tme into tminfo as localtime
	tminfo = localtime(&tme);

	// copies the information from tminfo into timeString based on the provided format
	strftime(timeString,sizeof(timeString),"%I:%M%P, %A, %B %e, %Y",tminfo);

	// if first character is a '0' change to a space
	// ensures 08:00am will show as 8:00am
	if(timeString[0] == '0')
	{
		timeString[0] = ' ';
	}

	// write timeString to file
	fprintf(fp,"%s\n",timeString);

	// close file
	fclose(fp);

	// unlock thread
	pthread_mutex_unlock(&mtx1);

}	

// read time from currentTime.txt and write to console
void readTime()
{
	// lock thread
	pthread_mutex_lock(&mtx1);

	// hold line being read
	char line[64];

	// opens the file
	FILE* fp;
	fp = fopen("currentTime.txt","r");

	// retrieve line into line variable
	fgets(line,64,fp);

	// print the retrieved line
	printf("\n%s\n",line);

	// close the file
	fclose(fp);

	//unlock the thread
	pthread_mutex_unlock(&mtx1);
}

// main loop function to run the game
// accepts pointer to the starting room
void runGame(Room* startRoom)
{
	// set current room to startRoom
	Room* currRoom = startRoom;

	// track the number of move / visits
	int visitCount = 0;

	// holds the index of each room visited in order of their being visited
	// used to print out the path at the end
	int visitedRooms[128]; 

	// used to obtain input from user
	size_t inputSize = (sizeof(char)* 32);
	char *inputBuffer = malloc(inputSize);

	// lock thread
	pthread_mutex_lock(&mtx1);

	// loop while the player has not reached the END_ROOM
	while((strcmp(currRoom->type,"END_ROOM") != 0))
	{
		// used to confirm if user has entered a valid command
		bool badInput = true;

		// print current location name and connections
		printLocation(currRoom);

		// prompts user for command and loops as long as a bad command is given
		while(badInput)
		{
			printf("WHERE TO? >");

			// clear input buffer to ensure command is read correctly
			memset(inputBuffer,'\0', inputSize);

			// retrieves command from stdin
			getline(&inputBuffer,&inputSize,stdin);

			// trim the trailing newline from the command
			trimNewline(inputBuffer);

			// if command was 'time' call the write and read time functions
			if (strcmp(inputBuffer,"time") == 0)
			{
				// unlock thread to allow write and read to use it
				pthread_mutex_unlock(&mtx1);
				writeTime();
				readTime();
				// relock thread to continue
				pthread_mutex_lock(&mtx1);
			
			}

			// check if command matches potential connection name
			else
			{
				// loop through the current room's connections
				int i;
				for(i = 0; i < currRoom->connectionCount; i++)
				{
					// if entered command matches a connection name
					if(strcmp(inputBuffer,currRoom->connectionNames[i]) == 0)
					{
						// mark input as valid
						badInput = false;
					}
				}

				// if input is invalid print error string and location before
				// returning to prompt
				if(badInput)
				{
					printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
					printLocation(currRoom);		
				}
			}
		}

		// if input was a valid room, loop through the roomArray to locate
		// it and assign it as a the current room
		int i;
		for(i = 0; i < 7; i++)
		{
			// if input matches room's name
			if(strcmp(inputBuffer, roomArray[i].name) == 0)
			{
				currRoom = &roomArray[i];
				// add room's index to the visitedRooms array
				visitedRooms[visitCount] = i;
			}
		}

		// increment count of moves / visits
		visitCount++;
	}

	// CAN ONLY ARRIVE HERE IF CURRENT ROOM'S TYPE IS END_ROOM
	//
	// prints victory string along with the number of steps
	printf("\n\nYOU HAVE FOUND THE END ROOM. CONGRATS!\nYOU TOOK %d STEPS. YOUR PATH WAS:\n", visitCount);

	// loop through visitedRooms and print out the name of each room visited in the order it was visited
	int i;
	for(i = 0; i < visitCount; i++)
	{
		printf("%s\n",roomArray[visitedRooms[i]].name);
	}

	// free up the input buffer
	free(inputBuffer);
}


int main()
{

	char newestDirName[256];
	memset(newestDirName, '\0', sizeof(newestDirName));
	findDir(newestDirName, 256);

	Room* startRoom;

	initRoomArray();

	//loop through each index of roomArray and
	//load the room
	int i;
	for(i = 0; i < 7; i++)
	{
		loadRoom(newestDirName, i);
		// if the loaded room is the start room
		// make startRoom point to that room
		if(strcmp(roomArray[i].type, "START_ROOM") == 0)
			startRoom = &roomArray[i];
	}
	
	//start the game
	runGame(startRoom);

//	loop through roomArray and free
//	all allocations
	for(i = 0; i < 7; i++)
	{
		free(roomArray[i].name);
		free(roomArray[i].type);

		int j;
		for(j = 0; j < 6; j++)
		{
			free(roomArray[i].connectionNames[j]);
		}

	}

	return 0;
}

