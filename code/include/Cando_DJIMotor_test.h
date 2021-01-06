#include <string>
//#include "std_msgs.h"
#include <sstream>

#define target_rpm -2500

#define Kp_speed 3.6
#define Ki_speed 1.0
#define Kd_speed 2.0
#define Kp_T 1.50
#define Ki_T 0.40
#define Kd_T 0.10
#define RpmIntegralSaturation 600 // highly influence the wave

#define Kp_position 1
#define Ki_position 0.1
#define Kd_position 40
#define positionIntegralSaturation 6000

#define CAN1 0x200
#define CAN2 0x1FF

#define Tx_C610_1_High 0
#define Tx_C610_1_Low 1
#define Tx_C610_2_High 2
#define Tx_C610_2_Low 3
#define Tx_C610_3_High 4
#define Tx_C610_3_Low 5
#define Tx_C610_4_High 6
#define Tx_C610_4_Low 7
#define Tx_C610_5_High 0
#define Tx_C610_5_Low 1
#define Tx_C610_6_High 2
#define Tx_C610_6_Low 3
#define MaxCurrent 10000
#define MinCurrent -10000

// Rx data frame
#define Rx_C610_1ID 0x202
#define Angle_High 0
#define Angle_Low 1
#define Omega_High 2
#define Omega_Low 3
#define Current_High 4
#define Current_Low 5

typedef struct 
{
    // last_time for error
    int16_t rpm_error2;
    int16_t rpm_now;
    int integral_rpm;

    int16_t current_last_time;
    int16_t current_now;

    int angle_error1;
    int angle_error2;
    int angle_last;
    int angle_now;
    int angle_total;
    int angle_offset;
    int abgle_goal;
    int integral_angle;
    long int round;
} Motor_Control_Info;

typedef struct 
{
  Motor_Control_Info motor_Control_Info[6]; 
}All_motor_info;