//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
 * Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
 * Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
 * Con ayuda de: José Guerra
 * Modificaciones y adaptación: Diego Morales
 * IE3027: Electrónica Digital 2 - 2021
 * adaptacion para uso de proyecto 2 de Andy Bonilla y Pablo Herrarte
 */
//***************************************************************************************************************************************

/*-----------------------------------------------------------------------------
 ----------------------------L I B R E R I A S---------------------------------
 -----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"


#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"
//#include "pitches.h"
#include <SPI.h>
#include <SD.h>

/*-----------------------------------------------------------------------------
 -----------------V A R I A B L E S   A   I M P L E M T E N T A R--------------
 -----------------------------------------------------------------------------*/
//-------DIRECTIVA DE PROGRAMA
#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
//-------VARIABLES DE PROGRAMA
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};
/*int melody[]={NOTE_G2, NOTE_C2, NOTE_A2, NOTE_G2, NOTE_G2, 0, NOTE_C2, NOTE_E3}; //notas de la cancioncita
int noteDurations[]={4, 8, 7, 5, 4, 3, 4, 4}; */  
extern uint8_t fondo[];
extern uint8_t arcade[];
bool antirrebote1, antirrebote2;    //variables para antirrebote
bool b1 =1;   //variable booleanaa para J1
bool b2 =1;   //variable booleana para J2
int n = 0;
int nn;
int y = 175;
int x = 50;
int xx = 270;
int yy = 240-175;
int w=x;
int z=y;
int J1=0;
int J2x=270;
int J2y=240-175;
int E = 5;
int l = 0;
int h = 0;
unsigned int m1=2;
unsigned int m2=1;
int serial;
File myFile;
bool iniciado=0;
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button
int buttonPushCounter = 0;   // counter for the number of button presses
/*-----------------------------------------------------------------------------
 ------------ P R O T O T I P O S   D E   F U N C I O N E S -------------------
 -----------------------------------------------------------------------------*/
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);
void printDirectory(File dir, int numTabs); //informacion de directorio
int ascii2hex(int a);                       //funcion de mapeo de texto para imagen
void mapeo_SD(char doc[]);                  //despliegue de imagen mapeada
//--
void inicio(void);
//void musica(void);
void defensiva(void);
/*-----------------------------------------------------------------------------
 --------------------- I N T E R R U P C I O N E S ----------------------------
 -----------------------------------------------------------------------------*/
//de momento no hay interrupciones

/*-----------------------------------------------------------------------------
 ------------------------------ S E T   U P -----------------------------------
 -----------------------------------------------------------------------------*/
 void setup() {
  //-------ENTRADAS Y SALIDA
  pinMode(31, INPUT_PULLUP);    //boton para imagen 1
  /*pinMode(17, INPUT_PULLUP);    //boton para imagen 2
  pinMode(9, INPUT_PULLUP);    //boton para imagen 1
  pinMode(10, INPUT_PULLUP);    //boton para imagen 2*/
  pinMode(PA_3, OUTPUT);    //se define salida del CS para comunicacion con SD
  /*pinMode(PE_2, INPUT_PULLUP);
  pinMode(PE_3, INPUT_PULLUP);
  pinMode(PF_1, INPUT_PULLUP);
  pinMode(PE_5, INPUT_PULLUP);*/
  //-------INICIALIZACION DE PROTOCOLOS DE COMUNICACION
  SPI.setModule(0);         //SPI para SD
  
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);       //UART para menu
  Serial3.begin(9600);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  //-------INICIALIZACION DE TFT
  LCD_Init();
  LCD_Clear(0x00);

    
  //-------MENSAJES DE INICIALIZACION DE COMUNICACION CON SD
  Serial.println("Inicializando tarjeta SD...");
 
  if (!SD.begin(PA_3)) {
    Serial.println("initialización fallida!");  //mensaje si hay algun error
    return;
  }
  Serial.println("initialización correcta.");   //mensaje si todo esta bien

  myFile = SD.open("/");                        //se abre el directorio de la SD
  printDirectory(myFile, 0);                    //se imprime el directorio de la SD

  //-------MENSAJES DE MENU AL INICIAR PROGRAMA
  mapeo_SD("inicio.txt"); 
  //LCD_Print(String text, int x, int y, int fontSize, int color, int background)
  String text1 = "Presiona un boton";                  //texto inicial a desplegar
  LCD_Print(text1, 10, 110, 2, 0x0000, 0xffff);
  delay(1000);
}
/*-----------------------------------------------------------------------------
 -------------------------- M A I N   L O O P ---------------------------------
 -----------------------------------------------------------------------------*/
void loop() {
  //antirrebote1
  b1 = digitalRead(31);         //se toma la lectura del boton 1
  //-------antirrebote1
  if (b1==0 && antirrebote1==0){
    antirrebote1=1;
  }
  else{
    antirrebote1=0;
  } 
  //-------accion luego del antirrebote1
  if (antirrebote1==1 && b1==0){
    antirrebote1=0;
    iniciado=1;
    LCD_Bitmap(0, 0, 320, 240, arcade);
    String text1 = "El primero que choque";                  //texto para las instrucciones del juego
    String text2 = "el camino del otro,";                    //divido en lineas de juego
    String text3 = "sera el ganador :D"; 
    String text4 = "SUERTE :o";
    LCD_Print(text1, 70, 90, 1, 0xffff, 0x0000);
    LCD_Print(text2, 70, 110, 1, 0xffff, 0x0000);
    LCD_Print(text3, 70, 130, 1, 0xffff, 0x0000);
    LCD_Print(text4, 70, 160, 2, 0xffff, 0x0000);
    delay(1000);
    inicio();                                                 //funcion con semafo para inicio de juego
    delay(1000);
    //LCD_Bitmap(0, 0, 320, 240, arcade);                       //se despliega el fondo del arcade donde se jugara  
    LCD_Clear(0x00);
  }
  //musica();
    while(iniciado){
      int H[3200];
      int V[2400];
      w=x;
      z=y;
      xx=J2x;
      yy=J2y;
      defensiva();
      if (E==5){
        Serial3.write(1);
        for(x; x <320; x++){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2x=J2x-1;
          LCD_Bitmap(J2x,yy,32,24,tron2);
          if (x-w>16){
            V_line( x -1, y+12, 2, 0x421b  );
            V_line( J2x+33, yy+12, 2, 0xfc00  );
            H[n] = x-1;
            V[n] = y+12;
            n++;
            H[n] = J2x+33;
            V[n] = yy+12;
            n++;
            E=0;
            nn=0;
          }
            for(nn; nn<n; nn++){
              if (((H[nn]==x+32)&&(V[nn]<y+24)&&(V[nn]>y))||(x+32>320)){
                while(true){
                  
                }
              }
              else if (((V[nn]>yy)&&(V[nn]<yy+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2x<0)){
              while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=2){
            E=4;
            J1=1;
            break;
          }
          else if (m2!=1){
            E=3;
            J1=2;
            break;
          }
        } 
      }
      
  
      else if ((m1==2)&&(m2==1)){
        for(x; x <320; x++){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2x=J2x-1;
          LCD_Bitmap(J2x,yy,32,24,tron2);
          if (x-w<17){
            if (J1==1){
              V_line(J2x+33,J2y+12,2,0xfc00);
              H[n] = J2x+33;
              V[n] = J2y+12;
              n++;
            }
            else if (J1==2){
              V_line(x-1,y+12,2,0x421b);
              H[n] = x-1;
              V[n] = y+12;
              n++;
            }
          }
          else if (x-w>16){
            V_line( x -1, y+12, 2, 0x421b  );
            V_line( J2x+33, yy+12, 2, 0xfc00  );
            H[n] = x-1;
            V[n] = y+12;
            n++;
            H[n] = J2x+33;
            V[n] = yy+12;
            n++;
            if (J1==1){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l+12, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l+12, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l+12;
                n++;
                }
                E=0;
              }
            }
            J1=0;
          }
            nn=0;
            for(nn; nn<n; nn++){
              if (((H[nn]==x+32)&&(V[nn]<y+24)&&(V[nn]>y))||(x+32>320)){
                while(true){
                  
                }
              }
              else if (((V[nn]>yy)&&(V[nn]<yy+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2x<0)){
              while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=2){
            E=4;
            J1=1;
            break;
          }
          else if (m2!=1){
            E=3;
            J1=2;
            break;
          }
        } 
      }
      
      
      else if ((m1==1)&&(m2==1)){
        for(x; x <320-32; x--){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2x=J2x-1;
          LCD_Bitmap(J2x,yy,32,24,tron2);
          if (w-x<17){
            if (J1==1){
              V_line(J2x+33,yy+12,2,0xfc00);
              H[n] = J2x+33;
              V[n] = yy+12;
              n++;
            }
            else if (J1==2){
              V_line(x+33,y+12,2,0x421b);
              H[n] = x+33;
              V[n] = y+12;
              n++;
            }
          }
          else if (w-x>16){
            V_line( x +33, y+12, 2, 0x421b  );
            V_line(J2x+33, yy+12, 2, 0xfc00);
            H[n] = x+33;
            V[n] = y+12;
            n++;
            H[n] = J2x+33;
            V[n] = yy+12;
            n++;
            if (J1==1){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l+12, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l+12, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l+12;
                n++;
                }
                E=0;
              }
            }
            J1=0;
          }
          nn=0;
          for(nn; nn<n; nn++){
              if (((H[nn]==(x))&&(V[nn]<y+24)&&(V[nn]>y))||(x<0)){
                while(true){
                  Serial3.write(49);
                }
              }
              else if (((V[nn]>yy)&&(V[nn]<yy+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2x<0)){
              while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=1){
            E=3;
            J1=1;
            break;
          }
          else if (m2!=1){
            E=3;
            J1=2;
            break;
          }
        } 
      }
      
      
      else if ((m1==4)&&(m2==1)){
        for(y; y <240; y++){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2x=J2x-1;
          LCD_Bitmap(J2x,yy,32,24,tron2);
          if (y-z<13){
            if (J1==1){
              V_line(J2x+33,yy+12,2,0xfc00);
              H[n] = J2x+33;
              V[n] = yy+12;
              n++;
            }
            else if (J1==2){
              H_line(x+16,y-1,2,0x421b);
              H[n] = x+16;
              V[n] = y-1;
              n++;
            }
          }
          else if (y-z>12){
            H_line( x+16, y-1, 2, 0x421b  );
            V_line(J2x+33, yy+12, 2, 0xfc00);
            H[n] = x+16;
            V[n] = y-1;
            n++;
            H[n] = J2x+33;
            V[n] = yy+12;
            n++;
            if (J1==1){
              if (E==3){
                h=0;
                for(h; h<17; h++){
                V_line( w+h+16, z+12, 2, 0x421b);
                H[n] = w+h+16;
                V[n] = z+12;
                n++;
                }
                E=0;
              }
              else if (E==4){
                h=0;
                for(h; h<16; h++){
                V_line( w+h, z+12, 2, 0x421b);
                H[n] = w+h;
                V[n] = z+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l+12, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l+12;
                n++;
                }
                E=0;
              }
            }
            J1=0;
          }
          nn = 0;
          for(nn; nn<n; nn++){
            if(((V[nn]<y+24)&&(V[nn]>y)&&(H[nn]>x)&&(H[nn]<x+32))||(y+24>240)){
              while(true){
                
              }
            }
            else if (((V[nn]>yy)&&(V[nn]<yy+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2x<0)){
              while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=4){
            E=1;
            J1=1;
            break;
          }
          else if (m2!=1){
            E=3;
            J1=2;
            break;
          }
        } 
      }

      
      else if ((m1==3)&&(m2==1)){
        for(y; y <240; y--){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2x=J2x-1;
          LCD_Bitmap(J2x,yy,32,24,tron2);
          if (z-y<13){
            if (J1==1){
              V_line(J2x+33,yy+12,2,0xfc00);
              H[n] = J2x+33;
              V[n] = yy+12;
              n++;
            }
            else if (J1==2){
              H_line(x+16,y+25,2,0x421b);
              H[n] = x+16;
              V[n] = y+25;
              n++;
            }
          }
          else if (z-y>12){
            H_line( x+16, y+25, 2, 0x421b  );
            V_line(J2x+33,yy+12,2,0xfc00);
            H[n]=x+16;
            V[n]=y+25;
            n++;
            H[n] = J2x+33;
            V[n] = yy+12;
            n++;
            if (J1==1){
              if (E==3){
                h=0;
                for(h; h<17; h++){
                V_line( w+h+16, z+12, 2, 0x421b);
                H[n]=w+h+16;
                V[n]=z+12;
                n++;
                }
                E=0;
              }
              else if (E==4){
                h=0;
                for(h; h<16; h++){
                V_line( w+h, z+12, 2, 0x421b);
                H[n]=w+h;
                V[n]=z+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l+12, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l+12;
                n++;
                }
                E=0;
              }
            }
            J1=0;
          }
          nn=0;
          for(nn; nn<n; nn++){
            if(((V[nn]>y)&&(V[nn]<y+24)&&(H[nn]>x)&&(H[nn]<x+32))||(y<0)){
              while(true){
                
              }
            }
            else if (((V[nn]>yy)&&(V[nn]<yy+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2x<0)){
              while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=3){
            E=2;
            J1=1;
            break;
          }
          else if (m2!=1){
            E=3;
            J1=2;
            break;
          }
        } 
      }

      //---------------------------------------Jugador 2 hacia la derecha------------------------------------------
      else if ((m1==2)&&(m2==2)){
        for(x; x <320; x++){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2x=J2x+1;
          LCD_Bitmap(J2x,J2y,32,24,tron2);
          if (x-w<17){
            if (J1==1){
              V_line(J2x-1,yy+12,2,0xfc00);
              H[n] = J2x-1;
              V[n] = yy+12;
              n++;
            }
            else if (J1==2){
              V_line(x-1,y+12,2,0x421b);
              H[n] = x-1;
              V[n] = y+12;
              n++;
            }
          }
          else if (x-w>16){
            V_line( x -1, y+12, 2, 0x421b  );
            H[n] = x-1;
            V[n] = y+12;
            n++;
            V_line( J2x -1, yy+12, 2, 0xfc00  );
            H[n] = J2x-1;
            V[n] = yy+12;
            n++;
            if (J1==1){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l+12, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l+12, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l+12;
                n++;
                }
                E=0;
              }
            }
            J1=0;
          }
            nn=0;
            for(nn; nn<n; nn++){
              if (((H[nn]==x+32)&&(V[nn]<y+24)&&(V[nn]>y))||(x+32>320)){
                while(true){
                  
                }
              }
              else if (((V[nn]>yy)&&(V[nn]<yy+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2x>320-32)){
                while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=2){
            E=4;
            J1=1;
            break;
          }
          else if (m2!=2){
            E=4;
            J1=2;
            break;
          }
        } 
      }


      else if ((m1==1)&&(m2==2)){
        for(x; x <320-32; x--){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2x=J2x+1;
          LCD_Bitmap(J2x,J2y,32,24,tron2);
          if (w-x<17){
            if (J1==1){
              V_line(J2x-1,yy+12,2,0xfc00);
              H[n] = J2x-1;
              V[n] = yy+12;
              n++;
            }
            else if (J1==2){
              V_line(x+33,y+12,2,0x421b);
              H[n] = x+33;
              V[n] = y+12;
              n++;
            }
          }
          else if (w-x>16){
            V_line( x +33, y+12, 2, 0x421b  );
            H[n] = x+33;
            V[n] = y+12;
            n++;
            V_line( J2x -1, yy+12, 2, 0xfc00  );
            H[n] = J2x-1;
            V[n] = yy+12;
            n++;
            if (J1==1){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l+12, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l+12;
                n++;
                }
              E=0;
              }
            }
            else if (J1==2){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l+12, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l+12;
                n++;
                }
                E=0;
              }
            }
            J1=0;
          }
          nn=0;
          for(nn; nn<n; nn++){
              if (((H[nn]==(x))&&(V[nn]<y+24)&&(V[nn]>y))||(x<0)){
                while(true){
                  
                }
              }
              else if (((V[nn]>yy)&&(V[nn]<yy+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2x>320-32)){
                while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=1){
            E=3;
            J1=1;
            break;
          }
          else if (m2!=2){
            E=4;
            J1=2;
            break;
          }
        } 
      }

      
      else if ((m1==4)&&(m2==2)){
        for(y; y <240; y++){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2x=J2x+1;
          LCD_Bitmap(J2x,J2y,32,24,tron2);
          if (y-z<12){
            if (J1==1){
              V_line(J2x-1,yy+12,2,0xfc00);
              H[n] = J2x-1;
              V[n] = yy+12;
              n++;
            }
            else if (J1==2){
              H_line(x+16,y-1,2,0x421b);
              H[n] = x+16;
              V[n] = y-1;
              n++;
            }
          }
          else if (y-z>12){
            H_line( x+16, y-1, 2, 0x421b  );
            H[n] = x+16;
            V[n] = y-1;
            n++;
            V_line( J2x -1, yy+12, 2, 0xfc00  );
            H[n] = J2x-1;
            V[n] = yy+12;
            n++;
            if (J1==1){
              if (E==3){
                h=0;
                for(h; h<17; h++){
                V_line( w+h+16, z+12, 2, 0x421b);
                H[n] = w+h+16;
                V[n] = z+12;
                n++;
                }
                E=0;
              }
              else if (E==4){
                h=0;
                for(h; h<16; h++){
                V_line( w+h, z+12, 2, 0x421b);
                H[n] = w+h;
                V[n] = z+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l+12, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l+12;
                n++;
                }
                E=0;
              }
            }
            J1=0;
          }
          nn = 0;
          for(nn; nn<n; nn++){
            if(((V[nn]<y+24)&&(V[nn]>y)&&(H[nn]>x)&&(H[nn]<x+32))||(y+24>240)){
              while(true){
                
              }
            }
            else if (((V[nn]>yy)&&(V[nn]<yy+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2x>320-32)){
                while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=4){
            E=1;
            J1=1;
            break;
          }
          else if (m2!=2){
            E=4;
            J1=2;
            break;
          }
        } 
      }

      
      else if ((m1==3)&&(m2==2)){
        for(y; y <240; y--){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2x=J2x+1;
          LCD_Bitmap(J2x,J2y,32,24,tron2);
          if (z-y<13){
            if (J1==1){
              V_line(J2x-1,yy+12,2,0xfc00);
              H[n] = J2x-1;
              V[n] = yy+12;
              n++;
            }
            else if (J1==2){
              H_line(x+16,y+25,2,0x421b);
              H[n] = x+16;
              V[n] = y+25;
              n++;
            }
          }
          else if (z-y>12){
            H_line( x+16, y+25, 2, 0x421b  );
            H[n]=x+16;
            V[n]=y+25;
            n++;
            V_line( J2x -1, yy+12, 2, 0xfc00  );
            H[n] = J2x-1;
            V[n] = yy+12;
            n++;
            if (J1==1){
              if (E==3){
                h=0;
                for(h; h<17; h++){
                V_line( w+h+16, z+12, 2, 0x421b);
                H[n]=w+h+16;
                V[n]=z+12;
                n++;
                }
                E=0;
              }
              else if (E==4){
                h=0;
                for(h; h<16; h++){
                V_line( w+h, z+12, 2, 0x421b);
                H[n]=w+h;
                V[n]=z+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( xx+16, yy+l+12, 2, 0xfc00);
                H[n] = xx+16;
                V[n] = yy+l+12;
                n++;
                }
                E=0;
              }
            }
            J1=0;
          }
          nn=0;
          for(nn; nn<n; nn++){
            if(((V[nn]>y)&&(V[nn]<y+24)&&(H[nn]>x)&&(H[nn]<x+32))||(y<0)){
              while(true){
                
              }
            }
            else if (((V[nn]>yy)&&(V[nn]<yy+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2x>320-32)){
                while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=3){
            E=2;
            J1=1;
            break;
          }
          else if (m2!=2){
            E=4;
            J1=2;
            break;
          }
        } 
      }

      //----------------------------------------------Jugador 2 para abajo-------------------------------------------------
      else if ((m1==2)&&(m2==4)){
        for(x; x <320; x++){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2y=J2y+1;
          LCD_Bitmap(J2x,J2y,32,24,tron2);
          if (x-w<17){
            if (J1==1){
              H_line(J2x+16,J2y-1,2,0xfc00);
              H[n] = J2x+16;
              V[n] = J2y-1;
              n++;
            }
            else if (J1==2){
              V_line(x-1,y+12,2,0x421b);
              H[n] = x-1;
              V[n] = y+12;
              n++;
            }
          }
          else if (x-w>16){
            V_line( x -1, y+12, 2, 0x421b  );
            H[n] = x-1;
            V[n] = y+12;
            n++;
            H_line( J2x+16, J2y-1, 2, 0xfc00  );
            H[n] = J2x+16;
            V[n] = J2y-1;
            n++;
            if (J1==1){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l+12, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==3){
              h=0;
              for(h; h<17; h++){
              V_line( xx+h+16, yy+12, 2, 0xfc00);
              H[n] = xx+h+16;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            else if (E==4){
              h=0;
              for(h; h<16; h++){
              V_line( xx+h, yy+12, 2, 0xfc00);
              H[n] = xx+h;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            }
            J1=0;
          }
            nn=0;
            for(nn; nn<n; nn++){
              if (((H[nn]==x+32)&&(V[nn]<y+24)&&(V[nn]>y))||(x+32>320)){
                while(true){
                  
                }
              }
              else if (((V[nn]>J2y)&&(V[nn]<J2y+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2y>240)){
                while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=2){
            E=4;
            J1=1;
            break;
          }
          else if (m2!=4){
            E=1;
            J1=2;
            break;
          }
        } 
      }

      
      else if ((m1==1)&&(m2==4)){
        for(x; x <320-32; x--){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2y=J2y+1;
          LCD_Bitmap(J2x,J2y,32,24,tron2);
          if (w-x<17){
            if (J1==1){
              H_line(J2x+16,J2y-1,2,0xfc00);
              H[n] = J2x+16;
              V[n] = J2y-1;
              n++;
            }
            else if (J1==2){
              V_line(x+16,y-1,2,0x421b);
              H[n] = x+16;
              V[n] = y-1;
              n++;
            }
          }
          else if (w-x>16){
            V_line( x +33, y+12, 2, 0x421b  );
            H[n] = x+33;
            V[n] = y+12;
            n++;
            H_line( J2x+16, J2y-1, 2, 0xfc00  );
            H[n] = J2x+16;
            V[n] = J2y-1;
            n++;
            if (J1==1){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l+12, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l+12;
                n++;
                }
                E=0;
                }
            }
            else if (J1==2){
              if (E==3){
              h=0;
              for(h; h<17; h++){
              V_line( xx+h+16, yy+12, 2, 0xfc00);
              H[n] = xx+h+16;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            else if (E==4){
              h=0;
              for(h; h<16; h++){
              V_line( xx+h, yy+12, 2, 0xfc00);
              H[n] = xx+h;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            }
            J1=0;
          }
          nn=0;
          for(nn; nn<n; nn++){
              if (((H[nn]==(x))&&(V[nn]<y+24)&&(V[nn]>y))||(x<0)){
                while(true){
                  
                }
              }
              else if (((V[nn]>J2y)&&(V[nn]<J2y+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2y>240)){
                while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=1){
            E=3;
            J1=1;
            break;
          }
          else if (m2!=4){
            E=1;
            J1=2;
            break;
          }
        } 
      }

      
      else if ((m1==4)&&(m2==4)){
        for(y; y <240; y++){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2y=J2y+1;
          LCD_Bitmap(J2x,J2y,32,24,tron2);
          if (y-z<13){
            if (J1==1){
              H_line(J2x+16,J2y-1,2,0xfc00);
              H[n] = J2x+16;
              V[n] = J2y-1;
              n++;
            }
            else if (J1==2){
              H_line(x+16,y-1,2,0x421b);
              H[n] = x+16;
              V[n] = y-1;
              n++;
            }
          }
          else if (y-z>12){
            H_line( x+16, y-1, 2, 0x421b  );
            H[n] = x+16;
            V[n] = y-1;
            n++;
            H_line( J2x+16, J2y-1, 2, 0xfc00  );
            H[n] = J2x+16;
            V[n] = J2y-1;
            n++;
            if (J1==1){
              if (E==3){
                h=0;
                for(h; h<17; h++){
                V_line( w+h+16, z+12, 2, 0x421b);
                H[n] = w+h+16;
                V[n] = z+12;
                n++;
                }
                E=0;
              }
              else if (E==4){
                h=0;
                for(h; h<16; h++){
                V_line( w+h, z+12, 2, 0x421b);
                H[n] = w+h;
                V[n] = z+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==3){
              h=0;
              for(h; h<17; h++){
              V_line( xx+h+16, yy+12, 2, 0xfc00);
              H[n] = xx+h+16;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            else if (E==4){
              h=0;
              for(h; h<16; h++){
              V_line( xx+h, yy+12, 2, 0xfc00);
              H[n] = xx+h;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            }
            J1=0;
          }
          nn = 0;
          for(nn; nn<n; nn++){
            if(((V[nn]<y+24)&&(V[nn]>y)&&(H[nn]>x)&&(H[nn]<x+32))||(y+24>240)){
              while(true){
                
              }
            }
            else if (((V[nn]>J2y)&&(V[nn]<J2y+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2y>240)){
                while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=4){
            E=1;
            J1=1;
            break;
          }
          else if (m2!=4){
            E=1;
            J1=2;
            break;
          }
        } 
      }

      
      else if ((m1==3)&&(m2==4)){
        for(y; y <240; y--){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2y=J2y+1;
          LCD_Bitmap(J2x,J2y,32,24,tron2);
          if (z-y<13){
            if (J1==1){
              H_line(J2x+16,J2y-1,2,0xfc00);
              H[n] = J2x+16;
              V[n] = J2y-1;
              n++;
            }
            else if (J1==2){
              H_line(x+16,y+25,2,0x421b);
              H[n] = x+16;
              V[n] = y+25;
              n++;
            }
          }
          else if (z-y>12){
            H_line( x+16, y+25, 2, 0x421b  );
            H[n]=x+16;
            V[n]=y+25;
            n++;
            H_line( J2x+16, J2y-1, 2, 0xfc00  );
            H[n] = J2x+16;
            V[n] = J2y-1;
            n++;
            if (J1==1){
              if (E==3){
                h=0;
                for(h; h<17; h++){
                V_line( w+h+16, z+12, 2, 0x421b);
                H[n]=w+h+16;
                V[n]=z+12;
                n++;
                }
                E=0;
              }
              else if (E==4){
                h=0;
                for(h; h<16; h++){
                V_line( w+h, z+12, 2, 0x421b);
                H[n]=w+h;
                V[n]=z+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==3){
              h=0;
              for(h; h<17; h++){
              V_line( xx+h+16, yy+12, 2, 0xfc00);
              H[n] = xx+h+16;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            else if (E==4){
              h=0;
              for(h; h<16; h++){
              V_line( xx+h, yy+12, 2, 0xfc00);
              H[n] = xx+h;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            }
            J1=0;
          }
          nn=0;
          for(nn; nn<n; nn++){
            if(((V[nn]>y)&&(V[nn]<y+24)&&(H[nn]>x)&&(H[nn]<x+32))||(y<0)){
              while(true){
                
              }
            }
            else if (((V[nn]>J2y)&&(V[nn]<J2y+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2y>240-24)){
                while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=3){
            E=2;
            J1=1;
            break;
          }
          else if (m2!=4){
            E=1;
            J1=2;
            break;
          }
        } 
      }

      //-----------------------------------------Cuando Jugador 2 va hacia arriba---------------------------------------
      else if ((m1==2)&&(m2==3)){
        for(x; x <320; x++){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2y=J2y-1;
          LCD_Bitmap(J2x, J2y, 32,24,tron2);
          if (x-w<17){
            if (J1==1){
              H_line( J2x+16, J2y+25, 2, 0xfc00  );
              H[n]=J2x+16;
              V[n]=J2y+25;
              n++;
            }
            else if (J1==2){
              V_line( x-1, y+12, 2, 0x421b  );
              H[n]=x-1;
              V[n]=y+12;
              n++;
            }
          }
          else if (x-w>16){
            V_line( x -1, y+12, 2, 0x421b  );
            H[n] = x-1;
            V[n] = y+12;
            n++;
            H_line( J2x+16, J2y+25, 2, 0xfc00  );
            H[n]=J2x+16;
            V[n]=J2y+25;
            n++;
            if (J1==1){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l+12, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==3){
              h=0;
              for(h; h<17; h++){
              V_line( xx+h+16, yy+12, 2, 0xfc00);
              H[n] = xx+h+16;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            else if (E==4){
              h=0;
              for(h; h<16; h++){
              V_line( xx+h, yy+12, 2, 0xfc00);
              H[n] = xx+h;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            }
            J1=0;
          }
            nn=0;
            for(nn; nn<n; nn++){
              if (((H[nn]==x+32)&&(V[nn]<y+24)&&(V[nn]>y))||(x+32>320)){
                while(true){
                  
                }
              }
              else if (((V[nn]>J2y)&&(V[nn]<J2y+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2y<0)){
                while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=2){
            E=4;
            J1=1;
            break;
          }
          else if (m2!=3){
            E=2;
            J1=2;
            break;
          }
        } 
      }

      
      else if ((m1==1)&&(m2==3)){
        for(x; x <320-32; x--){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2y=J2y-1;
          LCD_Bitmap(J2x, J2y, 32,24,tron2);
          if (w-x<17){
            if (J1==1){
              H_line( J2x+16, J2y+25, 2, 0xfc00  );
              H[n]=J2x+16;
              V[n]=J2y+25;
              n++;
            }
            else if (J1==2){
              V_line( x+33, y+12, 2, 0x421b  );
              H[n]=x+33;
              V[n]=y+12;
              n++;
            }
          }
          else if (w-x>16){
            V_line( x +33, y+12, 2, 0x421b  );
            H[n] = x+33;
            V[n] = y+12;
            n++;
            H_line( J2x+16, J2y+25, 2, 0xfc00  );
            H[n]=J2x+16;
            V[n]=J2y+25;
            n++;
            if (J1==1){
              if (E==1){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l;
                n++;
                }
                E=0;
              }
              else if (E==2){
                l=0;
                for(l; l<12; l++){
                H_line( w+16, z+l+12, 2, 0x421b);
                H[n] = w+16;
                V[n] = z+l+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==3){
              h=0;
              for(h; h<17; h++){
              V_line( xx+h+16, yy+12, 2, 0xfc00);
              H[n] = xx+h+16;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            else if (E==4){
              h=0;
              for(h; h<16; h++){
              V_line( xx+h, yy+12, 2, 0xfc00);
              H[n] = xx+h;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            }
            J1=0;
          }
          nn=0;
          for(nn; nn<n; nn++){
              if (((H[nn]==(x))&&(V[nn]<y+24)&&(V[nn]>y))||(x<0)){
                while(true){
                  
                }
              }
              else if (((V[nn]>J2y)&&(V[nn]<J2y+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2y<0)){
                while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=1){
            E=3;
            J1=1;
            break;
          }
          else if (m2!=3){
            E=2;
            J1=2;
            break;
          }
        } 
      }

      
      else if ((m1==4)&&(m2==3)){
        for(y; y <240; y++){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2y=J2y-1;
          LCD_Bitmap(J2x, J2y, 32,24,tron2);
          if (y-z<13){
            if (J1==1){
              H_line( J2x+16, J2y+25, 2, 0xfc00  );
              H[n]=J2x+16;
              V[n]=J2y+25;
              n++;
            }
            else if (J1==2){
              H_line( x+16, y-1, 2, 0x421b  );
              H[n]=x+16;
              V[n]=y-1;
              n++;
            }
          }
          else if (y-z>12){
            H_line( x+16, y-1, 2, 0x421b  );
            H[n] = x+16;
            V[n] = y-1;
            n++;
            H_line( J2x+16, J2y+25, 2, 0xfc00  );
            H[n]=J2x+16;
            V[n]=J2y+25;
            n++;
            if (J1==1){
              if (E==3){
                h=0;
                for(h; h<17; h++){
                V_line( w+h+16, z+12, 2, 0x421b);
                H[n] = w+h+16;
                V[n] = z+12;
                n++;
                }
                E=0;
              }
              else if (E==4){
                h=0;
                for(h; h<16; h++){
                V_line( w+h, z+12, 2, 0x421b);
                H[n] = w+h;
                V[n] = z+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==3){
              h=0;
              for(h; h<17; h++){
              V_line( xx+h+16, yy+12, 2, 0xfc00);
              H[n] = xx+h+16;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            else if (E==4){
              h=0;
              for(h; h<16; h++){
              V_line( xx+h, yy+12, 2, 0xfc00);
              H[n] = xx+h;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            }
            J1=0;
          }
          nn = 0;
          for(nn; nn<n; nn++){
            if(((V[nn]<y+24)&&(V[nn]>y)&&(H[nn]>x)&&(H[nn]<x+32))||(y+24>240)){
              while(true){
                
              }
            }
            else if (((V[nn]>J2y)&&(V[nn]<J2y+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2y<0)){
                while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=4){
            E=1;
            J1=1;
            break;
          }
          else if (m2!=3){
            E=2;
            J1=2;
            break;
          }
        } 
      }

      
      else if ((m1==3)&&(m2==3)){
        for(y; y <240; y--){
          delay(10);
          LCD_Bitmap(x,y,32,24,tron);
          J2y=J2y-1;
          LCD_Bitmap(J2x, J2y, 32,24,tron2);
          if (z-y<13){
            if (J1==1){
              H_line( J2x+16, J2y+25, 2, 0xfc00  );
              H[n]=J2x+16;
              V[n]=J2y+25;
              n++;
            }
            else if (J1==2){
              H_line( x+16, y+25, 2, 0x421b  );
              H[n]=x+16;
              V[n]=y+25;
              n++;
            }
          }
          else if (z-y>12){
            H_line( x+16, y+25, 2, 0x421b  );
            H[n]=x+16;
            V[n]=y+25;
            n++;
            H_line( J2x+16, J2y+25, 2, 0xfc00  );
            H[n]=J2x+16;
            V[n]=J2y+25;
            n++;
            if (J1==1){
              if (E==3){
                h=0;
                for(h; h<17; h++){
                V_line( w+h+16, z+12, 2, 0x421b);
                H[n]=w+h+16;
                V[n]=z+12;
                n++;
                }
                E=0;
              }
              else if (E==4){
                h=0;
                for(h; h<16; h++){
                V_line( w+h, z+12, 2, 0x421b);
                H[n]=w+h;
                V[n]=z+12;
                n++;
                }
                E=0;
              }
            }
            else if (J1==2){
              if (E==3){
              h=0;
              for(h; h<17; h++){
              V_line( xx+h+16, yy+12, 2, 0xfc00);
              H[n] = xx+h+16;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            else if (E==4){
              h=0;
              for(h; h<16; h++){
              V_line( xx+h, yy+12, 2, 0xfc00);
              H[n] = xx+h;
              V[n] = yy+12;
              n++;
              }
              E=0;
            }
            }
            J1=0;
          }
          nn=0;
          for(nn; nn<n; nn++){
            if(((V[nn]>y)&&(V[nn]<y+24)&&(H[nn]>x)&&(H[nn]<x+32))||(y<0)){
              while(true){
                
              }
            }
            else if (((V[nn]>J2y)&&(V[nn]<J2y+24)&&(H[nn]>J2x)&&(H[nn]<J2x+32))||(J2y<0)){
                while(true){
                
              }
            }
          }
          defensiva();
          if (m1!=3){
            E=2;
            J1=1;
            break;
          }
          else if (m2!=3){
            E=2;
            J1=2;
            break;
          }
        } 
      }
      /*else if (digitalRead(PUSH1)==LOW){
        for(x; x <320-32; x=x+5){
          Serial.println("CAMINA RÁPIDO");
          delay(10);
          int mario_index = (x/11)%8;
    
          //LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);
          int bowser_index = (x/11)%4;
          //-------Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
          //void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[])
          LCD_Bitmap(x,175,32,24,tron);
          V_line( x -1, 185, 2, 0x421b  );
          V_line( x -2, 185, 2, 0x421b  );
          V_line( x -3, 185, 2, 0x421b  );
          V_line( x -4, 185, 2, 0x421b  );
          V_line( x -5, 185, 2, 0x421b  );
          if (digitalRead(31)==HIGH){
            break;
          }
        }
      }*/
    }
  }
  //-------control de cual imagen se pone
  //antirrebote1
  /*buttonState = digitalRead(17);         //se toma la lectura del boton 2
  if (buttonState != lastButtonState) {
    if (buttonState == 1) {
      wenas++;
      Serial.println(wenas);
    } 
    else {
      Serial.println("wenas");
    }
  }
  delay(1);
  lastButtonState = buttonState;
  //-------antirrebote2
  /*if (b2==0 && antirrebote2==0){
    antirrebote2=1;
  }
  else{
    antirrebote2=0;
  } 
  //-------accion luego del antirrebote1
  if (antirrebote2==1 && b2==0){
    wenas++;
    Serial.println(wenas);
  }*/


  /*if (wenas==15){
    LCD_Clear(0x00);
    mapeo_SD("yourock.txt");                       //imagen alma
    String text1 = "ganaste perro";         //pequeña descripcion
    LCD_Print(text1, 60, 185, 2, 0x0000, 0xffff);  //caracteristicas de texto
  }
  
  
}*/



/*-----------------------------------------------------------------------------
 ------------------------- F U N C I O N E S ----------------------------------
 -----------------------------------------------------------------------------*/
//-------FUNCION PARA CUENTA REGRESIVA
void inicio(void){
  LCD_Clear(0x00);
  String text1 = "Comienza en 3";                  //texto inicial a desplegar
  LCD_Print(text1, 50, 110, 2, 0x0000, 0xffff);
  delay(1000);
  LCD_Clear(0x00);
  String text2 = "Comienza en 2";                  //texto inicial a desplegar
  LCD_Print(text2, 50, 110, 2, 0x0000, 0xffff);
  delay(1000);
  LCD_Clear(0x00);
  String text3 = "Comienza en 1";                  //texto inicial a desplegar
  LCD_Print(text3, 50, 110, 2, 0x0000, 0xffff);
  delay(1000);
  LCD_Clear(0x00);
}


 
//-------Función para inicializar LCD
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  //-------Secuencia de Inicialización
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
//  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}

//-------Función para enviar comandos a la LCD - parámetro (comando)
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}

//-------Función para enviar datos a la LCD - parámetro (dato)
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}

//-------Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}

//-------Función para borrar la pantalla - parámetros (color)
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 

//-------Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}

//-------Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}

//-------Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}

//-------Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+w;
  y2 = y+h;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = w*h*2-1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c); 
      k = k - 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}

//-------Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) 
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}

//-------Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      //LCD_DATA(bitmap[k]);    
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}

//-------Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  if(flip){
    for (int j = 0; j < height; j++){
        k = (j*(ancho) + index*width -1 - offset)*2;
        k = k+width*2;
       for (int i = 0; i < width; i++){
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k+1]);
        k = k - 2;
       } 
    }
  }
  else{
     for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k + 2;
     } 
    }
  }
  digitalWrite(LCD_CS, HIGH);
}

//-------FUNCION PARA MOSTRAR INFORMACION DE DIRECTORIO
void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println(" ");

    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size());
    }
    entry.close();
  }
}
//-------FUNCION PARA MAPEAR LOS VALORES HEX DEL BITMAP A DECIMALES
int ascii2hex(int a) {
  switch (a) {
    case (48):      //caso 0
      return 0;
    case (49):      //caso 1
      return 1;
    case (50):      //caso 2
      return 2;
    case (51):      //caso 3
      return 3;
    case (52):      //caso 4
      return 4;
    case (53):      //caso 5
      return 5;
    case (54):      //caso 6
      return 6;
    case (55):      //caso 7
      return 7;
    case (56):      //caso 8
      return 8;
    case (57):      //caso 9
      return 9;
    case (97):      //caso A
      return 10;
    case (98):      //caso B
      return 11;
    case (99):      //caso C
      return 12;
    case (100):     //caso D
      return 13;
    case (101):     //caso E
      return 14;
    case (102):     //caso F
      return 15;
  }
}
//-------FUNCION PARA MOSTRAR LAS IMAGENES DESDE SD
void mapeo_SD(char doc[]) {
  myFile = SD.open(doc, FILE_READ);   //se toma el archivo de la imagen 
  int hex1 = 0;                       //declaracion de variable 1 para valor hex
  int val1 = 0;                       
  int val2 = 0;
  int mapear = 0;
  int vertical = 0;
  unsigned char maps[640];            //se crea arreglo vacio para almacenar el mapeo

  if (myFile) {
    while (myFile.available() ) {     //se leen datos mientras este disponible
      mapear = 0;
      while (mapear < 640) {          //se limita el rango
        hex1 = myFile.read();         //se lee el archivo con la imagen
        if (hex1 == 120) {
          val1 = myFile.read();       //se lee el primer valor hexadecimal del bitmap
          val2 = myFile.read();       //se lee el segundo valor hexadecimal del bitmap
          val1 = ascii2hex(val1);     //se mapea el primer valor hexadecimal 
          val2 = ascii2hex(val2);     //se mapea el segundo valor hexadecimal 
          maps[mapear] = val1 * 16 + val2;  //se colona en el arreglo nuevo
          mapear++;                         //se cambia de posicion
        }
      }
      LCD_Bitmap(0, vertical, 320, 1, maps);
      vertical++;
    }
    myFile.close();
  }
  else {
    Serial.println("No se pudo abrir la imagen, prueba nuevamente");
    myFile.close();
  }
}

/*void musica(void){
  for (int thisNote=0; thisNote <8; thisNote++){
    int noteDuration = f00b0 / noteDurations [thisNote]; //se define la duracion aproximada de cada nota
    tone(PF_2, melody [thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;        //se alarga ligeramente el tiempo de duracion
    delay(pauseBetweenNotes);                           //se para por un tiempo
    noTone(8);
    delay(10);
  }
}

void defensiva(void){
  if ((digitalRead(PUSH1)==HIGH || digitalRead(PA_7)==HIGH || digitalRead(PA_6)==HIGH) && digitalRead(PUSH2)==LOW){
    m1=2;
  }
  else if (digitalRead(PUSH1)==LOW && (digitalRead(PUSH2)==HIGH || digitalRead(PA_7)==HIGH || digitalRead(PA_6)==HIGH)){
    m1=1;
  }
  else if (digitalRead(PA_7)==LOW && (digitalRead(PUSH2)==HIGH || digitalRead(PUSH1)==HIGH || digitalRead(PA_6)==HIGH)){
    m1=3;
  }
  else if ((digitalRead(PUSH2)==HIGH || digitalRead(PUSH1)==HIGH || digitalRead(PA_7)==HIGH) && digitalRead(PA_6)==LOW){
    m1=4;
  }
  if ((digitalRead(PE_2)==HIGH || digitalRead(PE_3)==HIGH || digitalRead(PF_1)==HIGH) && digitalRead(PE_5)==LOW){
    m2=1;
  }
  else if ((digitalRead(PE_2)==HIGH || digitalRead(PE_3)==HIGH || digitalRead(PE_5)==HIGH) && digitalRead(PF_1)==LOW){
    m2=2;
  }
  else if ((digitalRead(PE_5)==HIGH || digitalRead(PE_3)==HIGH || digitalRead(PF_1)==HIGH) && digitalRead(PE_2)==LOW){
    m2=3;
  }
  else if ((digitalRead(PE_2)==HIGH || digitalRead(PE_5)==HIGH || digitalRead(PF_1)==HIGH) && digitalRead(PE_3)==LOW){
    m2=4;
  }
}

*/

/*void defensiva(void){
  if (Serial3.available()){
    serial=Serial3.read();
    if(52<serial<57){
      m1 = serial-52;
      Serial.println(m1);
    }
    if(48<serial<53){
      m2 = serial-48;
      Serial.println(m2);
    }
  }
}*/
void defensiva(void){
  if (Serial3.available()){
    char readData[3];
    Serial3.readBytesUntil(13, readData, 3);
    m1=readData[0];
    m2=readData[1];
    Serial.println(m1);
    Serial.println(m2);
    m1 = m1-52;
    m2 = m2-48;
    Serial.println(m1);
    Serial.println(m2);
  }
}
