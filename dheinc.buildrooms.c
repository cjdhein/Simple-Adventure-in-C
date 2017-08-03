#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>

// Room struct
typedef struct temp{
	char* name; // holds name of room
	char* type; // holds type of room
	int id;	// unique id for room
	char *connectionNames[6]; // array of name's of rooms this room is connected to 
	int nextConnIndex; // index of next empty connection
	int connectionCount; // the number of active connections
} Room;



// Holds the 7 rooms representing the map
static Room roomArray[7];

// constans for the number of rooms and the length of their names
static const int numOfRooms = 10;
static const int nameLength = 20;

// used to assign room id
static int roomCount = 0;

// creates the uniquely named directory
char* createDir();

// loops through roomArray and saves a file for each room
void saveRoom(char* dir, Room rm);

// assigns attributes for each room (name, type, id)
void setRoomAttribs(Room* newRoom, char names[numOfRooms][nameLength], int usedNames[10], int usedRoomTypes[3]);

// counts the number of active connections for a room
int countConnections(Room temp);

// grabs and adds a random connection to a room (performs validation)
void addRandomConnection();

// returns a randomly selected room from roomArray
Room* getRandomRoom();

// checks if all rooms have 3-6 connections
bool isGraphFull();

// connects rooms by adding each other's names to their connectedNames array
void connectRoom(Room* x, Room* y);

// checks if rooms are connectable (not already connected, not the same room, not already at max (6) connections
bool areConnectable(Room* x, Room* y);



int main()
{

	//test array of room names
	char roomNames[numOfRooms][nameLength];

	//copy names into array
	strcpy(roomNames[0], "Conservatory"); 
	strcpy(roomNames[1], "Library"); 
	strcpy(roomNames[2], "Tea Room"); 
	strcpy(roomNames[3], "Study"); 
	strcpy(roomNames[4], "Lavatory"); 
	strcpy(roomNames[5], "Parlor"); 
	strcpy(roomNames[6], "Cellar"); 
	strcpy(roomNames[7], "Foyer"); 
	strcpy(roomNames[8], "Ballroom"); 
	strcpy(roomNames[9], "Billiard Room"); 

	// hold the name of the directory we are going to save rooms into
	char* roomDir = createDir();


	// holds the index of the selected room
	int selection;

	// seed random with process id
	srand(getpid());

	// used to track the used names and types during generation
	int usedNames[10] = {0};
	int usedTypes[3] = {0};


	// loops through array to initialize the rooms and save to file
	int i;
	for(i = 0; i < 7; i++)
	{
		roomArray[i].name = malloc(sizeof(char)*20);
		roomArray[i].type = malloc(sizeof(char)*10);
		setRoomAttribs(&roomArray[i], roomNames, usedNames, usedTypes);
	}

	// while the graph (map) is not full / finished continue adding connections
	while(isGraphFull() == false)
	{
		addRandomConnection();
	}
	
	// once full, save all rooms as files into roomDir
	for(i = 0; i < 7; i++)
	{
		saveRoom(roomDir, roomArray[i]);
	}

	// free the rooms
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
		free(roomDir);
	return 0;
}



// returns the number of active connections a room has
// Accepts:
//	Room temp: room that is having its connections counted
// Returns:
//	count of connections for this room
int countConnections(Room temp)
{
	int count = 0;
	int i;
	for(i = 0; i < 6; i++)
	{
		// if a connection name exists, increment the count
		if(temp.connectionNames[i] != NULL)
			count++;
	}
	return count;
}

// Selects two rooms at random and connects them if they can make a 
// valid connection
void addRandomConnection()
{
	Room* A;
	Room* B;

	// call function to get random rooms
	do{
		A = getRandomRoom();
		B = getRandomRoom();
	} while(areConnectable(A,B) == false);	// as long as they do not make a valid connection, continue selecting at random

	// if they are connectable, call function to connect A & B
	connectRoom(A,B);
}

// Return a randomly selected room from roomArray
Room* getRandomRoom()
{
	// generate a random in form 0-6
	int r = rand() % 7;

	// use that int as the index for a room and return
	// a pointer to that room
	Room* selected = &roomArray[r];
	return selected;
} 
// returns true if graph is full 
bool isGraphFull() 
{ 
	// loop over each room in roomArray and check if 
	// each room as at least 3 connections 
	int i; 
	for(i = 0; i < 7; i++) 
	{ // less than 3 connections, return false 
		if(roomArray[i].connectionCount < 3) 
			return false; 
	} 

	return true; 
} 

// connect rooms x & y 
// Accepts:
// 	Room* x, Room* y: pointers to the two rooms to be checked 
void connectRoom(Room* x, Room* y) 
{ 
	// allocate size for name string in each room's connectionNames array 
	x->connectionNames[x->nextConnIndex] = malloc(sizeof(y->name)); 
	y->connectionNames[y->nextConnIndex] = malloc(sizeof(x->name)); 
	
	// copy each room name into the other rooms connectionName array 
	strcpy(x->connectionNames[x->nextConnIndex],y->name); 
	strcpy(y->connectionNames[y->nextConnIndex],x->name);
   
	// increment nextConnIndex to ensure all connections are tracked 
	x->nextConnIndex++; 
	y->nextConnIndex++; 
	
	// update each rooms connection count 
	x->connectionCount = countConnections(*x); 
	y->connectionCount = countConnections(*y); 
} 

// checks if rooms x & y are connectable 
// Accepts:
//  Room* x, Room* y: pointers to the two rooms to be checked 
//  Returns: Boolean status of whether rooms are connectable 
bool areConnectable(Room* x, Room* y) 
{
  // if either room has already reached the max connection 
  // amount (6) return false so new room can be grabbed 
  if (x->connectionCount >=6 || y->connectionCount >= 6) 
	  return false; 
  
  // if ids are equal the same room was grabbed and this is an invalid connection 
  if(x->id == y->id) 
	  return false; 
  
  // loop through each room's connectionNames array 
  int i; 
  for(i = 0; i < 6; i++) 
  { 
	  // if the index is not NULL, a connection exists here so we check it 
	  if(x->connectionNames[i] != NULL) 
	  { 
		  // if y's name exists at x's connectionName index, this connection already exists 
		  if(strcmp(y->name, x->connectionNames[i]) == 0) 
			  return false; 
	  } 
	  
	  if(y->connectionNames[i] != NULL) 
	  { 
		  if(strcmp(x->name, y->connectionNames[i]) == 0) 
			  return false;
	  }
  }

	// if above requirements are met, the rooms are connectable
	return true;
}



// Accepts:
//	Room* newRoom:	pointer to the room being established
//	char names:	array of names for the rooms	
//	int usedNames:	array to track which indicies of 'names' have already been used (index in this array
//					is flipped to 1 once that index in 'names' has been used)
//	int usedRoomTypes:	tracks which roomTypes have been used (1 is START_ROOM, 3 is END_ROOM)	
void setRoomAttribs(Room* newRoom, char names[numOfRooms][nameLength], int usedNames[10], int usedRoomTypes[3])
{
	// holds index of random generated room name
	int tmp;
	
	// continue grabbing random names until one is selected that 
	// has not been used
	do{
		tmp	= rand() % 10;
	}while (usedNames[tmp] != 0);

	// set selected name's index in usedNames to 1, so this room name cannot be selected again
	usedNames[tmp] = 1;
	// copy name from names to the actual room's name
	strcpy(newRoom->name, names[tmp]);

	// if all three room types already exist, assign room as a MID_ROOM
	// (ensures that only 1 START_ROOM and END_ROOM can exist)
	if (usedRoomTypes[0] == 1 && usedRoomTypes[1] == 1 && usedRoomTypes[2] == 1)
	{
		strcpy(newRoom->type, "MID_ROOM");
	}
	// otherwise, generate a random int from 0-2 and use that room type as long as it is unused
	else
	{
		do{
			tmp = rand() % 3;
		} while (usedRoomTypes[tmp] == 1);
		if(tmp == 0)
		{
			// copy room type over and set room type to 1 for used
			strcpy(newRoom->type, "START_ROOM");
			usedRoomTypes[0] = 1;
		}
		else if(tmp ==2)
		{
			// copy room type over and set room type to 1 for used
			strcpy(newRoom->type, "END_ROOM");
			usedRoomTypes[2] = 1;
		}
		else
		{
			// copy room type over and set room type to 1 for used
			strcpy(newRoom->type, "MID_ROOM");
			usedRoomTypes[1] = 1;
		}
	}

	// assign id and increment roomCount
	newRoom->id = roomCount;
	roomCount++;

	// initialize total connections and nextConnIndex to zero
	// (ensures that connections are added at start of array)
	newRoom->connectionCount = 0;
	newRoom->nextConnIndex = 0;	   
}




// creates and saves a file for the specified room in the specified directory
// Accepts:
//	char* dir:	pointer to char array holding name of the directory
//	Room rm:	the room struct holding the room's information to be saved
void saveRoom(char* dir, Room rm)
{
	char temp[128];
	char buf[256];

	// build 'full' room name including directory, and room + id number
	// ie: /dheinc.rooms.[PID]/room1
	snprintf(buf, sizeof(buf), "%sroom%d", dir, rm.id);

	// create file pointer, open file in write moed and print name into file
	FILE *fp;
	fp = fopen(buf,"w+");
	fprintf(fp,"ROOM NAME: %s\n",rm.name);

	// loop through the room's connections
	int i;
	for(i = 0; i < rm.connectionCount; i++)
	{
		// print a line for each connection
		fprintf(fp,"CONNECTION %d: %s\n",i+1, rm.connectionNames[i]);
	}

	// lastly print the room's type and close the file pointer
	fprintf(fp,"ROOM TYPE: %s\n",rm.type);
	fclose(fp);


}

// Create the directory to be used for this program instance
// Returns: String with full directory name being used to store rooms
char* createDir()
{	
	// hold the PID 
	char pidString[20];
	// hold the directory name
	char dname[40];
   
	// set both to empty to ensure accuracy 
	memset(pidString,'\0',sizeof(pidString)); 
	memset(dname,'\0',sizeof(dname)); 
	
	// obtain PID 
	int pid = getpid(); 
	
	// write pid as string to pidString
	sprintf(pidString,"%d",pid); 
	
	// copy prefix into dname 
	strcpy(dname, "dheinc.rooms."); 
	
	// concatenate pid onto dname 
	strcat(dname,pidString); 
	
	// make directory with proper privileges
	int result = mkdir(dname, 0755); 

	// if result is not 0, an error occured
	if(result != 0)
	{
		fprintf(stderr,"%s%s%s","Error creating directory '",dname, "'  Program terminated.\n");
		exit(1);
	}
	
	// allocate memory for string containing full directory name
	// and set it to empty for accuracy
	char* fullDirString = malloc(128 * sizeof(char));
	memset(fullDirString, '\0', sizeof(fullDirString) / sizeof(char));

	// copy dname to full string and add the ending '/'
	strcat(fullDirString,dname);
	strcat(fullDirString,"/");

	// return the full string
	return fullDirString;
}


