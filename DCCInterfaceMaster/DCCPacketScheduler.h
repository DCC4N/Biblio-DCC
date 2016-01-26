/*
* DCC Waveform Generator
*
* modified by Philipp Gahtow
* Copyright 2015 digitalmoba@arcor.de, http://pgahtow.de
*
*/

#ifndef __DCCCOMMANDSTATION_H__
#define __DCCCOMMANDSTATION_H__
#include "DCCPacket.h"
#include "DCCPacketQueue.h"

/*******************************************************************/
//Protokoll can handel max 16384 switch (Weichenzust�nde max 16384):
#if defined(__AVR_ATmega1284P__) 
//more then 8 KB RAM
#define AccessoryMax 2048	//max DCC 2048 Weichen / 8 = 255 byte 
#define SlotMax 128			//Slots f�r Lokdaten
#define PERIODIC_REFRESH_QUEUE_SIZE 110
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) 
//8 KB RAM
#define AccessoryMax 1024	//max DCC 2048 Weichen / 8 = 255 byte 
#define SlotMax 64			//Slots f�r Lokdaten
#define PERIODIC_REFRESH_QUEUE_SIZE 90
#elif defined (__AVR_ATmega644P__)	
//4 KB RAM
#define AccessoryMax 512	//normal 512 Weichen / 8 = 64 byte
#define SlotMax 36			//Slots f�r Lokdaten
#define PERIODIC_REFRESH_QUEUE_SIZE 60
#else	
//less then 2,5 KB RAM
#define AccessoryMax 128		//64 Weichen / 8 = 8 byte
#define SlotMax 15			//Slots f�r Lokdaten
#define PERIODIC_REFRESH_QUEUE_SIZE 50
#endif
/*******************************************************************/

#define E_STOP_QUEUE_SIZE           10	//old 2
//#define HIGH_PRIORITY_QUEUE_SIZE    10		//30
//#define LOW_PRIORITY_QUEUE_SIZE     10		//90
#define REPEAT_QUEUE_SIZE           20

//How often a packet is repeat:
#define ONCE_REFRESH_INTERVAL	2	//send estop, switch, pom each "second" packet
//#define LOW_PRIORITY_INTERVAL     5
//#define REPEAT_INTERVAL           11
//#define PERIODIC_REFRESH_INTERVAL 23

//Repaet for Packetkinds:
#define SPEED_REPEAT      3
#define FUNCTION_REPEAT   3
#define E_STOP_REPEAT     5
#define OPS_MODE_PROGRAMMING_REPEAT 3
#define OTHER_REPEAT      3

//State of Railpower:
#define OFF	false		//no power on the rails
#define ON	true		//signal on the rails
#define ESTOP 0x03		//no Loco drive but rails have power

//Trnt message paket format (inc)
#define ROCO 0
#define IB 4

typedef struct	//Lokdaten	(Lok Events)
{
	uint16_t adr;		// SS1, SS0, A13, A12| A11, A10, A9, A8| A7, A6, A5, A4| A3, A2, A1, A0
	// A0-A13 = Adresse
	// SS = Fahrstufen-speedsteps (0=14, 1=27, 2=28, 3=128) 
	uint8_t speed;	//X, Speed 0..127 (0x00 - 0x7F) -> 0SSS SSSS
	uint8_t f0;		//X   X   Dir F0 | F4  F3  F2  F1			
	uint8_t f1;		//F12 F11 F10 F9 | F8  F7  F6  F5	
	uint8_t f2;		//F20 F19 F18 F17| F16 F15 F14 F13
	uint8_t f3;		//F28 F27 F26 F25| F24 F23 F22 F21 
} NetLok;

class DCCPacketScheduler
{
  public:
  
    DCCPacketScheduler(void);
    
    //for configuration
    //void setDefaultSpeedSteps(uint8_t new_speed_steps);
	void setup(uint8_t pin = 6, uint8_t format = ROCO); 	//for any post-constructor initialization

	//more specific functions
    void setpower(bool state);		//set Mode of output aktive/inactive
    byte getpower(void);		//get the Mode of output power
	void eStop(void); //Broadcast Stop Packet For All Decoders
	// bool eStop(uint16_t address); //just one specific loco -> use setSpeed with speed = 1
    
    //for enqueueing packets
    //bool setSpeed(uint16_t address, uint8_t new_speed, uint8_t steps = 0); //new_speed: [-127,127]
    bool setSpeed14(uint16_t address, uint8_t speed); //new_speed: [-13,13]
    bool setSpeed28(uint16_t address, uint8_t speed); //new_speed: [-28,28]
    bool setSpeed128(uint16_t address, uint8_t speed); //new_speed: [-127,127]

	void getLocoStateFull(uint16_t adr, bool bc);	//aktuellen Zustand aller Funktionen und Speed der Lok
	byte getLocoDir(uint16_t adr); //Gibt aktuelle Fahrtrichtung der angefragen Lok zur�ck
	byte getLocoSpeed(uint16_t adr);	//Gibt aktuelle Geschwindigkeit der angefragten Lok zur�ck
    
    //the function methods are NOT stateful; you must specify all functions each time you call one
    //keeping track of function state is the responsibility of the calling program.
    //bool setFunctions(uint16_t address, uint8_t address_kind, uint8_t F0to4, uint8_t F5to9=0x00, uint8_t F9to12=0x00);
    //bool setFunctions(uint16_t address, uint8_t address_kind, uint16_t functions);
	void setLocoFunc(uint16_t address, uint8_t type, uint8_t fkt);
    bool setFunctions0to4(uint16_t address, uint8_t functions);	//- F0 F4 F3 F2 F1
    bool setFunctions5to8(uint16_t address, uint8_t functions);	//- F8 F7 F6 F5
    bool setFunctions9to12(uint16_t address, uint8_t functions);	//- F12 F11 F10 F9
	bool setFunctions13to20(uint16_t address, uint8_t functions);	//F20 F19 F18 F17 F16 F15 F14 F13
	bool setFunctions21to28(uint16_t address, uint8_t functions);	//F28 F27 F26 F25 F24 F23 F22 F21

	byte getFunktion0to4(uint16_t address);	//gibt Funktionszustand - F0 F4 F3 F2 F1 zur�ck
	byte getFunktion5to8(uint16_t address);	//gibt Funktionszustand - F8 F7 F6 F5 zur�ck
	byte getFunktion9to12(uint16_t address);	//gibt Funktionszustand - F12 F11 F10 F9 zur�ck
	byte getFunktion13to20(uint16_t address);	//gibt Funktionszustand F20 - F13 zur�ck
	byte getFunktion21to28(uint16_t address);	//gibt Funktionszustand F28 - F21 zur�ck
    
    bool setBasicAccessoryPos(uint16_t address, bool state);
	bool setBasicAccessoryPos(uint16_t address, bool state, bool activ);
	bool getBasicAccessoryInfo(uint16_t address);
	
	bool opsProgDirectCV(uint16_t CV, uint8_t CV_data);		//using Direct Mode - Write byte
    bool opsProgramCV(uint16_t address, uint16_t CV, uint8_t CV_data);
	bool opsDecoderReset(void);		//Decoder Reset Packet For all Decoders

    //to be called periodically within loop()
    void update(void); //checks queues, puts whatever's pending on the rails via global current_packet. easy-peasy

  private:
	  
	uint8_t TrntFormat;		// The Addressing of BasicAccessory Messages
	byte railpower;				 // actual state of the power that goes to the rails

	byte BasicAccessory[AccessoryMax / 8];	//Speicher f�r Weichenzust�nde
	NetLok LokDataUpdate[SlotMax];	//Speicher zu widerholdene Lok Daten
	byte LokStsgetSlot(uint16_t adr);		//gibt Slot f�r Adresse zur�ck / erzeugt neuen Slot (0..126)
	void LokStsSetNew(byte Slot, uint16_t adr);	//Neue Lok eintragen mit Adresse

	byte slotFullNext;	//if no free slot, override existing slots

	//bool LokStsIsEmpty(byte Slot);	//pr�ft ob Datenpacket/Slot leer ist?
	//uint16_t LokStsgetAdr(byte Slot);			//gibt Lokadresse des Slot zur�ck, wenn 0x0000 dann keine Lok vorhanden
	//byte getNextSlot(byte Slot);	//gibt n�chsten genutzten Slot
	//void setFree(uint16_t adr);
  
  //  void stashAddress(DCCPacket *p); //remember the address to compare with the next packet
   // void repeatPacket(DCCPacket *p); //insert into the appropriate repeat queue
    //uint8_t default_speed_steps;
    uint16_t last_packet_address;
  
    uint8_t packet_counter;	//to not repeat only one queue
    
    DCCEmergencyQueue e_stop_queue;
//    DCCPacketQueue high_priority_queue;
//    DCCPacketQueue low_priority_queue;
    DCCRepeatQueue repeat_queue;
//NEW
	DCCTemporalQueue periodic_refresh_queue;
    
    //TODO to be completed later.
    //DCC_Packet ops_programming_queue[10];
    
    //some handy thingers
    //DCCPacket idle_packet;
};

//DCCPacketScheduler packet_scheduler;

#if defined (__cplusplus)
extern "C" {
#endif

	extern void notifyLokAll(uint16_t Adr, uint8_t Steps, uint8_t Speed, uint8_t F0, uint8_t F1, uint8_t F2, uint8_t F3, bool bc) __attribute__((weak));
	extern void notifyTrnt(uint16_t Adr, bool State) __attribute__((weak));

#if defined (__cplusplus)
}
#endif

#endif //__DCC_COMMANDSTATION_H__
