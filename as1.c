#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define JSUP 26
#define JSRT 47
#define JSDN 46
#define JSLFT 65
#define JSUP_VALUE "/sys/class/gpio/gpio26/value"
#define JSRT_VALUE "/sys/class/gpio/gpio47/value"
#define JSDN_VALUE "/sys/class/gpio/gpio46/value"
#define JSLFT_VALUE "/sys/class/gpio/gpio65/value"
#define LED0_BRIGHT "/sys/class/leds/beaglebone:green:usr0/brightness"
#define LED1_BRIGHT "/sys/class/leds/beaglebone:green:usr1/brightness"
#define LED2_BRIGHT "/sys/class/leds/beaglebone:green:usr2/brightness"
#define LED3_BRIGHT "/sys/class/leds/beaglebone:green:usr3/brightness"
#define MAX_LENGTH 1024

bool getRandBool(void);
void exportPin(int pinNum);
void delayNano(long nanoseconds);

void setBrightOn(char* brightPath);
void setBrightOff(char* brightPath);
void turnOnAllLED(void);
void turnOffAllLED(void);
void successFlash(void);
void failureFlash(void);

int readJoystickValue(char *fileName);
int checkJoystick(void);

int main(int argc, char* argv[])
{
	printf("Hello embedded world, from Calvin Truong!\n\n");
	printf("Press the Zen cape's Joystick in the direction of the LED.\n");
	printf("	UP for LED 0 (top)\n");
	printf("	DOWN for LED 3 (bottom)\n");
	printf("	LEFT/RIGHT for exit app.\n");

	srand(time(0));	

	exportPin(JSUP);
	exportPin(JSRT);
	exportPin(JSDN);
	exportPin(JSLFT);

	turnOffAllLED();

	bool upGame = false;
	bool upUser = false;
	int userScore = 0;
	int round = 0;
	int joystickFlag = 0;
	int gameOverFlag = 0;
	int direction = 0;

	while(1){
		printf("Press joystick; current user score (%d / %d)\n", userScore, round);

		// Select a random boolean to initiate the game with a direction
		upGame = getRandBool();
		if(upGame){
			setBrightOn(LED0_BRIGHT);
		}else{
			setBrightOn(LED3_BRIGHT);
		}

		// Check that joystick input exists to move on to assessment
		while(joystickFlag == 0){
			direction = checkJoystick(); // 0: none, 1: up, 2: right, 3: down, 4:left
			if (direction != 0){
				joystickFlag = 1;
				if (direction == 1){
					upUser = true;
				}else if ((direction == 2) || (direction == 4)){
					gameOverFlag = true;
				}else{
					upUser = false;
				}
			}
		}

		// Assess joystick direction to determine score
		if ((direction == 1) || (direction == 3)){
			if(upUser == upGame){
				printf("Correct!\n");
				userScore = userScore +1;
				successFlash();
			}else{
				printf("Incorrect! :(\n");
				failureFlash();
			}
		}

		// Wait until joy stick is neutral to avoid touble triggers
		while(joystickFlag == 1){
			direction = checkJoystick();
			if (direction == 0){
				joystickFlag = 0;
			}
		}

		// End the game to match user's game exit request
		if (gameOverFlag == 1){
			printf("Your final score was (%d / %d)\n", userScore, round);
			printf("Thank you for playing, goodbye!\n");
			turnOffAllLED();
			break;
		}
		
		round = round + 1;
	}
	return 0;
}

bool getRandBool(void)
{
	bool randBool = rand() % 2;
	return randBool;
}

void exportPin(int pinNum)
{
	FILE *pFile = fopen("/sys/class/gpio/export", "w");
	if (pFile == NULL) {printf("ERROR: Unable to open export file.\n");
		exit(1);
	}
	fprintf(pFile, "%d", pinNum);
	fclose(pFile);
	delayNano(300000000);
}

void delayNano(long nanoseconds)
{
	long seconds = 0;
	struct timespec reqDelay = {seconds, nanoseconds};
	nanosleep(&reqDelay, (struct timespec *) NULL);
}

void setBrightOn(char* brightPath)
{
	FILE *pLedBrightnessFile = fopen(brightPath, "w");
	if (pLedBrightnessFile == NULL) {
		printf("ERROR OPENING %s.\n", brightPath);
		exit(1);
	}
	int charWritten = fprintf(pLedBrightnessFile, "1");
	if (charWritten <= 0) {
		printf("ERROR WRITING DATA");
		exit(1);
	}
	fclose(pLedBrightnessFile);
}

void setBrightOff(char* brightPath)
{
	FILE *pLedBrightnessFile = fopen(brightPath, "w");
	if (pLedBrightnessFile == NULL) {
		printf("ERROR OPENING %s.\n", brightPath);
		exit(1);
	}
	int charWritten = fprintf(pLedBrightnessFile, "0");
	if (charWritten <= 0) {
		printf("ERROR WRITING DATA");
		exit(1);
	}
	fclose(pLedBrightnessFile);
}

void turnOnAllLED(void)
{
	setBrightOn(LED0_BRIGHT);
	setBrightOn(LED1_BRIGHT);
	setBrightOn(LED2_BRIGHT);
	setBrightOn(LED3_BRIGHT);
}

void turnOffAllLED(void)
{
	setBrightOff(LED0_BRIGHT);
	setBrightOff(LED1_BRIGHT);
	setBrightOff(LED2_BRIGHT);
	setBrightOff(LED3_BRIGHT);
}

void successFlash(void)
{
	turnOnAllLED();
	delayNano(100000000);
	turnOffAllLED();
}

void failureFlash(void)
{
	for(int i = 0; i < 5; i++){
		turnOnAllLED();
		delayNano(100000000);
		turnOffAllLED();
		delayNano(100000000);
	}
}

int readJoystickValue(char *fileName)
{
	FILE *pFile = fopen(fileName, "r");
	if (pFile == NULL) {
		printf("ERROR: Unable to open file (%s) for read\n", fileName);
		exit(-1);
	}
	char buff[MAX_LENGTH];
	fgets(buff, MAX_LENGTH, pFile);
	fclose(pFile);
	if (strcmp(buff,"1\n") == 0){
		return 1;
	}else{
		return 0;
	}
}

int checkJoystick(void)
{
	delayNano(5000000);

	int upRead = readJoystickValue(JSUP_VALUE);
	int downRead = readJoystickValue(JSDN_VALUE);
	int leftRead = readJoystickValue(JSLFT_VALUE);
	int rightRead = readJoystickValue(JSRT_VALUE);
	
	/* 
		Return -- 0: none, 1: up, 2: right, 3: down, 4:left;
		Return if the reading is 0 because the joystick input is inverted.
		I.e., 0 means joystick is pressed, 1 means it is not pressed
	*/
	if(upRead == 0){
		return 1;
	}else if (rightRead == 0){
		return 2;
	}else if (downRead == 0){
		return 3;
	}else if (leftRead == 0){
		return 4;
	}else{
		return 0;
	}
}