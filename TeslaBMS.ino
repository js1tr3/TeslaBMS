#if defined (__arm__) && defined (__SAM3X8E__)
#include <chip.h>
#endif

#include <Arduino.h>
#include "Logger.h"
#include "SerialConsole.h"
#include "BMSModuleManager.h"
//#include "SystemIO.h"
#include <mcp_can.h>
//#include <due_wire.h>
//#include <Wire_EEPROM.h>

//#define BMS_BAUD  612500
#define BMS_BAUD  617647
//#define BMS_BAUD  608695

BMSModuleManager bms;
EEPROMSettings settings;
SerialConsole console;
uint32_t lastUpdate;

// //This code only applicable to Due to fixup lack of functionality in the arduino core.
// #if defined (__arm__) && defined (__SAM3X8E__)
// void serialSpecialInit(Usart *pUsart, uint32_t baudRate)
// {
//   // Reset and disable receiver and transmitter
//   pUsart->US_CR = UART_CR_RSTRX | UART_CR_RSTTX | UART_CR_RXDIS | UART_CR_TXDIS;

//   // Configure mode
//   pUsart->US_MR =  US_MR_CHRL_8_BIT | US_MR_NBSTOP_1_BIT | UART_MR_PAR_NO | US_MR_USART_MODE_NORMAL | 
//                    US_MR_USCLKS_MCK | US_MR_CHMODE_NORMAL | US_MR_OVER; // | US_MR_INVDATA;

//   //Get the integer divisor that can provide the baud rate
//   int divisor = SystemCoreClock / baudRate;
//   int error1 = abs(baudRate - (SystemCoreClock / divisor)); //find out how close that is to the real baud
//   int error2 = abs(baudRate - (SystemCoreClock / (divisor + 1))); //see if bumping up one on the divisor makes it a better match

//   if (error2 < error1) divisor++;   //If bumping by one yielded a closer rate then use that instead

//   // Configure baudrate including the optional fractional divisor possible on USART
//   pUsart->US_BRGR = (divisor >> 3) | ((divisor & 7) << 16);

//   // Enable receiver and transmitter
//   pUsart->US_CR = UART_CR_RXEN | UART_CR_TXEN;
// }
// #endif

void loadSettings()
{
  //  EEPROM.read(EEPROM_PAGE, settings);

  //  if (settings.version != EEPROM_VERSION) //if settings are not the current version then erase them and set defaults
   // {
        Logger::console("Resetting to factory defaults");
        settings.version = EEPROM_VERSION;
        settings.checksum = 0;
        settings.canSpeed = 500000;
        settings.batteryID = 0x01; //in the future should be 0xFF to force it to ask for an address
        settings.OverVSetpoint = 4.1f;
        settings.UnderVSetpoint = 2.3f;
        settings.OverTSetpoint = 65.0f;
        settings.UnderTSetpoint = -10.0f;
        settings.balanceVoltage = 3.9f;
        settings.balanceHyst = 0.04f;
        settings.logLevel = 2;
    //    EEPROM.write(EEPROM_PAGE, settings);
    //}
    //else {
    //    Logger::console("Using stored values from EEPROM");
  //  }
        
    Logger::setLoglevel((Logger::LogLevel)settings.logLevel);
}

// void initializeCAN()
// {
//     uint32_t id;
//     Can0.begin(settings.canSpeed);
//     if (settings.batteryID < 0xF)
//     {
//         //Setup filter for direct access to our registered battery ID
//         id = (0xBAul << 20) + (((uint32_t)settings.batteryID & 0xF) << 16);
//         Can0.setRXFilter(0, id, 0x1FFF0000ul, true);
//         //Setup filter for request for all batteries to give summary data
//         id = (0xBAul << 20) + (0xFul << 16);
//         Can0.setRXFilter(1, id, 0x1FFF0000ul, true);
//     }
// }

void setup() 
{
    delay(4000);  //just for easy debugging. It takes a few seconds for USB to come up properly on most OS's
    SERIALCONSOLE.begin(115200);
    SERIALCONSOLE.println("Starting up!");
    SERIAL.begin(BMS_BAUD);
// #if defined (__arm__) && defined (__SAM3X8E__)
//     serialSpecialInit(USART0, BMS_BAUD); //required for Due based boards as the stock core files don't support 612500 baud.
// #endif

    SERIALCONSOLE.println("Started serial interface to BMS.");

    pinMode(13, INPUT);

   // loadSettings();
    //initializeCAN();

   // systemIO.setup();

    bms.renumberBoardIDs();

    //Logger::setLoglevel(Logger::Debug);

    lastUpdate = 0;

    bms.clearFaults();
}

void loop() 
{
   // CAN_FRAME incoming;

    console.loop();

    if (millis() > (lastUpdate + 1000))
    {    
        lastUpdate = millis();
        bms.balanceCells();
        bms.getAllVoltTemp();
    }

    // if (Can0.available()) {
    //     Can0.read(incoming);
    //     bms.processCANMsg(incoming);
    // }
}
