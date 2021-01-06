// #include <chrono>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <time.h>

#include <termio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "Cando_DJIMotor_test.h"
#if defined(__linux__) || defined(__APPLE__)
#include <fcntl.h>
#include <termios.h>
#define STDIN_FILENO 0
#elif defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif
#define ESC_ASCII_VALUE                 0x1b
int key=0;  //keybroad input
int s= 0 ;  //can flag
int state_selection = 1;//state_selection为0时，代表速度控制，state_selection为1时，代表1位置控制
int desired_current[6] = {0}; //定义为全局变量

std::mutex info_lock;

struct can_frame rxframe;
struct can_frame txframe[2];

All_motor_info all_motor_info;

void Sleep(int ms)
{
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = ms * 1000; // 20 ms
    select(0, NULL, NULL, NULL, &delay);
}

void getch(int &t)
{ while(1){
#if defined(__linux__) || defined(__APPLE__)
  struct termios oldt, newt;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  t = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  // return ch;
#elif defined(_WIN32) || defined(_WIN64)
  return _getch();
#endif
}
}




// open network, configure socket CAN, return file descriptor
int socketCANSetUp(void)
{
  
  int nbytes;
  struct sockaddr_can addr;
  struct ifreq ifr;
  socklen_t len = sizeof(addr); // for recvfrom
  const char *ifname = "can0";

  /* set up can network in ubuntu */   
  const char* set_bitrate = "sudo ip link set can0 type can bitrate 1000000";
  const char* set_up = "sudo ip link set up can0";
  if(system(set_bitrate)||system(set_up))
    printf("set up network fail");

  /* open the socket */
  if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
  {
    perror("Error while opening socket");
    return -1;
  }

  /* get interface name from the received CAN frame */
  strcpy(ifr.ifr_name, ifname);
  ioctl(s, SIOCGIFINDEX, &ifr);

  addr.can_family = AF_CAN;           
  addr.can_ifindex = ifr.ifr_ifindex;

  /* bind socket to the can interface */
  if (bind(s, (struct sockaddr *)&addr, len) < 0)
  {
    perror("Error in socket bind!!!");
    return -2;
  }

  /* configure CAN info and fresh the data */
  txframe[0].can_id = CAN1;
  txframe[0].can_dlc = 8;
  for (int i = 0; i < 8; i++)
    txframe[0].data[i] = 0;
  nbytes = write(s, &txframe[0], sizeof(struct can_frame));

 /*  txframe[1].can_id = CAN2;
  txframe[1].can_dlc = 8;
  for (int i = 0; i < 8; i++)
    txframe[1].data[i] = 0;
  nbytes = write(s, &txframe[1], sizeof(struct can_frame));
  */


  return s;
}



void canTx(int16_t  Current_sent[4],int s)
{
 
  /* waiting to add other motor */
for(int i=0;i<2;i++){
  txframe[0].data[2*i] = Current_sent[i] >> 8; // done: negative with type convertion
  txframe[0].data[2*i+1] = Current_sent[i] ;
}
  write(s, &txframe[0], sizeof(struct can_frame));
  printf("%d  %d\n",txframe[0].data[0],txframe[0].data[1]);
  printf("%d  %d\n",txframe[0].data[2],txframe[0].data[3]);
  printf("%d  %d\n",txframe[0].data[4],txframe[0].data[5]);
  printf("%d  %d\n",txframe[0].data[6],txframe[0].data[7]);
  
  //write(s, &txframe[0], sizeof(struct can_frame));
  
}
void rhex_init()
{  int16_t init_current[2]={829,829};

   canTx(init_current,s);

}

int main()
{    

   s = socketCANSetUp();

   rhex_init();
   
  const char* set_down = "sudo ip link set down can0";
  system(set_down);

 
  return 0;
}
