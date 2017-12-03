// Lab3.c
// Runs on either MSP432 or TM4C123
// Starter project to Lab 3.  Take sensor readings, process the data,
// and output the results.  Specifically, this program will
// measure steps using the accelerometer, audio sound amplitude using
// the microphone, temperature using the TMP006, and light using the
// OPT3001.
// Daniel and Jonathan Valvano
// March 24, 2016

/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2016

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdint.h>
#include "../../Shared/Edx_files/BSP.h"
#include "../../Shared/Edx_files/CortexM.h"
#include "os.h"
#include "../../Shared/Edx_files/Profile.h"
#include "../../Shared/Edx_files/Texas.h"

uint32_t sqrt32(uint32_t s);
#define THREADFREQ 1000   // frequency in Hz of round robin scheduler





//---------------- Global variables shared between tasks ----------------
uint32_t Time;              // elasped time in 100 ms units
uint32_t Steps;             // number of steps counted
uint32_t Magnitude;         // will not overflow (3*1,023^2 = 3,139,587)
                            // Exponentially Weighted Moving Average
uint32_t EWMA;              // https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
uint16_t SoundData;         // raw data sampled from the microphone
int32_t SoundAvg;

uint32_t SoundRMS;          // Root Mean Square average of most recent sound samples
uint32_t LightData;         // 100 lux
int32_t TemperatureData;    // 0.1C
// semaphores
int32_t NewData;  // true when new numbers to display on top of LCD
int32_t LCDmutex; // exclusive access to LCD
int32_t I2Cmutex; // exclusive access to I2C
int ReDrawAxes = 0;         // non-zero means redraw axes on next display task






enum plotstate{
  Accelerometer,
  Microphone,
  Temperature,
  Light
};
enum plotstate PlotState = Accelerometer;
//color constants
#define BGCOLOR     LCD_BLACK
#define AXISCOLOR   LCD_ORANGE
#define MAGCOLOR    LCD_YELLOW
#define EWMACOLOR   LCD_CYAN
#define SOUNDCOLOR  LCD_CYAN
#define LIGHTCOLOR  LCD_WHITE  //LCD_RED
#define TEMPCOLOR   LCD_LIGHTGREEN
#define TOPTXTCOLOR LCD_WHITE
#define TOPNUMCOLOR LCD_ORANGE
//------------ end of Global variables shared between tasks -------------

//---------------- Task0 samples sound from microphone ----------------
// Event thread run by OS in real time at 1000 Hz
#define SOUNDRMSLENGTH 1000 // number of samples to collect before calculating RMS (may overflow if greater than 4104)
int16_t SoundArray[SOUNDRMSLENGTH];
// *********Task0_Init*********
// initializes microphone
// Task0 measures sound intensity
// Inputs:  none
// Outputs: none
void Task0_Init(void){
  BSP_Microphone_Init();
  SoundRMS = 0;
}
// *********Task0*********
// Periodic event thread runs in real time at 1000 Hz
// collects data from microphone
// Inputs:  none
// Outputs: none
void Task0(void){
  static int32_t soundSum = 0;
  static int time = 0;// units of microphone sampling rate

  TExaS_Task0();     // record system time in array, toggle virtual logic analyzer
 // Profile_Toggle0(); // viewed by a real logic analyzer to know Task0 started
  BSP_Microphone_Input(&SoundData);
  soundSum = soundSum + (int32_t)SoundData;
  SoundArray[time] = SoundData;
  time = time + 1;
  if(time == SOUNDRMSLENGTH){
    SoundAvg = soundSum/SOUNDRMSLENGTH;
    soundSum = 0;
    OS_Signal(&NewData); // makes task5 run every 1 sec
    time = 0;
  }
}
/* ****************************************** */
/*          End of Task0 Section              */
/* ****************************************** */

//---------------- Task1 measures acceleration ----------------
// Event thread run by OS in real time at 10 Hz
uint32_t LostTask1Data;     // number of times that the FIFO was full when acceleration data was ready
uint16_t AccX, AccY, AccZ;  // returned by BSP as 10-bit numbers
#define ALPHA 128           // The degree of weighting decrease, a constant smoothing factor between 0 and 1,023. A higher ALPHA discounts older observations faster.
                            // basic step counting algorithm is based on a forum post from
                            // http://stackoverflow.com/questions/16392142/android-accelerometer-profiling/16539643#16539643
enum state{                 // the step counting algorithm cycles through four states
  LookingForMax,            // looking for a local maximum in current magnitude
  LookingForCross1,         // looking for current magnitude to cross average magnitude, minus a constant
  LookingForMin,            // looking for a local minimum in current magnitude
  LookingForCross2          // looking for current magnitude to cross average magnitude, plus a constant
};
enum state AlgorithmState = LookingForMax;
#define LOCALCOUNTTARGET 5  // The number of valid measured magnitudes needed to confirm a local min or local max.  Increase this number for longer strides or more frequent measurements.
#define AVGOVERSHOOT 25     // The amount above or below average a measurement must be to count as "crossing" the average.  Increase this number to reject increasingly hard shaking as steps.
// *********Task1_Init*********
// initializes accelerometer
// Task1 counts Steps
// Inputs:  none
// Outputs: none
void Task1_Init(void){
  BSP_Accelerometer_Init();
  // initialize the exponential weighted moving average filter
  BSP_Accelerometer_Input(&AccX, &AccY, &AccZ);
  Magnitude = sqrt32(AccX*AccX + AccY*AccY + AccZ*AccZ);
  EWMA = Magnitude;                // this is a guess; there are many options
  Steps = 0;
  LostTask1Data = 0;
}
// *********Task1*********
// collects data from accelerometer
// Inputs:  none
// Outputs: none
void Task1(void){uint32_t squared;
  TExaS_Task1();     // records system time in array, toggles virtual logic analyzer
 // Profile_Toggle1(); // viewed by a real logic analyzer to know Task1 started

  BSP_Accelerometer_Input(&AccX, &AccY, &AccZ);
  squared = AccX*AccX + AccY*AccY + AccZ*AccZ;
  if(OS_FIFO_Put(squared) == -1){  // makes Task2 run every 100ms
    LostTask1Data = LostTask1Data + 1;
  }
  Time++; // in 100ms units
}
/* ****************************************** */
/*          End of Task1 Section              */
/* ****************************************** */


//---------------- Task2 calculates steps and plots data on LCD ----------------
// Main thread scheduled by OS round robin preemptive scheduler
// accepts data from accelerometer, calculates steps, plots on LCD
// If no data are lost, the main loop in Task2 runs exactly at 10 Hz, but not in real time
#define ACCELERATION_MAX 1400
#define ACCELERATION_MIN 600
#define SOUND_MAX 900
#define SOUND_MIN 300
#define LIGHT_MAX 2000
#define LIGHT_MIN 0
#define TEMP_MAX 1023
#define TEMP_MIN 0
void drawaxes(void){
  OS_Wait(&LCDmutex);
  if(PlotState == Accelerometer){
    BSP_LCD_Drawaxes(AXISCOLOR, BGCOLOR, "Time", "Mag", MAGCOLOR, "Ave", EWMACOLOR, ACCELERATION_MAX, ACCELERATION_MIN);
  } else if(PlotState == Microphone){
    BSP_LCD_Drawaxes(AXISCOLOR, BGCOLOR, "Time", "Sound", SOUNDCOLOR, "", 0, SoundData+100, SoundData-100);
  } else if(PlotState == Temperature){
    BSP_LCD_Drawaxes(AXISCOLOR, BGCOLOR, "Time", "Temp", TEMPCOLOR, "", 0, TEMP_MAX, TEMP_MIN);
  } else if(PlotState == Light){
    BSP_LCD_Drawaxes(AXISCOLOR, BGCOLOR, "Time", "Light", LIGHTCOLOR, "", 0, LIGHT_MAX, LIGHT_MIN);
  }
  OS_Signal(&LCDmutex);  ReDrawAxes = 0;
}
void Task2(void){uint32_t data;
  uint32_t localMin;   // smallest measured magnitude since odd-numbered step detected
  uint32_t localMax;   // largest measured magnitude since even-numbered step detected
  uint32_t localCount; // number of measured magnitudes above local min or below local max
  localMin = 1024;
  localMax = 0;
  localCount = 0;
  drawaxes();
  while(1){
    data = OS_FIFO_Get();
    TExaS_Task2();     // records system time in array, toggles virtual logic analyzer
 //   Profile_Toggle2(); // viewed by a real logic analyzer to know Task2 started
    Magnitude = sqrt32(data);
    EWMA = (ALPHA*Magnitude + (1023 - ALPHA)*EWMA)/1024;
    if(AlgorithmState == LookingForMax){
      if(Magnitude > localMax){
        localMax = Magnitude;
        localCount = 0;
      } else{
        localCount = localCount + 1;
        if(localCount >= LOCALCOUNTTARGET){
          AlgorithmState = LookingForCross1;
        }
      }
    } else if(AlgorithmState == LookingForCross1){
      if(Magnitude > localMax){
      // somehow measured a very large magnitude
        localMax = Magnitude;
        localCount = 0;
        AlgorithmState = LookingForMax;
      } else if(Magnitude < (EWMA -  AVGOVERSHOOT)){
        // step detected
        Steps = Steps + 1;
        localMin = 1024;
        localCount = 0;
        AlgorithmState = LookingForMin;
      }
    } else if(AlgorithmState == LookingForMin){
      if(Magnitude < localMin){
        localMin = Magnitude;
        localCount = 0;
      } else{
        localCount = localCount + 1;
        if(localCount >= LOCALCOUNTTARGET){
          AlgorithmState = LookingForCross2;
        }
      }
    } else if(AlgorithmState == LookingForCross2){
      if(Magnitude < localMin){
      // somehow measured a very small magnitude
        localMin = Magnitude;
        localCount = 0;
        AlgorithmState = LookingForMin;
      } else if(Magnitude > (EWMA + AVGOVERSHOOT)){
        // step detected
        Steps = Steps + 1;
        localMax = 0;
        localCount = 0;
        AlgorithmState = LookingForMax;
      }
    }
    if(ReDrawAxes){
      drawaxes();
      ReDrawAxes = 0;
    }
    OS_Wait(&LCDmutex);
    if(PlotState == Accelerometer){
      BSP_LCD_PlotPoint(Magnitude, MAGCOLOR);
      BSP_LCD_PlotPoint(EWMA, EWMACOLOR);
    } else if(PlotState == Microphone){
      BSP_LCD_PlotPoint(SoundData, SOUNDCOLOR);
    } else if(PlotState == Temperature){
      BSP_LCD_PlotPoint(TemperatureData, TEMPCOLOR);
    } else if(PlotState == Light){
      BSP_LCD_PlotPoint(LightData, LIGHTCOLOR);
    }
    BSP_LCD_PlotIncrement();
    OS_Signal(&LCDmutex);
  }
}
/* ****************************************** */
/*          End of Task2 Section              */
/* ****************************************** */


//------------Task3 handles switch input, buzzer output, LED output-------
// *********Task3*********
// Main thread scheduled by OS round robin preemptive scheduler
// non-real-time task
// checks the switches, updates the mode, and outputs to the buzzer and LED
// Inputs:  none
// Outputs: none
void Task3(void){
  static uint8_t prev1 = 0, prev2 = 0;
  uint8_t current;
  while(1){
    TExaS_Task3();     // records system time in array, toggles virtual logic analyzer
  //  Profile_Toggle3(); // viewed by a real logic analyzer to know Task3 started
    BSP_Buzzer_Set(0);
    current = BSP_Button1_Input();
    if((current == 0) && (prev1 != 0)){
      // Button1 was pressed since last loop
      if(PlotState == Accelerometer){
        PlotState = Microphone;
      } else if(PlotState == Microphone){
        PlotState = Temperature;
      } else if(PlotState == Temperature){
        PlotState = Light;
      } else if(PlotState == Light){
        PlotState = Accelerometer;
      }
      ReDrawAxes = 1;                // redraw axes on next call of display task
      BSP_Buzzer_Set(512);           // beep until next call of this task
    }
    prev1 = current;
    current = BSP_Button2_Input();
    if((current == 0) && (prev2 != 0)){
      // Button2 was pressed since last loop
      if(PlotState == Accelerometer){
        PlotState = Light;
      } else if(PlotState == Microphone){
        PlotState = Accelerometer;
      } else if(PlotState == Temperature){
        PlotState = Microphone;
      } else if(PlotState == Light){
        PlotState = Temperature;
      }
      ReDrawAxes = 1;                // redraw axes on next call of display task
      BSP_Buzzer_Set(512);           // beep until next call of this task
    }
    prev2 = current;
    // update the LED
    switch(AlgorithmState){
      case LookingForMax: BSP_RGB_Set(500, 0, 0); break;
      case LookingForCross1: BSP_RGB_Set(350, 350, 0); break;
      case LookingForMin: BSP_RGB_Set(0, 500, 0); break;
      case LookingForCross2: BSP_RGB_Set(0, 0, 500); break;
      default: BSP_RGB_Set(0, 0, 0);
    }
    OS_Sleep(10); // debounce the switches
  }
}
/* ****************************************** */
/*          End of Task3 Section              */
/* ****************************************** */


//------------Task4 measures temperature-------
// *********Task4*********
// Main thread scheduled by OS round robin preemptive scheduler
// measures temperature
// Inputs:  none
// Outputs: none
void Task4(void){int32_t voltData,tempData;
  int done;
  while(1){
    TExaS_Task4();     // records system time in array, toggles virtual logic analyzer
   // Profile_Toggle4(); // viewed by a real logic analyzer to know Task4 started

    OS_Wait(&I2Cmutex);
    BSP_TempSensor_Start();
    OS_Signal(&I2Cmutex);
    done = 0;
    OS_Sleep(1000);    // waits about 1 sec
    while(done == 0){
      OS_Wait(&I2Cmutex);
      done = BSP_TempSensor_End(&voltData, &tempData);
      OS_Signal(&I2Cmutex);
    }
    TemperatureData = tempData/10000;
  }
}
/* ****************************************** */
/*          End of Task4 Section              */
/* ****************************************** */


/* ------------------------------------------ */
//------- Task5 displays text on LCD -----------
/* ------------------------------------------ */
// If no data are lost, the main loop in Task5 runs exactly at 1 Hz, but not in real time

// *********Task5*********
// Main thread scheduled by OS round robin preemptive scheduler
// updates the text at the top and bottom of the LCD
// Inputs:  none
// Outputs: none
void Task5(void){int32_t soundSum;
  OS_Wait(&LCDmutex);
  BSP_LCD_DrawString(0,  0, "Temp=",  TOPTXTCOLOR);
  BSP_LCD_DrawString(0,  1, "Step=",  TOPTXTCOLOR);
  BSP_LCD_DrawString(10, 0, "Light=", TOPTXTCOLOR);
  BSP_LCD_DrawString(10, 1, "Sound=", TOPTXTCOLOR);
  OS_Signal(&LCDmutex);
  while(1){
    OS_Wait(&NewData);
    TExaS_Task5();     // records system time in array, toggles virtual logic analyzer
    //Profile_Toggle5(); // viewed by a real logic analyzer to know Task5 started
    soundSum = 0;
    for(int i=0; i<SOUNDRMSLENGTH; i=i+1){
      soundSum = soundSum + (SoundArray[i] - SoundAvg)*(SoundArray[i] - SoundAvg);
    }
    SoundRMS = sqrt32(soundSum/SOUNDRMSLENGTH);
    OS_Wait(&LCDmutex);
    BSP_LCD_SetCursor(5,  0); BSP_LCD_OutUFix2_1(TemperatureData, TEMPCOLOR);
    BSP_LCD_SetCursor(5,  1); BSP_LCD_OutUDec4(Steps,             MAGCOLOR);
    BSP_LCD_SetCursor(16, 0); BSP_LCD_OutUDec4(LightData,         LIGHTCOLOR);
    BSP_LCD_SetCursor(16, 1); BSP_LCD_OutUDec4(SoundRMS,          SOUNDCOLOR);
    BSP_LCD_SetCursor(16,12); BSP_LCD_OutUDec4(Time/10,           TOPNUMCOLOR);
//debug code
    if(LostTask1Data){
      BSP_LCD_SetCursor(0, 12); BSP_LCD_OutUDec4(LostTask1Data, BSP_LCD_Color565(255, 0, 0));
    }
//end of debug code
    OS_Signal(&LCDmutex);
  }
}
/* ****************************************** */
/*          End of Task5 Section              */
/* ****************************************** */

//---------------- Task6 measures light ----------------
// *********Task6*********
// Main thread scheduled by OS round robin preemptive scheduler
// Task6 measures light intensity
// Inputs:  none
// Outputs: none
void Task6(void){ uint32_t lightData;
  int done;
  while(1){
    TExaS_Task6();     // records system time in array, toggles virtual logic analyzer
   // Profile_Toggle6(); // viewed by a real logic analyzer to know Task6 started

    OS_Wait(&I2Cmutex);
    BSP_LightSensor_Start();
    OS_Signal(&I2Cmutex);
    done = 0;
    OS_Sleep(800);     // waits about 0.8 sec
    while(done == 0){
      OS_Wait(&I2Cmutex);
      done = BSP_LightSensor_End(&lightData);
      OS_Signal(&I2Cmutex);
    }
    LightData = lightData/100;
  }
}
/* ****************************************** */
/*          End of Task6 Section              */
/* ****************************************** */

//---------------- Task7 dummy function ----------------
// *********Task7*********
// Main thread scheduled by OS round robin preemptive scheduler
// Task7 does nothing but never blocks or sleeps
// Inputs:  none
// Outputs: none
uint32_t Count7;
void Task7(void){
  Count7 = 0;
  while(1){
    Count7++;
    WaitForInterrupt();
  }
}
/* ****************************************** */
/*          End of Task7 Section              */
/* ****************************************** */
int main(void){
	vTaskPtr Task_Array[NUMTHREADS] = {&Task2, &Task3, &Task4, &Task5, &Task6, &Task7};
  OS_Init();
 // Profile_Init();  // initialize the 7 hardware profiling pins
  Task0_Init();    // microphone init
  Task1_Init();    // accelerometer init
  BSP_Button1_Init();
  BSP_Button2_Init();
  BSP_RGB_Init(0, 0, 0);
  BSP_Buzzer_Init(0);
  BSP_LCD_Init();
  BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));
//  for(int i=0; i<70;i++){
//   BSP_LCD_DrawBitmap(8,i,title2,110,44);
//    BSP_Delay1ms(20);
//  }
  BSP_LightSensor_Init();
  BSP_TempSensor_Init();
  Time = 0;
  OS_InitSemaphore(&NewData, 0);  // 0 means no data
  OS_InitSemaphore(&LCDmutex, 1); // 1 means free
  OS_InitSemaphore(&I2Cmutex, 1); // 1 means free
  OS_FIFO_Init();                 // initialize FIFO used to send data between Task1 and Task2
  // Task 0 should run every 1ms
  OS_AddPeriodicEventThread(&Task0, 1);
  // Task 1 should run every 100ms
  OS_AddPeriodicEventThread(&Task1, 100);
  // Task2, Task3, Task4, Task5, Task6, Task7 are main threads
 OS_AddThreads(Task_Array);
  // when grading change 1000 to 4-digit number from edX
  TExaS_Init(GRADER, 4027 );          // initialize the Lab 3 grader
//  TExaS_Init(LOGICANALYZER, 1000); // initialize the Lab 3 logic analyzer
  OS_Launch(BSP_Clock_GetFreq()/THREADFREQ); // doesn't return, interrupts enabled in here
  return 0;             // this never executes
}
// Newton's method
// s is an integer
// sqrt(s) is an integer
uint32_t sqrt32(uint32_t s){
uint32_t t;   // t*t will become s
int n;             // loop counter
  t = s/16+1;      // initial guess
  for(n = 16; n; --n){ // will finish
    t = ((t*t+s)/t)/2;
  }
  return t;
}







