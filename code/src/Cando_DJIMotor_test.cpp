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
int ch=0;
int loop=0;
int state_selection = 1;//state_selection为0时，代表速度控制，state_selection为1时，代表1位置控制
int16_t  desired_current[6] = {0,0,0,0,0,0}; //定义为全局变量
int16_t  angle[6]={200,-200,200,-200,200,-200};

struct can_frame rxframe;
struct can_frame txframe[2];

All_motor_info all_motor_info={0};

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
  printf("     %d \n",ch);
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  if (t==27)
  break;
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

Sleep(10);

  txframe[1].can_id = CAN2;
  txframe[1].can_dlc = 8;
  for (int i = 0; i < 8; i++)
    txframe[1].data[i] = 0;
  nbytes = write(s, &txframe[1], sizeof(struct can_frame));
  return s;
}

void canTx(int16_t  Current_sent[6],int s)
{
for(int i=0;i<4;i++){
  txframe[0].data[2*i] = Current_sent[i] >> 8; // done: negative with type convertion
  txframe[0].data[2*i+1] = Current_sent[i]&&255;
}
write(s, &txframe[0], sizeof(struct can_frame)); 


for(int i=0;i<2;i++){
  txframe[1].data[2*i]=Current_sent[i+4] >> 8;
  txframe[1].data[2*i+1] = Current_sent[i+4]&&255;
}
write(s, &txframe[1], sizeof(struct can_frame)); 

}


void canRx( int s)
{ 
  while(1){
  //info_lock.lock();  //lock info
  if (ch==119)
  break;
  int  nbytes = read(s, &rxframe, sizeof(struct can_frame));
  if (nbytes < 0)
  {
    perror("Read");
  }
  }
}

void get_moto_offset(){
  for(int i=0;i<100;i++){
  int  nbytes = read(s, &rxframe, sizeof(struct can_frame));
  if (nbytes < 0)
  {
    perror("Read");
  }
  int id=(uint16_t)rxframe.can_id-513;
  all_motor_info.motor_Control_Info[id].angle_offset= (uint16_t)(rxframe.data[0] << 8 | rxframe.data[1]);
  all_motor_info.motor_Control_Info[id].angle_now=  (uint16_t)(rxframe.data[0] << 8 | rxframe.data[1]);
  Sleep(1);
  }
}

void rhex_init()
{  
   int16_t init_current[4]={600,-500,600,-600};
  for(int i=0;i++;i<100){
  canTx(init_current,s);
  sleep(0.1);
  }

}

void positionPID(int16_t increase_angle[4])
{  
 while(1){
   int id=(uint16_t)rxframe.can_id-513;
  all_motor_info.motor_Control_Info[id].angle_last=all_motor_info.motor_Control_Info[id].angle_now;
  all_motor_info.motor_Control_Info[id].angle_now = (uint16_t)(rxframe.data[0] << 8 | rxframe.data[1]);
  all_motor_info.motor_Control_Info[id].rpm_now = (uint16_t)(rxframe.data[2] << 8 |rxframe.data[3]);
  all_motor_info.motor_Control_Info[id].current_now = (int16_t)(rxframe.data[4] << 8 |rxframe.data[5]);
  if(all_motor_info.motor_Control_Info[id].angle_now-all_motor_info.motor_Control_Info[id].angle_last>5000)
    (all_motor_info.motor_Control_Info[id].round)--;
  else if(all_motor_info.motor_Control_Info[id].angle_now-all_motor_info.motor_Control_Info[id].angle_last<-5000)
    (all_motor_info.motor_Control_Info[id].round)++;
  all_motor_info.motor_Control_Info[id].angle_total=((((all_motor_info.motor_Control_Info[id].round))*8192+all_motor_info.motor_Control_Info[id].angle_now-all_motor_info.motor_Control_Info[id].angle_offset));
 
   for(int i=0;i<4;i++){
       int delta_angle =increase_angle[i] - all_motor_info.motor_Control_Info[i].angle_total;// e(t) => （对应kp）
       int diff_angle = delta_angle - all_motor_info.motor_Control_Info[i].angle_error2;// e(t)-e(t-1) => 差分（对应Kd）
       printf("id %d delta_angle: %d diff:%d round : %d  total: %d,angle_now: %d\n",i,delta_angle,diff_angle,all_motor_info.motor_Control_Info[i].round,all_motor_info.motor_Control_Info[i].angle_total,all_motor_info.motor_Control_Info[i].angle_now);
       all_motor_info.motor_Control_Info[i].integral_angle+= abs(all_motor_info.motor_Control_Info[i].integral_angle + delta_angle) < RpmIntegralSaturation ? delta_angle : 0;// e(0)+e(1)+e(2)+...e(t-1)
       desired_current[i] = (Kp_position * delta_angle+ Ki_position * all_motor_info.motor_Control_Info[i].integral_rpm + Kd_position * diff_angle);
         // update error last time
      all_motor_info.motor_Control_Info[i].angle_error1=delta_angle;
      all_motor_info.motor_Control_Info[i].angle_error2 = all_motor_info.motor_Control_Info[i].angle_error1;

      if (desired_current[i] > 5000)
        desired_current[i]=5000;
    else if (desired_current[i] < -5000)
        desired_current[i]=-5000;
   }
  printf("curent  %d   %d\n",desired_current[0],desired_current[1]);
    //`info_lock.unlock();
  canTx(desired_current,s);
  if(ch==119){
    for(int i=0;i<4;i++){
    desired_current[i]=0;
    canTx(desired_current,s);
    }
  }
 }
}

void speedPID(int16_t desired_rpm[6] )
{
 while(1){
   Sleep(1);
   int id=(uint16_t)rxframe.can_id-513;
  all_motor_info.motor_Control_Info[id].angle_last=all_motor_info.motor_Control_Info[id].angle_now;
  all_motor_info.motor_Control_Info[id].angle_now = (uint16_t)(rxframe.data[0] << 8 | rxframe.data[1]);
  all_motor_info.motor_Control_Info[id].rpm_now = (uint16_t)(rxframe.data[2] << 8 |rxframe.data[3]);
  all_motor_info.motor_Control_Info[id].current_now = (int16_t)(rxframe.data[4] << 8 |rxframe.data[5]);
  if(all_motor_info.motor_Control_Info[id].angle_now-all_motor_info.motor_Control_Info[id].angle_last>5000)
    (all_motor_info.motor_Control_Info[id].round)--;
  else if(all_motor_info.motor_Control_Info[id].angle_now-all_motor_info.motor_Control_Info[id].angle_last<-5000)
    (all_motor_info.motor_Control_Info[id].round)++;
  all_motor_info.motor_Control_Info[id].angle_total=((((all_motor_info.motor_Control_Info[id].round))*8192+all_motor_info.motor_Control_Info[id].angle_now-all_motor_info.motor_Control_Info[id].angle_offset));
 
   for(int i=0;i<6;i++){
       int delta_angle =desired_rpm[i] - all_motor_info.motor_Control_Info[i].rpm_now;// e(t) => （对应kp）
       int diff_angle = delta_angle - all_motor_info.motor_Control_Info[i].angle_error2;// e(t)-e(t-1) => 差分（对应Kd）
      // printf("id %d delta_angle: %d diff:%d rpm : %d \n",i,delta_angle,diff_angle,all_motor_info.motor_Control_Info[i].rpm_now);
       all_motor_info.motor_Control_Info[i].integral_angle+= abs(all_motor_info.motor_Control_Info[i].integral_angle + delta_angle) < RpmIntegralSaturation ? delta_angle : 0;// e(0)+e(1)+e(2)+...e(t-1)
       desired_current[i] = (Kp_speed * delta_angle+ Ki_speed * all_motor_info.motor_Control_Info[i].integral_rpm + Kd_speed * diff_angle);
         // update error last time
      all_motor_info.motor_Control_Info[i].angle_error1=delta_angle;
      all_motor_info.motor_Control_Info[i].angle_error2 = all_motor_info.motor_Control_Info[i].angle_error1;

      if (desired_current[i] > 5000)
        desired_current[i]=5000;
    else if (desired_current[i] < -5000)
        desired_current[i]=-5000;
   }
 // printf("curent  %d   %d\n",desired_current[0],desired_current[1]);
    //`info_lock.unlock();
  canTx(desired_current,s);
 }
}
int main()
{    

  s = socketCANSetUp();
  std::thread get_key(getch,std::ref(ch));
   //rhex_init();
   //int16_t init_current[4]={50,-40,60,-60};
  //canTx(init_current,s);
  Sleep(100);
  get_moto_offset();
  std::thread rx(canRx,s);
  std::thread tx(speedPID,angle);
  
   rx.join();
   tx.join();
   get_key.join();
  const char* set_down = "sudo ip link set down can0";
  system(set_down);

 
  return 0;
}
