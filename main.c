#include <device.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define PID_CHARS 4

int get_distance_in_cm(void);
int calc_PID(int distance);
void update_PID();

//Globals
int loop_counter = 0;
double Proportional = 25.0;
double Integral = 0.025;
double Derivative = 30.0;
int desired = 40;

/* PWM can be between 0 and 1200 */
int main()
{
 char buffer[100] = {0};
 int isData = 0;
 int distance = 0;
 // Initialization Code:
 CYGlobalIntEnable;
 USBUART_1_Start(0, USBUART_1_5V_OPERATION);//!!NOTE!! Make sure this matches your board voltage!
 while(!USBUART_1_GetConfiguration());
 USBUART_1_CDC_Init();
    // Initial pwm value
    uint16_t pwm_input = 840;
    bool toggle = false;
    PWM_1_Start();
    int x=1;
    for(;;)
    {
        /*
        //Push button for testing
        while(x==1){ //wait for press
            x = sw_Read();
        }
        while(x!=1){ //wait for release
            x = sw_Read();
        }*/
        /*
        LED_2_Write(0); //turn off
        CyDelay(50);
        LED_2_Write(1); //turn on
        CyDelay(50);
        */
        /*
        if (!toggle){
            pwm_input+=20;
            LED_Write(1); //LED displays rising
        }
        else{
            pwm_input = pwm_input-20;
            LED_Write(0);//LED displays falling
        }
        if (pwm_input>1170||pwm_input<800){
            toggle=!toggle;
        }*/

        //Check to see if anything has been written to the terminal.
        isData = USBUART_1_GetCount();
        if(isData!= 0) // Check for input data from PC
        {
            update_PID();
        }
        distance = get_distance_in_cm();
        pwm_input = calc_PID(distance);
        PWM_1_WriteCompare(pwm_input);
    }
}

int integral_error = 0;
int min_integral = -10000;
int max_integral = 10000;
int last_error = 0;
int last_pid_out = 0;
int calc_PID(int distance) {
    int error  = distance-desired;
    integral_error += error;
    if (integral_error>=max_integral) {
        integral_error = max_integral;
    }
    else if (integral_error<=min_integral) {
        integral_error = min_integral;
    }
    double p = Proportional * (double)error;
    double i = Integral * (double)integral_error;
    double d = Derivative * (double)(error - last_error);
    last_error = error;
    int pid_out = (int)(p + i + d);
    last_pid_out = pid_out;
    if (pid_out>=600) {
        pid_out = 600;
    }
    if (pid_out <= 0) {
        pid_out = 0;
    }
    //We max out at 1200 but anything less than 800 the fan basically doesn't spin
    return pid_out+600;
}

int get_distance_in_cm(void) {
        char buffer[100] = {0};
        int Echo_Pin_Val = 0;
        uint32_t count = 0;
        /*
        Ultrasonic Senosor
        */
        Trigger_Signal_Write(1);
        CyDelayUs(10);
        Trigger_Signal_Write(0);
        while(!Echo_Pin_Val)
        {
            Echo_Pin_Val = Echo_Signal_Read();
        }
        Timer_1_Start();
        while(Echo_Pin_Val)
        {
            Echo_Pin_Val = Echo_Signal_Read();
        }
        Timer_1_Stop();
        count = Timer_1_ReadCounter();
        uint32 dist = ((16777216 - count) * 50)/141176;
        if (loop_counter == 100) {
            loop_counter = 0;
            sprintf(buffer,"\r\ndistance : %ld cms",dist);
            USBUART_1_PutString(buffer);
            while(!USBUART_1_CDCIsReady()); // Wait for Tx to finish
        }
        loop_counter++;
        Timer_1_WriteCounter(0);
        return dist;
}
char echo(void);
void update_PID(void) {
    int i = 0;
    char setpoint[2] = {0};
    char P[PID_CHARS+1] = {0}; 
    char I[PID_CHARS+1] = {0};
    char D[PID_CHARS+1] = {0};
    
    USBUART_1_PutString("\r\nsetpoint = ");
    while(!USBUART_1_CDCIsReady()); // Wait for Tx to finish
    //We already have p[0]
    setpoint[0] = echo();
    //Wait for the dest 2
    while(!USBUART_1_GetCount());
    setpoint[1] = echo();
    /*
    USBUART_1_PutString("\r\nP = ");
    while(!USBUART_1_CDCIsReady()); // Wait for Tx to finish
        for (i=0; i<PID_CHARS; i++) {
        //Wait for the P
        while(!USBUART_1_GetCount());
        P[i] = echo();
    }
    
    USBUART_1_PutString("\r\nI = ");
    while(!USBUART_1_CDCIsReady()); // Wait for Tx to finish
        for (i=0; i<PID_CHARS; i++) {
        //Wait for the I
        while(!USBUART_1_GetCount());
        I[i] = echo();
    }
    
    USBUART_1_PutString("\r\nD = ");
    while(!USBUART_1_CDCIsReady()); // Wait for Tx to finish
        for (i=0; i<PID_CHARS; i++) {
        //Wait for the D
        while(!USBUART_1_GetCount());
        D[i] = echo();
    }
    
    USBUART_1_PutString("\r\n");
    while(!USBUART_1_CDCIsReady()); // Wait for Tx to finish
    */
    desired = atoi(setpoint);
    //Proportional = atof(P);
    //Integral = atof(I);
    //Derivative = atof(D);
            
}
            
char echo(void) {
    char buffer[100] = {0};
    while(!USBUART_1_CDCIsReady()); // Wait for Tx to finish
    USBUART_1_GetAll(buffer);
    USBUART_1_PutString(buffer);
    while(!USBUART_1_CDCIsReady()); // Wait for Tx to finish
    return buffer[0];
}

/* [] END OF FILE */
