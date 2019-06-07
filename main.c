#include <stdlib.h>
#include <fcntl.h>
#include <sys/select.h>
#include <time.h>
#include <string.h>
#include <termios.h>
#include "A053BasicKit.h"

#define ALIVE 0
#define DIGITAL 1
#define ANALOG 2
#define PWM 3
#define SERVO_PIN 4
#define TONE 5
#define PULSEIN 6
#define ULTRASONIC 7
#define TIMER 8

#define GET 1
#define SET 2
#define RESET 3

union{
  uint8_t byteVal[4];
  float floatVal;
  long longVal;
}val;

union{
  uint8_t byteVal[2];
  short shortVal;
}valShort;

int servos[8];

struct timespec stime;

int trigPin = 13;
int echoPin = 12;

int analogs[6]={0,};
int digitals[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int servo_pins[8]={0,0,0,0,0,0,0,0};

float lastUltrasonic = 0;

char buffer[52];
unsigned char prevc=0;

int inDex = 0;
uint8_t dataLen;

double lastTime = 0.0;
double currentTime = 0.0;

uint8_t command_index = 0;

bool isStart = false;
bool isUltrasonic = false;



int pwm_pin[4] = {0,1,2,4};
int gpio_pin[9] = {51,51,46,51,47,51,51,48,50};
int fd_pwm[4];
static char buf[16] = "/dev/ttyS0";



void writeBuffer(int index,unsigned char c){
  buffer[index]=c;
}

void callOK(){
	  int idx_tx = 0;
	  char buf_tx[16] = {0,};
	  int fd;
	  fd = open(buf, O_RDWR | O_NOCTTY);
	  buf_tx[idx_tx] = 0xff;
	  idx_tx++;
	  buf_tx[idx_tx] = 0x55;
	  idx_tx++;
	  buf_tx[idx_tx] = 10;
	  idx_tx++;
	  int c;
	  for(c = 0 ; c < idx_tx ; c++){
		  write(fd, &buf_tx[c], 1);
	  }

	  close(fd);
}

unsigned char readBuffer(int index){
  return buffer[index];
}

short readShort(int idx){
  valShort.byteVal[0] = readBuffer(idx);
  valShort.byteVal[1] = readBuffer(idx+1);
  return valShort.shortVal;
}

void sendFloat(float value){
  writeSerial(2);
  val.floatVal = value;
  writeSerial(val.byteVal[0]);
  writeSerial(val.byteVal[1]);
  writeSerial(val.byteVal[2]);
  writeSerial(val.byteVal[3]);
}

void runModule(int device) {
  //0xff 0x55 0x6 0x0 0x1 0xa 0x9 0x0 0x0 0xa
  int port = readBuffer(6);
  int pin = port;

  switch(device){
    case DIGITAL:{
    if(digitals[pin] == 0){
    	digitals[pin] = 1;
    }
      int v = readBuffer(7);
      if(v == 255)
    	  v = 1;
      gpio_write(gpio_pin[pin],v);
    }
    break;
    case PWM:{
        if(digitals[pin] == 0){
        	digitals[pin] = 1;
        }
      int v = readBuffer(7);
      switch(pin){
      	  case 3:
      		  pwm_write(fd_pwm[0], 256, v);
      		  break;
      	  case 5:
      		  pwm_write(fd_pwm[1], 256, v);
      		  break;
      	  case 6:
      		  pwm_write(fd_pwm[2], 256, v);
      		  break;
      	  case 9:
      		  pwm_write(fd_pwm[3], 256, v);
      		  break;
      }
    }
    break;
    case TONE:{
        if(digitals[pin] == 0){
        	digitals[pin] = 1;
        }
      int hz = readShort(7);
      int period = 1000000/hz;
      int ms = readShort(9);
      if(ms>0) {
          switch(pin){
          case 3:
			  if(hz == 0)
				  pwm_write(fd_pwm[0], 256, 0);
			  else
				  pwm_write(fd_pwm[0], period, 100);
			  break;
		  case 5:
			  if(hz == 0)
				pwm_write(fd_pwm[1], 256, 0);
			else
			  pwm_write(fd_pwm[1], period, 100);
			  break;
		  case 6:
			  if(hz == 0)
			  pwm_write(fd_pwm[2], 256, 0);
			else
			  pwm_write(fd_pwm[2], period, 100);
			  break;
		  case 9:
			  if(hz == 0)
			  pwm_write(fd_pwm[3], 256, 0);
			else
			  pwm_write(fd_pwm[3], period, 100);
			  break;
          }
      } else {
          switch(pin){
          	  case 3:
          		  pwm_write(fd_pwm[0], 256, 0);
          		  break;
          	  case 5:
          		  pwm_write(fd_pwm[1], 256, 0);
          		  break;
          	  case 6:
          		  pwm_write(fd_pwm[2], 256, 0);
          		  break;
          	  case 9:
          		  pwm_write(fd_pwm[3], 256, 0);
          		  break;
          }
      }
    }
    break;
    case SERVO_PIN:{
        if(digitals[pin] == 0){
        	digitals[pin] = 1;
        }
      int v = readBuffer(7);
      if(v>=0&&v<=180){
        searchServoPin(pin);

      }
    }
    break;
    case TIMER:{
      clock_gettime( CLOCK_REALTIME, &stime);

		float mils = stime.tv_nsec/1000000000.0;
		float sec = (stime.tv_sec-1262304000);

      lastTime = sec + mils;
    }
    break;
  }
}

void parseData() {
  isStart = false;
  int idx = readBuffer(3);
  command_index = (uint8_t)idx;
  int action = readBuffer(4);
  int device = readBuffer(5);
  int port = readBuffer(6);
  switch(action){
    case GET:{
      if(device == ULTRASONIC) {
        if(!isUltrasonic) {
          setUltrasonicMode(true);
          trigPin = readBuffer(6);
          echoPin = readBuffer(7);
          digitals[trigPin] = 1;
          digitals[echoPin] = 1;
          up_mdelay(50);
        } else {
          int trig = readBuffer(6);
          int echo = readBuffer(7);
          if(trig != trigPin || echo != echoPin) {
            digitals[trigPin] = 0;
            digitals[echoPin] = 0;
            trigPin = trig;
            echoPin = echo;
            digitals[trigPin] = 1;
            digitals[echoPin] = 1;
            up_mdelay(50);
          }
        }
      } else if(port == trigPin || port == echoPin) {
        setUltrasonicMode(false);
        digitals[port] = 0;
      } else {
        setUltrasonicMode(false);
        digitals[port] = 0;
      }
    }
    break;
    case SET:{
      runModule(device);
      callOK();
    }
    break;
    case RESET:{
      callOK();
    }
    break;
  }
}

void setPinValue(unsigned char c) {
  if(c==0x55&&isStart==false){
    if(prevc==0xff){
      inDex=1;
      isStart = true;
    }
  } else {
    prevc = c;
    if(isStart) {
      if(inDex==2){
        dataLen = c;
      } else if(inDex>2) {
        dataLen--;
      }

      writeBuffer(inDex,c);
    }
  }

  inDex++;

  if(inDex>51) {
    inDex=0;
    isStart=false;
  }


  if(isStart&&inDex>3&&dataLen == 0){
    isStart = false;
    parseData();
    inDex=0;
  }

}
void writeHead(){
	 int idx_tx = 0;

	  char buf_tx[16] = {0,};

	  int fd = open(buf, O_RDWR | O_NOCTTY | O_NONBLOCK);

	  buf_tx[idx_tx] = 0xff;
	  idx_tx++;
	  buf_tx[idx_tx] = 0x55;
	  idx_tx++;
	  int i;
	  for( i = 0 ; i < idx_tx ; i++){
			  write(fd, &buf_tx[i], 1);
	  }
		close(fd);
}

void writeSerial(char c){
	 int idx_tx = 0;

	  char buf_tx[16] = {0,};

	  int fd = open(buf, O_RDWR | O_NOCTTY | O_NONBLOCK);

	  buf_tx[idx_tx] = c;
	  idx_tx++;
	  int i;
	  for( i = 0 ; i < idx_tx ; i++){
			  write(fd, &buf_tx[i], 1);
	  }
		close(fd);
}

void writeEnd(){
	 int idx_tx = 0;

	  char buf_tx[16] = {0,};

	  int fd = open(buf, O_RDWR | O_NOCTTY | O_NONBLOCK);

	  buf_tx[idx_tx] = 10;
	  idx_tx++;
	  int i;
	  for( i = 0 ; i < idx_tx ; i++){
			  write(fd, &buf_tx[i], 1);
	  }
		close(fd);
}






void sendDigitalValue(int pinNumber) {

  int idx_tx = 0;

 	  char buf_tx[16] = {0,};

 	  int fd = open(buf, O_RDWR | O_NOCTTY | O_NONBLOCK);

 	  float digitalVal  = gpio_read(gpio_pin[pinNumber]);

 	  buf_tx[idx_tx] = 0xff;
 	  idx_tx++;
 	  buf_tx[idx_tx] = 0x55;
 	  idx_tx++;
 	  buf_tx[idx_tx] = 2;
 	  idx_tx++;
 	  val.floatVal = digitalVal;
 	  buf_tx[idx_tx] = val.byteVal[0];
 	  idx_tx++;
 	  buf_tx[idx_tx] = val.byteVal[1];
 	  idx_tx++;
 	  buf_tx[idx_tx] = val.byteVal[2];
 	  idx_tx++;
 	  buf_tx[idx_tx] = val.byteVal[3];
 	  idx_tx++;
 	  buf_tx[idx_tx] = pinNumber;
 	  idx_tx++;
 	  buf_tx[idx_tx] = DIGITAL;
 	  idx_tx++;
 	  buf_tx[idx_tx] = 10;
 	  idx_tx++;
 	  int i;
 	  for( i = 0 ; i < idx_tx ; i++){
 			  write(fd, &buf_tx[i], 1);
 	  }
 		close(fd);
}

void sendAnalogValue(int pinNumber) {

	  int idx_tx = 0;

	  char buf_tx[16] = {0,};

	  int fd = open(buf, O_RDWR | O_NOCTTY | O_NONBLOCK);

	  float analogVal  = read_adc(pinNumber);

	  buf_tx[idx_tx] = 0xff;
	  idx_tx++;
	  buf_tx[idx_tx] = 0x55;
	  idx_tx++;
	  buf_tx[idx_tx] = 2;
	  idx_tx++;
	  val.floatVal = analogVal;
	  buf_tx[idx_tx] = val.byteVal[0];
	  idx_tx++;
	  buf_tx[idx_tx] = val.byteVal[1];
	  idx_tx++;
	  buf_tx[idx_tx] = val.byteVal[2];
	  idx_tx++;
	  buf_tx[idx_tx] = val.byteVal[3];
	  idx_tx++;
	  buf_tx[idx_tx] = pinNumber;
	  idx_tx++;
	  buf_tx[idx_tx] = ANALOG;
	  idx_tx++;
	  buf_tx[idx_tx] = 10;
	  idx_tx++;
	  int i;
	  for( i = 0 ; i < idx_tx ; i++){
			  write(fd, &buf_tx[i], 1);
	  }
		close(fd);
}

void sendPinValues() {
  int pinNumber = 0;
  for (pinNumber = 0; pinNumber < 9; pinNumber++) {
    if(digitals[pinNumber] == 0) {
      sendDigitalValue(pinNumber);
    	callOK();
    }
  }
  for (pinNumber = 0; pinNumber < 4; pinNumber++) {
    if(analogs[pinNumber] == 0) {
      sendAnalogValue(pinNumber);
      callOK();
    }
  }

}

void setUltrasonicMode(bool mode) {
  isUltrasonic = mode;
  if(!mode) {
    lastUltrasonic = 0;
  }
}

void sendUltrasonic() {
  gpio_write(gpio_pin[trigPin], LOW);
  up_mdelay(2);
  gpio_write(gpio_pin[trigPin], HIGH);
  up_mdelay(10);
  gpio_write(gpio_pin[trigPin], LOW);

  float value = 500 / 29.0 / 2.0;

  if(value == 0) {
    value = lastUltrasonic;
  } else {
    lastUltrasonic = value;
  }
  writeHead();
  sendFloat(value);
  writeSerial(trigPin);
  writeSerial(echoPin);
  writeSerial(ULTRASONIC);
  writeEnd();
}





void sendShort(double value){
  writeSerial(3);
  valShort.shortVal = value;
  writeSerial(valShort.byteVal[0]);
  writeSerial(valShort.byteVal[1]);
}

float readFloat(int idx){
  val.byteVal[0] = readBuffer(idx);
  val.byteVal[1] = readBuffer(idx+1);
  val.byteVal[2] = readBuffer(idx+2);
  val.byteVal[3] = readBuffer(idx+3);
  return val.floatVal;
}

long readLong(int idx){
  val.byteVal[0] = readBuffer(idx);
  val.byteVal[1] = readBuffer(idx+1);
  val.byteVal[2] = readBuffer(idx+2);
  val.byteVal[3] = readBuffer(idx+3);
  return val.longVal;
}

int searchServoPin(int pin){
	int i;
  for(i=0;i<8;i++){
    if(servo_pins[i] == pin){
      return i;
    }
    if(servo_pins[i]==0){
      servo_pins[i] = pin;
      return i;
    }
  }
  return 0;
}

void setPortWritable(int pin) {
  if(digitals[pin] == 0) {
    digitals[pin] = 1;
  }
}



void callDebug(char c){
  writeSerial(0xff);
  writeSerial(0x55);
  writeSerial(c);
  writeEnd();
}

int main(int argc, char *argv[])
{
	int i;



		for (i = 0; i < 4 ; i++ ){
			fd_pwm[i] = pwm_open(pwm_pin[i]);
			pwm_write(fd_pwm[i],256,0);
		}


		while(1){

			int fd = open(buf, O_RDWR | O_NOCTTY);
			struct timeval tv;
			fd_set readfds;
			FD_ZERO(&readfds);
			FD_SET(fd, &readfds);

			tv.tv_sec = 0;
			tv.tv_usec = 10000;

				char c[16] = {0,};

				if (select(fd + 1, &readfds, NULL, NULL, &tv) >= 0 && FD_ISSET(fd, &readfds)) {
					read(fd, c, 16);
				}
				int k = strlen(c);
				if(k>0){
					for(i = 0 ; i < 16 ; i++){
						setPinValue(c[i]&0xff);
					}
				}
				close(fd);

				  up_mdelay(15);

				  sendPinValues();

				  up_mdelay(20);

			}



	return 0;

}


