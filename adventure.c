/* Name: Grace Thompson
 * 
 * Program Name: Adventure Game 
 * Created: 5/2/16
 *
 * This program generates 7 room files in a directory labeled 
 * thompsgr.rooms.<process id>, each with a room name, type,
 * and between 3 and 6 outgoing connections.
 *
 * Once the program files have been created, the program reads the data
 * back into Room structs for storage to play the game.
 * The user starts out in the starting room, and is given a list
 * of the possible connections to choose from.
 * The user chooses the next room to go in, until the user
 * finds the end room. The program displays a congratulatory message
 * and displays the rooms that the user went through to get
 * to the end room, as well as the count of steps. 
 *
 **/

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <dirent.h>

//struct that holds the data for each individual room file
typedef struct Room {
	int roomNumber; //int identifier for room, corresponds to index in array
	char roomName[20]; //name of room
	int roomType; //start, end, or mid (0, -1, 1)
	int roomConnections[6]; //array to hold the roomNumber identifiers for the connecting rooms
	int connectionNumber; //number of connections for the room
	int desiredNumConnect; //random number of connections
	char pathname[100]; //holds the pathname to access the corresponding room file
} Room;

//array that holds seven Room structs for when generating the room files
struct Room *rooms[7];

//function to create Room struct and fill it with the passed data
void createRoom(struct Room *r, int i, char *roomName, char *pathname) {
	r->roomNumber = i; //maintains the index for the room, according to the position in the array of Rooms
	snprintf(r->roomName, 20, roomName);
	r->connectionNumber = 0; //initialize number of connections to 0
	r->desiredNumConnect = 0; //initialize to zero, will randomly generate this number between 3 and 6	
	r->roomType = -1; //initialize to an mid room type
	//initialize connection array to -1 for all indices
	int index;
	for(index = 0; index < 6; index++) {
		r->roomConnections[index] = -1;
	}
	//stores the pathname to open the associated room file
	snprintf(r->pathname, 100, pathname);
}


//function to find a room not yet connected to the passed room, and that is not the room itself
//returns the index of a valid room connection
int findConnection(struct Room *r) {
	int found = 0;
	int connectingIndex;
	while(found == 0) {
		connectingIndex = rand() % 7;
		//check if connectingIndex can receive another connecting room, check if index is same as room's index
		if (rooms[connectingIndex]->connectionNumber < 6 && connectingIndex != r->roomNumber) {
			//check if index is already in roomConnections array
			int x;
			int match = 0;
			for(x = 0; x < 6; x++) {

				if (connectingIndex == r->roomConnections[x]) {
					match = 1; //room already in array, need to find a new room to add
					break;
				}
			}
			
			if (match == 0) { //if no match was found, the room is a valid connection
				found = 1; //found a room to connect, exit out of while loop
			}
		}	
	}
	return connectingIndex;	
}

//function to add the connections between two rooms
//writes the connection information to the corresponding room files
void addConnection(Room *r, Room *c) {
	//add room id to each other's roomConnections array
	int rNumId = r->roomNumber;
	int cNumId = c->roomNumber;
//	printf("r: %s, c: %s \n", r->roomName, c->roomName);
	r->roomConnections[r->connectionNumber] = cNumId;
	r->connectionNumber++; //update the number of connections for the room

	//open room file and add connecting room 
	char buffer [1024]; //to hold string data to write to file
	int fd;
	fd = open(r->pathname, O_WRONLY | O_APPEND);
	//check for error opening
	if (fd == -1) {
		printf("error opening file to write room connection \n");
		
	} else {
		//write info to file
		snprintf(buffer, 1024, "CONNECTION %i: %s \n", r->connectionNumber, c->roomName);
		write(fd, buffer, strlen(buffer));
	}	
	//close the file
	close(fd);
	//clear buffer
	strcpy(buffer, "");

	//add connection info for the connecting room
	c->roomConnections[c->connectionNumber] = rNumId;
	c->connectionNumber++; //update the number of connections for the room
//	printf("path: %s \n", c->pathname);
	//open connecting room file and add connection to current room
	fd = open(c->pathname, O_WRONLY | O_APPEND);
	//check for error opening
	if (fd == -1) {
		printf("error opening file to write room connection \n");
	} else {
		//write info to file
		snprintf(buffer, 1024, "CONNECTION %i: %s \n", c->connectionNumber, r->roomName);
		write(fd, buffer, strlen(buffer));
	}
	//close the file
	close(fd);
	//clear the buffer
	strcpy(buffer, "");
}

//function that sets the room at index 0 to the start room,
//the room at the last index to the end room,
//and all rooms in between to mid rooms.
void setRoomTypes() {
	rooms[0]->roomType = 0;
	rooms[6]->roomType = 1;
	//all other rooms can maintain their original int -1, meaning they are mid rooms.
}

//function that generates 7 room files, using a list of 10 possible room names
void generateRooms(char *r) {
	//array of struct Rooms
//	struct Room *rooms[6];

	//array that holds the 10 possible room names
	char *roomNames[10];
		roomNames[0] = "foyer";
		roomNames[1] = "kitchen";
		roomNames[2] = "bedroom";
		roomNames[3] = "library";
		roomNames[4] = "bathroom";
		roomNames[5] = "study";
		roomNames[6] = "attic";
		roomNames[7] = "playroom";
		roomNames[8] = "greenhouse";
		roomNames[9] = "cellar";
	
	int i;
	int roomIndex;
	int fd;
	char buffer [1024];
	char roomPathname [1024]; //string holds pathname for a given file
	int roomIsFree = 1; //init value to false, for checking if a room is available
	int numOfConnections; //stores randomly generated number of connections for each room
	

	//seed time for randomization
	srand(time(NULL));
	//generate 7 room files
	for(i = 0; i < 7; i++) {
		while(roomIsFree == 1) {
			//generate random number between 0 - 9 for room name
			roomIndex = rand() % 10;
			//check if room name is already in use
			snprintf(roomPathname, 1024, "%s%s", r, roomNames[roomIndex]);
			//try and open a file, create only if doesn't already exist
			fd = open(roomPathname, O_CREAT | O_RDWR | O_EXCL | O_APPEND, 0777);
			if (fd >= 0) {
				//file was created successfully, store name of room
				snprintf(buffer, 1024, "ROOM NAME: %s \n", roomNames[roomIndex]);
				write(fd, buffer, strlen(buffer));

				//create struct for Room and fill
				rooms[i] = (struct Room*)malloc(sizeof(struct Room));
				createRoom(rooms[i], i, roomNames[roomIndex], roomPathname); 
									
				//set number of connections, randomly generated between 3 and 6
				numOfConnections = (rand() % 4) + 3;
				rooms[i]->desiredNumConnect = numOfConnections;	
	
				roomIsFree = 0; //exit out of while loop, file successfully created
			}
			
			//close file opened
			close(fd);

			//clear buffer and pathname for next room file
			strcpy(roomPathname, "");
			strcpy(buffer, "");
		}	
		//return roomIsFree to false for next file generation
		roomIsFree = 1;
	}
	//add connections to rooms
	for(i = 0; i < 7; i++) {

		int connection;
		//keep adding connections until the room has the desired number of connections
		while (rooms[i]->connectionNumber < rooms[i]->desiredNumConnect) {
			//find a room to connect to
			connection = findConnection(rooms[i]);
//			printf("connection to: %i \n", connection);
			addConnection(rooms[i], rooms[connection]);
		}		
	}
	
	//make the first room the start, last the end, and all others, mid rooms. Since the rooms
	//are named randomly on generation, this will always be random automatically.
	setRoomTypes();
	
	char roomtype[100]; //string to hold string room type written to file
	//write types to files
	for(i = 0; i < 7; i++) {
		fd = open(rooms[i]->pathname, O_WRONLY | O_APPEND);
		if (fd == -1) {
			printf("error opening file to write room type \n");
		} else {
			//write to file according to type
			if (rooms[i]->roomType == 0) {
				//if start room
				snprintf(roomtype, 100, "ROOM TYPE: START_ROOM \n");
			} else if (rooms[i]->roomType == 1) {
				//if end room
				snprintf(roomtype, 100, "ROOM TYPE: END_ROOM \n");
			} else {
				//mid room
				snprintf(roomtype, 100, "ROOM TYPE: MID_ROOM \n");
			}
			//write to file
			write(fd, roomtype, strlen(roomtype));
			//close the file
			close(fd);
			//clear buffer roomtype
			strcpy(roomtype, "");
		}
	}

	//free the memory used to store the Room structs
	for(i = 0; i < 7; i++) {
		free(rooms[i]);
	}
}

//function to read the connections for a room file and adds the int identifier to the 
//passed room's array of connections
void readConnections(struct Room *r, struct Room **rooms) {
	//open file for the room by the pathname
	FILE *roomFile;
	roomFile = fopen(r->pathname, "r");
	if (roomFile == NULL) {
		printf("error opening file to read connections \n");
	} else {
		//read in file contents
		char buffer [1024];
		char description;
		char rn[20];
		while (!feof(roomFile)) {
			fgets(buffer, 1024, roomFile);
			description = buffer[0];
			//if description char is 'c', line contains a connection
			if(description == 'C') {
				//grab name of connection
				char *token;
				token = strtok(buffer, ":"); //move cursor to after the file line description
				token = strtok(NULL, " "); //token now contains the name of the connection
				snprintf(rn, 20, token);	
				//find the room with the matching name
				int n;
				int connectId; //stores the id of the connecting room
				for (n = 0; n < 7; n++) {
					if (strcmp(rn, rooms[n]->roomName) == 0) {
						//found room, get int id
						connectId = rooms[n]->roomNumber;
					}
				}
				//add id to room's array of connecting rooms
				int pos = r->connectionNumber; //next available index in array
				r->roomConnections[pos] = connectId;
				//update connectionNumber
				r->connectionNumber++;
			}
		//	printf("%s", buffer);
			strcpy(buffer, "");
			strcpy(rn, "");
		}
		fclose(roomFile);
	}
}

//function that displays the current room and possible connections, and gets a valid room from the user
//returns the int id for the room chosen by the user, loops until a valid room is entered
int getNextRoom(struct Room *r, struct Room **rooms) {
	int validRoom = -1;
	char buffer [1024];
	int connNum = -1;
	while (validRoom == -1) {
		snprintf(buffer, 1024, "CURRENT LOCATION: %s", r->roomName);
		printf("\n%s \n", buffer);
	
		//clear the buffer
		strcpy(buffer, "");
			
		snprintf(buffer, 1024, "POSSIBLE CONNECTIONS: "); //start string that tells user possible rooms
		//get the possible connection ids from Room r
		int ctrooms[6] = { -1 }; //array stores up to 6 connecting rooms by ids, init to invalid value
		char connectionName[20];
		int i;
		int connectId;
		for (i = 0; i < 6; i++) {
			//iterate through r's array of connecting rooms, grab all not -1
			if (r->roomConnections[i] != -1) {
				connectId = r->roomConnections[i];
				snprintf(connectionName, 20, "%s, ", rooms[connectId]->roomName);
				strcat(buffer, connectionName);
				//add connectId to array holding possible connection ids
				ctrooms[i] = connectId; //since all -1 are at the end of the array, will add in order
				
			}
		}
		//replace last character in string with .
		buffer[strlen(buffer) - 2] = '.';
		printf("%s \n", buffer);
		//clear the buffer and connectionName char array
		strcpy(buffer, "");
		strcpy(connectionName, "");

		//ask user where to go next
		snprintf(buffer, 1024, "WHERE TO? >");
		printf("%s", buffer);
	
		//clear buffer
		strcpy(buffer, "");

		//read input from user
		char userInput[100];
		scanf("%s", userInput);
		//compare userInput to possible connection names for math
		int index;
		for (index = 0; index < r->connectionNumber; index++) {
			if(strcmp(userInput, rooms[r->roomConnections[index]]->roomName) == 0) {
		//		printf("found match \n");
				connNum = r->roomConnections[index]; //grab int id
				validRoom = 0; //found valid room
			}
		}
		if (connNum == -1) {
			//display message that input was invalid
			printf("\n");
			snprintf(buffer, 1024, "HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.");
			printf( "%s \n", buffer);
			printf("\n");
			//clear buffer
			strcpy(buffer, "");
		}
		//clear userInput
		strcpy(userInput, "");	
	}
	return (connNum);
}

//function for the game play
//begins the game by grabbing the starting room, and presenting the possible
//connecting rooms to the user to choose from. This continues until
//the user chooses the end room, with a roomType 1
//Game ends by displaying a congratulatory message
//and displaying the number of steps, and the rooms passed through,
//to get to the end room.
void playAdventure(struct Room **rooms) {
	int foundEnd = 0; //signals the end of the game
	int stepsCount = 0; //counts the number of steps to find the end room
	char currentRoom[20]; //stores the string name of the current room
	struct Room *cur;
	char buffer[1024]; //stores message about game
	int pathToVictory[100]; //stores int ids of each room passed through
	int nextPos = 0; //stores the index of the next available position in pathToVictory

	int i;
	for (i = 0; i < 7; i++) {
		if (rooms[i]->roomType == 0) {
			//if start room, grab room struct
			cur = rooms[i];
			snprintf(currentRoom, 20, rooms[i]->roomName);
		}
	}
	int nextRoom; //stores int id of next room
	//while the end room is not selected
	while (foundEnd != 1) {
		//present user with current location and possible connections
		nextRoom = getNextRoom(cur, rooms);
		stepsCount++; //increment count for user's next room pick
		//store room id number in pathToVictory
		pathToVictory[nextPos] = nextRoom;
		nextPos++; //increment nextPos to point to next available position

		//check if roomtype is end room, value 1
		if (rooms[nextRoom]->roomType == 1) {
			snprintf(buffer, 1024, "YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!");
			printf("\n%s \n", buffer);
			//clear buffer
			strcpy(buffer, "");
			//display number of steps taken to victory
			snprintf(buffer, 1024, "YOU TOOK %i STEPS. YOU PATH TO VICTORY WAS: \n", stepsCount);
			printf(buffer);
			strcpy(buffer, "");		
			//display rooms entered in order 
			int x;
			for (x = 0; x < nextPos; x++) {
				snprintf(buffer, 1024, "%s", rooms[pathToVictory[x]]->roomName);
				printf("%s \n", buffer);
				strcpy(buffer, ""); //clear buffer for next name
			}
			foundEnd = 1;
		} else {
			//move to next room chosen by user
			cur = rooms[nextRoom];
		}
	}
}


int main(int argc, char *argv[]) {
	//create directory to hold room files
	//get process id for directory name
	int processID = getpid();
	
	//array that holds seven Room structs for game play
	struct Room *gameRooms[7];	

	//string to hold name of room directory
	char roomDirName [100];
	snprintf(roomDirName, 100, "thompsgr.rooms.%i/", processID);
	
	//create directory
	int roomDirectory = mkdir(roomDirName, 0777);

	//check to make sure directory was successfully created
	if(roomDirectory != 0) {
		printf("Error! Directory could not be made. \n");
		return 1;
	} else {
		generateRooms(roomDirName);
		//read data from room files to gameRoom structs
		DIR *dp; //to open the directory
		struct dirent *ep;
		FILE *file; //to read individual files
		char roomPathname[100]; //buffer to store pathname to file in directory
		dp = opendir (roomDirName);
		if (dp != NULL) {
			//store index of next available position in the gameRooms room array
			int index = 0; //starts at beginning of array
			//if directory opened successfully
			while (ep = readdir (dp)) {
				if (strcmp(ep->d_name, ".") != 0 && strcmp(ep->d_name, "..") != 0) {
					snprintf(roomPathname, 100, "%s%s", roomDirName, ep->d_name);
					//open file for reading
					file = fopen(roomPathname, "r");
					if (file == NULL) {
						printf("error opening file %s for reading \n", ep->d_name);
					} else {
						//allocate memory for Room struct for storing the file data
						gameRooms[index] = (struct Room *)malloc(sizeof(struct Room));
						//read in file one line at a time
						char buffer[1024];
						char lineDescription[1024];
						while (!feof(file)) {
							//read next line
							fgets(buffer, 1024, file);
							strncpy(lineDescription, buffer, 6);
							//move through tokens to grab the type of data on the current line
							char *token;
							token = strtok(buffer, ":");
							token = strtok(NULL, " ");
							//determine what type of data the line contains
							if (strcmp(lineDescription, "ROOM N") == 0) {
								//if the room name, create room struct and add name and path to file
								char roomName[100];
								snprintf(roomName, 100, token);
								createRoom(gameRooms[index], index, roomName, roomPathname);
							//	printf("room name: %s \n", gameRooms[index]->roomName);
							} else if (strcmp(lineDescription, "ROOM T") == 0) {
								//if start room, roomType = 0
								//if mid room, roomType = -1 (no need to do anything)
								//if end room, roomType = 1
								if(strcmp(token, "START_ROOM") == 0) {
									gameRooms[index]->roomType = 0;
							
								} else if (strcmp(token, "END_ROOM") == 0) {
									gameRooms[index]->roomType = 1;
								}
							}

							//clear buffer
							strcpy(buffer, "");
							strcpy(lineDescription, "");
						}
						index++; //move to next available position in gameRooms array

					}
					fclose(file);
					//clear buffer pathname string
					strcpy(roomPathname, "");
				}
			}
			//close directory
			(void) closedir (dp);

			//get connection information for rooms now that all structs have been initialized, each room now has an identifiying int
			int i;
			int j;
			for (i = 0; i < 7; i++) {
				readConnections(gameRooms[i], gameRooms);
			}

		} else { //send error that directory could not be opened
			perror("couldn't open the directory \n");
			exit (1);
		}
	}
	
	//start the game!
	playAdventure(gameRooms);
	int i;
	//free the memory allocated to the Room structs
	for (i = 0; i < 7; i++) {
		free(gameRooms[i]);
	}

	return 0;

}
