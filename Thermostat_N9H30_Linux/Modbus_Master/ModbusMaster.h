

/**
@file
Arduino library for communicating with Modbus slaves over RS232/485 (via RTU protocol).

@defgroup setup ModbusMaster Object Instantiation/Initialization
@defgroup buffer ModbusMaster Buffer Management
@defgroup discrete Modbus Function Codes for Discrete Coils/Inputs
@defgroup register Modbus Function Codes for Holding/Input Registers
@defgroup constant Modbus Function Codes, Exception Codes
*/
/*

  ModbusMaster.h - Arduino library for communicating with Modbus slaves
  over RS232/485 (via RTU protocol).

  Library:: ModbusMaster

  Copyright:: 2009-2016 Doc Walker

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

*/

  
#ifndef ModbusMaster_h
#define ModbusMaster_h

#include "def.h"
#include "stdio.h"
/**
@def __MODBUSMASTER_DEBUG__ (0)
Set to 1 to enable debugging features within class:
  - PIN A cycles for each byte read in the Modbus response
  - PIN B cycles for each millisecond timeout during the Modbus response
*/

/* _____UTILITY MACROS_______________________________________________________ */


/* _____PROJECT INCLUDES_____________________________________________________ */
// functions to calculate Modbus Application Data Unit CRC
#include "crc16.h"

// functions to manipulate words
#include "word.h"


/* _____CLASS DEFINITIONS____________________________________________________ */
/**
Arduino class library for communicating with Modbus slaves over 
RS232/485 (via RTU protocol).
*/


    void begin(uint8_t);

     // Modbus exception codes
    /**
    Modbus protocol illegal function exception.
    
    The function code received in the query is not an allowable action for
    the server (or slave). This may be because the function code is only
    applicable to newer devices, and was not implemented in the unit
    selected. It could also indicate that the server (or slave) is in the
    wrong state to process a request of this type, for example because it is
    unconfigured and is being asked to return register values.
    
    @ingroup constant
    */
    static const uint8_t ku8MBIllegalFunction            = 0x01;

    /**
    Modbus protocol illegal data address exception.
    
    The data address received in the query is not an allowable address for 
    the server (or slave). More specifically, the combination of reference 
    number and transfer length is invalid. For a controller with 100 
    registers, the ADU addresses the first register as 0, and the last one 
    as 99. If a request is submitted with a starting register address of 96 
    and a quantity of registers of 4, then this request will successfully 
    operate (address-wise at least) on registers 96, 97, 98, 99. If a 
    request is submitted with a starting register address of 96 and a 
    quantity of registers of 5, then this request will fail with Exception 
    Code 0x02 "Illegal Data Address" since it attempts to operate on 
    registers 96, 97, 98, 99 and 100, and there is no register with address 
    100. 
    
    @ingroup constant
    */
    static const uint8_t ku8MBIllegalDataAddress         = 0x02;
    
    /**
    Modbus protocol illegal data value exception.
    
    A value contained in the query data field is not an allowable value for 
    server (or slave). This indicates a fault in the structure of the 
    remainder of a complex request, such as that the implied length is 
    incorrect. It specifically does NOT mean that a data item submitted for 
    storage in a register has a value outside the expectation of the 
    application program, since the MODBUS protocol is unaware of the 
    significance of any particular value of any particular register.
    
    @ingroup constant
    */
    static const uint8_t ku8MBIllegalDataValue           = 0x03;
    
    /**
    Modbus protocol slave device failure exception.
    
    An unrecoverable error occurred while the server (or slave) was
    attempting to perform the requested action.
    
    @ingroup constant
    */
    static const uint8_t ku8MBSlaveDeviceFailure         = 0x04;

    // Class-defined success/exception codes
    /**
    ModbusMaster success.
    
    Modbus transaction was successful; the following checks were valid:
      - slave ID
      - function code
      - response code
      - data
      - CRC
      
    @ingroup constant
    */
    static const uint8_t ku8MBSuccess                    = 0x00;
    
    /**
    ModbusMaster invalid response slave ID exception.
    
    The slave ID in the response does not match that of the request.
    
    @ingroup constant
    */
    static const uint8_t ku8MBInvalidSlaveID             = 0xE0;
    
    /**
    ModbusMaster invalid response function exception.
    
    The function code in the response does not match that of the request.
    
    @ingroup constant
    */
    static const uint8_t ku8MBInvalidFunction            = 0xE1;
    
    /**
    ModbusMaster response timed out exception.
    
    The entire response was not received within the timeout period, 
    ModbusMaster::ku8MBResponseTimeout. 
    
    @ingroup constant
    */
    static const uint8_t ku8MBResponseTimedOut           = 0xE2;
    
    /**
    ModbusMaster invalid response CRC exception.
    
    The CRC in the response does not match the one calculated.
    
    @ingroup constant
    */
    static const uint8_t ku8MBInvalidCRC   = 0xE3;
    
    uint16_t getResponseBuffer(uint8_t);
    void     clearResponseBuffer(void);
    uint8_t  setTransmitBuffer(uint8_t, uint16_t);
    void     clearTransmitBuffer(void);
    
    void beginTransmission(uint16_t);
    uint8_t requestFrom(uint16_t, uint16_t);
    void sendBit(int);
    void send8(uint8_t);
    void send16(uint16_t);
    void send32(uint32_t);
    uint8_t available(void);
    uint16_t receive(void);
    
    
    uint8_t  readCoils(uint16_t, uint16_t);
    uint8_t  readDiscreteInputs(uint16_t, uint16_t);
    uint8_t  readHoldingRegisters(uint16_t, uint16_t);
    uint8_t  readInputRegisters(uint16_t, uint8_t);
    uint8_t  writeSingleCoil(uint16_t, uint8_t);
    uint8_t  writeSingleRegister(uint16_t, uint16_t);
    uint8_t  writeMultipleCoils(uint16_t, uint16_t);
    uint8_t  writeMultipleCoils_0(void);
    uint8_t  writeMultipleRegisters(uint16_t, uint16_t);
    uint8_t  writeMultipleRegisters_0(void);
    uint8_t  maskWriteRegister(uint16_t, uint16_t, uint16_t);
    uint8_t  readWriteMultipleRegisters(uint16_t, uint16_t, uint16_t, uint16_t);
    uint8_t  readWriteMultipleRegisters_0(uint16_t, uint16_t);
//PZEM-003
		uint8_t calibrateRegister(uint16_t u16ReadAddress, uint8_t u16ReadQty);
		uint8_t resetenergyRegister(uint16_t u16ReadAddress, uint8_t u16ReadQty);
    
    uint8_t  _u8MBSlave;                                         ///< Modbus slave (1..255) initialized in begin()
    static const uint8_t ku8MaxBufferSize                = 64;   ///< size of response/transmit buffers    
    uint16_t _u16ReadAddress;                                    ///< slave register from which to read
    uint16_t _u16ReadQty;                                        ///< quantity of words to read
    uint16_t _u16ResponseBuffer[64]; //ku8MaxBufferSize];               ///< buffer to store Modbus 
    uint16_t _u16WriteAddress;                                   ///< slave register to which to write
    uint16_t _u16WriteQty;                                       ///< quantity of words to write
    uint16_t _u16TransmitBuffer[64]; //ku8MaxBufferSize];               ///< buffer containing data to 
    uint16_t* txBuffer; // from Wire.h -- need to clean this up Rx
    uint8_t _u8TransmitBufferIndex;
    uint16_t u16TransmitBufferLength;
    uint16_t* rxBuffer; // from Wire.h -- need to clean this up Rx
    uint8_t _u8ResponseBufferIndex;
    uint8_t _u8ResponseBufferLength;
 #if 1  
    // Modbus function codes for bit access
    static const uint8_t ku8MBReadCoils                  = 0x01; ///< Modbus function 0x01 Read 
    static const uint8_t ku8MBReadDiscreteInputs         = 0x02; ///< Modbus function 0x02 Read 
    static const uint8_t ku8MBWriteSingleCoil            = 0x05; ///< Modbus function 0x05 
    static const uint8_t ku8MBWriteMultipleCoils         = 0x0F; ///< Modbus function 0x0F 

    // Modbus function codes for 16 bit access
    static const uint8_t ku8MBReadHoldingRegisters       = 0x03; ///< Modbus function 0x03 Read 
    static const uint8_t ku8MBReadInputRegisters         = 0x04; ///< Modbus function 0x04 Read 
    static const uint8_t ku8MBWriteSingleRegister        = 0x06; ///< Modbus function 0x06 
    static const uint8_t ku8MBWriteMultipleRegisters     = 0x10; ///< Modbus function 0x10 
    static const uint8_t ku8MBMaskWriteRegister          = 0x16; ///< Modbus function 0x16 Mask 
    static const uint8_t ku8MBReadWriteMultipleRegisters = 0x17; ///< Modbus function 0x17 Read 
    
		// PZEM-003 function codes
    static const uint8_t ku8MBResetEnergyRegister	 = 0x42; // Reset energy register
    static const uint8_t ku8MBCalibrationRegister	 = 0x41; // Calibration
    // Modbus timeout [milliseconds]
    static const uint16_t ku16MBResponseTimeout          = 1000; //Modbus timeout [milliseconds]
#else
    // Modbus function codes for bit access
    static uint8_t ku8MBReadCoils                  = 0x01; ///< Modbus function 0x01 Read 
    static uint8_t ku8MBReadDiscreteInputs         = 0x02; ///< Modbus function 0x02 Read 
    static uint8_t ku8MBWriteSingleCoil            = 0x05; ///< Modbus function 0x05 
    static uint8_t ku8MBWriteMultipleCoils         = 0x0F; ///< Modbus function 0x0F 

    // Modbus function codes for 16 bit access
    static uint8_t ku8MBReadHoldingRegisters       = 0x03; ///< Modbus function 0x03 Read 
    static uint8_t ku8MBReadInputRegisters         = 0x04; ///< Modbus function 0x04 Read 
    static uint8_t ku8MBWriteSingleRegister        = 0x06; ///< Modbus function 0x06 
    static uint8_t ku8MBWriteMultipleRegisters     = 0x10; ///< Modbus function 0x10 
    static uint8_t ku8MBMaskWriteRegister          = 0x16; ///< Modbus function 0x16 Mask 
    static uint8_t ku8MBReadWriteMultipleRegisters = 0x17; ///< Modbus function 0x17 Read 
    
		// PZEM-003 function codes
    static uint8_t ku8MBResetEnergyRegister	 = 0x42; // Reset energy register
    static uint8_t ku8MBCalibrationRegister	 = 0x41; // Calibration
    // Modbus timeout [milliseconds]
    static uint16_t ku16MBResponseTimeout          = 1000; //Modbus timeout [milliseconds]
#endif    
    // master function that conducts Modbus transactions
    uint8_t ModbusMasterTransaction(uint8_t u8MBFunction);
    
#endif

/**
@example examples/Basic/Basic.pde
@example examples/PhoenixContact_nanoLC/PhoenixContact_nanoLC.pde
@example examples/RS485_HalfDuplex/RS485_HalfDuplex.ino
*/
