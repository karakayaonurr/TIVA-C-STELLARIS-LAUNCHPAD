// Onur Karakaya 140201051 3.sýnýf 1.öðretim ProLab2-3.proje
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "driverlib/gpio.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"

#define D4 GPIO_PIN_0 // Pin 23
#define D5 GPIO_PIN_1 // Pin 24
#define D6 GPIO_PIN_2 // Pin 25
#define D7 GPIO_PIN_3 // Pin 26
#define RS GPIO_PIN_4 // herhangi pin 4
#define EN GPIO_PIN_5 // Pin 6
#define ALLDATAPINS  D7 | D6 | D5 | D4
#define ALLCONTROLPINS RS | EN
#define DATA_PORT_BASE GPIO_PORTD_BASE
#define CMD_PORT_BASE GPIO_PORTE_BASE
#define DATA_PERIPH SYSCTL_PERIPH_GPIOD
#define CMD_PERIPH SYSCTL_PERIPH_GPIOE
//srand(time(NULL));

void ekran_gecis(){ // düþük -> yüksek -> düþük
	GPIOPinWrite(CMD_PORT_BASE, EN, 0);
	GPIOPinWrite(CMD_PORT_BASE, EN, EN);
	GPIOPinWrite(CMD_PORT_BASE, EN, 0);
}

void cmd_ayar() {
	GPIOPinWrite(CMD_PORT_BASE, RS,0);
}

void veri_ayar() {
	GPIOPinWrite(CMD_PORT_BASE, RS,RS);
}

void byte_yolla(char byte, int veri){
	if (veri)
		veri_ayar();
	else
		cmd_ayar();
	SysCtlDelay(400);
	GPIOPinWrite(DATA_PORT_BASE, ALLDATAPINS, byte >>4);
	ekran_gecis();
	GPIOPinWrite(DATA_PORT_BASE, ALLDATAPINS, byte);
	ekran_gecis();
}

void ekran_konumu(char satir, char sutun){
	char konum;
	if (satir == 0)
		konum = 0;
	else if (satir==1)
		konum = 0x40;
	else if (satir==2)
		konum = 0x14;
	else if (satir==3)
		konum = 0x54;
	else
		konum = 0;
	konum |= sutun;
	byte_yolla(0x80 | konum, 0); // 16x2 LCD üzerine imlecin yerini degistirir
}

void ekrani_temizle(void){
	byte_yolla(0x01, 0); // ekrani temizle
	byte_yolla(0x02, 0); // baþa dön
	SysCtlDelay(30000);
}

void imleci_gizle(void) {
	byte_yolla(0x0C, 0); // imleci gizle
}

void sola_kay(){
	byte_yolla(0x18,0); // LCD ekranýný sola kaydirir
}

void saga_kay(){
   byte_yolla(0x1E,0); // LCD ekranýný saga kaydirir
}

// hazir olarak alip düzenledim
void init_port_E(){
   volatile unsigned long tmp;    // bu degisken gecikme yapmak icin gerekli
   SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;   // 1) E portunun osilatörünü etkinleþtir
   tmp = SYSCTL_RCGCGPIO_R;    	  // allow time for clock to start
   GPIO_PORTE_LOCK_R = 0x4C4F434B;// 2) unlock GPIO Port E
   GPIO_PORTE_CR_R = 0x3F;        // allow changes to PE5-0 //PE5-0 deðiþikliklerine izin ver
                                  // only PE0 needs to be unlocked, other bits can't be locked
    			                  // Sadece PE0 kilidinin açýlmasý gerekir, diðer bitler kilitlenemez
   GPIO_PORTE_AMSEL_R = 0x00;     // 3) disable analog on PE //PE'de analog devre dýþý býrak
   GPIO_PORTE_PCTL_R = 0x00000000;// 4) PCTL GPIO on PE4-0
   GPIO_PORTE_DIR_R = 0x3F;       // 5) PE4,PE5 in, PE3-0 out
   GPIO_PORTE_AFSEL_R = 0x00;     // 6) disable alt funct on PE7-0
   GPIO_PORTE_PUR_R = 0x3F;       // enable pull-up on PE5 and PE4
   	   	   	   	   	 	 	 	  //PE4 ve PE5'te pull up'ý etkinleþtir ( BUTON ÝÇÝN)
   GPIO_PORTE_DEN_R = 0x3F;       // 7) enable digital I/O on PE5-0 // portE 5-0 giriþ çýkýþ  etkinlerþtir.
}

// hazir olarak alip düzenledim
void init_port_D() {
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD; // Port D’yi aktiflestir
	delay = SYSCTL_RCGC2_R;  	// zaman gecirmek icin
	GPIO_PORTD_DIR_R |= 0x4F;	// PD 3,2,1,0 pinlerini cikis yap
	GPIO_PORTD_AFSEL_R &= ~0x4F; //GPIO_PORTD_AFSEL_R & = ~0x0F; // PD 3,2,1,0 pinlerini alternatif fonksinunu 0 yap
	GPIO_PORTD_DEN_R |= 0x4F;	// PD 3,2,1,0 pinlerini aktiflestir
}

// hazir olarak alip düzenledim
void initLCD(void){
	SysCtlPeripheralEnable(DATA_PERIPH);
	SysCtlPeripheralEnable(CMD_PERIPH);
	GPIOPinTypeGPIOOutput(DATA_PORT_BASE,  ALLDATAPINS);
	GPIOPinTypeGPIOOutput(CMD_PORT_BASE, ALLCONTROLPINS);
	GPIOPinWrite(DATA_PORT_BASE, ALLDATAPINS ,0);
	GPIOPinWrite(CMD_PORT_BASE, ALLCONTROLPINS ,0);
	SysCtlDelay(10000);
	cmd_ayar();
	SysCtlDelay(15000);
	GPIOPinWrite(DATA_PORT_BASE, ALLDATAPINS, 0b0010);
	ekran_gecis();
	GPIOPinWrite(DATA_PORT_BASE, ALLDATAPINS, 0b0010);
	ekran_gecis();
	byte_yolla(0x28,0);  // 2 satir ayarla
	imleci_gizle();
	byte_yolla(0x06, 0); // ekleme modunu ayarlar
	ekrani_temizle();
}

void ekrana_yaz(char *yazi){
	char *k;
	k = yazi;
	int i=0;
	while ((k != 0) && (*k != 0)){
		byte_yolla(*k, 1);
		k++;
	}
}

int main(){
	init_port_D();
	init_port_E();

	GPIO_PORTD_DATA_R |= 0b0001; // PD0’i 1 yap
	GPIO_PORTD_DATA_R |= 0b0010; // PD1’i 1 yap
	GPIO_PORTD_DATA_R |= 0b0100; // PD2’i 1 yap
	GPIO_PORTD_DATA_R |= 0b1000; // PD3’i 1 yap
	GPIO_PORTE_DATA_R |= 0b010000; // PD4’i 1 yap
	GPIO_PORTE_DATA_R |= 0b100000; // PD5’i 1 yap

    SysCtlClockSet(SYSCTL_SYSDIV_8|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    initLCD();

    int i,j,x,tekrar;


	while(1) {
	    x=rand()%4;
	    tekrar=1;

		if(x==0){ // kirmizi led
			GPIO_PORTE_DATA_R |=0b10; // port aktif edilir

			while(tekrar>0){
				ekran_konumu(0,2);
				ekrana_yaz("KOCAELI UNI");
				SysCtlDelay(40000000/3); // gecikme

				ekran_konumu(1,1);
				ekrana_yaz("ONUR KARAKAYA");
				SysCtlDelay(40000000/3); // gecikme
				ekrani_temizle();

				tekrar--;
			}
			GPIO_PORTE_DATA_R &= ~(0b10); // port deaktive edilir
		}
		else if(x==1){ // beyaz led
			GPIO_PORTE_DATA_R |=0b1000; // port aktif edilir

			while(tekrar>0){
				ekran_konumu(1,1);
				ekrana_yaz("ONUR KARAKAYA");
				SysCtlDelay(40000000/3); // gecikme

				ekran_konumu(0,2);
				ekrana_yaz("KOCAELI UNI");
				SysCtlDelay(40000000/3); // gecikme
				ekrani_temizle();

				tekrar--;
			}
			GPIO_PORTE_DATA_R &= ~(0b1000); // port deaktive edilir
		}
		else if(x==2){ // yesil led
			GPIO_PORTE_DATA_R |=0b1; // port aktif edilir

			while(tekrar>0){
				ekran_konumu(0,22);
				ekrana_yaz("KOCAELI UNI");
				for(i = 0; i < 39; i++){
					saga_kay();
					SysCtlDelay(2500000/3); // gecikme
				}
				ekrani_temizle();
				SysCtlDelay(2500000/3); // gecikme

				for(j = 0; j < 39; j++){
					ekran_konumu(1,20);
					ekrana_yaz("ONUR KARAKAYA");
					saga_kay();
					SysCtlDelay(2500000/3); // gecikme
				}
				ekrani_temizle();

				tekrar--;
			}
			GPIO_PORTE_DATA_R &= ~(0b1); // port deaktive edilir
		}

		else if(x==3){ // mavi led
			GPIO_PORTE_DATA_R |=0b100; // port aktif edilir

			while(tekrar>0){
				ekran_konumu(0,17);
				ekrana_yaz("KOCAELI UNI");
				for(i = 0; i < 39; i++){
					sola_kay();
					SysCtlDelay(2500000/3);// gecikme
				}
				ekrani_temizle();
				SysCtlDelay(2500000/3); // gecikme

				for(j = 0; j < 39; j++){
					ekran_konumu(1,17);
					ekrana_yaz("ONUR KARAKAYA");
					sola_kay();
					SysCtlDelay(2500000/3); // gecikme
				}
				ekrani_temizle();

				tekrar--;
			}
			GPIO_PORTE_DATA_R &= ~(0b100); // port deaktive edilir
		}
		ekrani_temizle();
	}
}
