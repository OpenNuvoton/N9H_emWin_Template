/**
@file
Arduino library for communicating with Modbus slaves over RS232/485 (via RTU protocol).
*/
/*

  ModbusMaster.cpp - Arduino library for communicating with Modbus slaves
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


/* _____PROJECT INCLUDES_____________________________________________________ */
#include "N9H20.h"
#include "ModbusMaster.h"


/* _____GLOBAL VARIABLES_____________________________________________________ */
extern volatile int     OS_TimeMS;
extern UARTDEV_T* pUartDevISR;

/* _____PUBLIC FUNCTIONS_____________________________________________________ */
void RS485_TX_Data(UARTDEV_T* pUartDev, int length, UINT8 *pData);
int RS485_RX(void);

void    ModbusDisplayMessage(uint8_t *u8ModbusADU, uint8_t u8ModbusADUSize);
void DispMsg_PZEM003(uint8_t *pu8Cur, uint8_t number);
void DispMsg_Temperature(uint8_t *pu8Cur, uint8_t number);
short c_temp, f_temp;
/**
Initialize class object.

Assigns the Modbus slave ID and serial port.
Call once class has been instantiated, typically within setup().

@param slave Modbus slave ID (1..255)
@param &serial reference to serial port object (Serial, Serial1, ... Serial3)
@ingroup setup
*/

void begin(uint8_t slave)
{
    _u8MBSlave = slave;
    _u8TransmitBufferIndex = 0;
    u16TransmitBufferLength = 0;

}


void beginTransmission(uint16_t u16Address)
{
    _u16WriteAddress = u16Address;
    _u8TransmitBufferIndex = 0;
    u16TransmitBufferLength = 0;
}

// eliminate this function in favor of using existing MB request functions
uint8_t requestFrom(uint16_t address, uint16_t quantity)
{
    uint8_t read;
    // clamp to buffer length
    if (quantity > ku8MaxBufferSize)
    {
        quantity = ku8MaxBufferSize;
    }
    // set rx buffer iterator vars
    _u8ResponseBufferIndex = 0;
    _u8ResponseBufferLength = read;

    return read;
}


void sendBit(int data)
{
    uint8_t txBitIndex = u16TransmitBufferLength % 16;
    if ((u16TransmitBufferLength >> 4) < ku8MaxBufferSize)
    {
        if (0 == txBitIndex)
        {
            _u16TransmitBuffer[_u8TransmitBufferIndex] = 0;
        }
//    bitWrite(_u16TransmitBuffer[_u8TransmitBufferIndex], txBitIndex, data);  // Ray ????
        u16TransmitBufferLength++;
        _u8TransmitBufferIndex = u16TransmitBufferLength >> 4;
    }
}


void send16(uint16_t data)
{
    if (_u8TransmitBufferIndex < ku8MaxBufferSize)
    {
        _u16TransmitBuffer[_u8TransmitBufferIndex++] = data;
        u16TransmitBufferLength = _u8TransmitBufferIndex << 4;
    }
}


void send32(uint32_t data)
{
    send16(lowWord(data));
    send16(highWord(data));
}


void send8(uint8_t data)
{
    send32( (uint32_t)(data));
}


uint8_t available(void)
{
    return _u8ResponseBufferLength - _u8ResponseBufferIndex;
}


uint16_t receive(void)
{
    if (_u8ResponseBufferIndex < _u8ResponseBufferLength)
    {
        return _u16ResponseBuffer[_u8ResponseBufferIndex++];
    }
    else
    {
        return 0xFFFF;
    }
}


/**
Retrieve data from response buffer.

@see ModbusMaster::clearResponseBuffer()
@param u8Index index of response buffer array (0x00..0x3F)
@return value in position u8Index of response buffer (0x0000..0xFFFF)
@ingroup buffer
*/
uint16_t getResponseBuffer(uint8_t u8Index)
{
    if (u8Index < ku8MaxBufferSize)
    {
        return _u16ResponseBuffer[u8Index];
    }
    else
    {
        return 0xFFFF;
    }
}


/**
Clear Modbus response buffer.

@see ModbusMaster::getResponseBuffer(uint8_t u8Index)
@ingroup buffer
*/
void clearResponseBuffer()
{
    uint8_t i;

    for (i = 0; i < ku8MaxBufferSize; i++)
    {
        _u16ResponseBuffer[i] = 0;
    }
}


/**
Place data in transmit buffer.

@see ModbusMaster::clearTransmitBuffer()
@param u8Index index of transmit buffer array (0x00..0x3F)
@param u16Value value to place in position u8Index of transmit buffer (0x0000..0xFFFF)
@return 0 on success; exception number on failure
@ingroup buffer
*/
uint8_t setTransmitBuffer(uint8_t u8Index, uint16_t u16Value)
{
    if (u8Index < ku8MaxBufferSize)
    {
        _u16TransmitBuffer[u8Index] = u16Value;
        return ku8MBSuccess;
    }
    else
    {
        return ku8MBIllegalDataAddress;
    }
}


/**
Clear Modbus transmit buffer.

@see ModbusMaster::setTransmitBuffer(uint8_t u8Index, uint16_t u16Value)
@ingroup buffer
*/
void clearTransmitBuffer()
{
    uint8_t i;

    for (i = 0; i < ku8MaxBufferSize; i++)
    {
        _u16TransmitBuffer[i] = 0;
    }
}


/**
Modbus function 0x01 Read Coils.

This function code is used to read from 1 to 2000 contiguous status of
coils in a remote device. The request specifies the starting address,
i.e. the address of the first coil specified, and the number of coils.
Coils are addressed starting at zero.

The coils in the response buffer are packed as one coil per bit of the
data field. Status is indicated as 1=ON and 0=OFF. The LSB of the first
data word contains the output addressed in the query. The other coils
follow toward the high order end of this word and from low order to high
order in subsequent words.

If the returned quantity is not a multiple of sixteen, the remaining
bits in the final data word will be padded with zeros (toward the high
order end of the word).

@param u16ReadAddress address of first coil (0x0000..0xFFFF)
@param u16BitQty quantity of coils to read (1..2000, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup discrete
*/
uint8_t readCoils(uint16_t u16ReadAddress, uint16_t u16BitQty)
{
    _u16ReadAddress = u16ReadAddress;
    _u16ReadQty = u16BitQty;
    return ModbusMasterTransaction(ku8MBReadCoils);
}


/**
Modbus function 0x02 Read Discrete Inputs.

This function code is used to read from 1 to 2000 contiguous status of
discrete inputs in a remote device. The request specifies the starting
address, i.e. the address of the first input specified, and the number
of inputs. Discrete inputs are addressed starting at zero.

The discrete inputs in the response buffer are packed as one input per
bit of the data field. Status is indicated as 1=ON; 0=OFF. The LSB of
the first data word contains the input addressed in the query. The other
inputs follow toward the high order end of this word, and from low order
to high order in subsequent words.

If the returned quantity is not a multiple of sixteen, the remaining
bits in the final data word will be padded with zeros (toward the high
order end of the word).

@param u16ReadAddress address of first discrete input (0x0000..0xFFFF)
@param u16BitQty quantity of discrete inputs to read (1..2000, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup discrete
*/
uint8_t readDiscreteInputs(uint16_t u16ReadAddress, uint16_t u16BitQty)
{
    _u16ReadAddress = u16ReadAddress;
    _u16ReadQty = u16BitQty;
    return ModbusMasterTransaction(ku8MBReadDiscreteInputs);
}


/**
Modbus function 0x03 Read Holding Registers.

This function code is used to read the contents of a contiguous block of
holding registers in a remote device. The request specifies the starting
register address and the number of registers. Registers are addressed
starting at zero.

The register data in the response buffer is packed as one word per
register.

@param u16ReadAddress address of the first holding register (0x0000..0xFFFF)
@param u16ReadQty quantity of holding registers to read (1..125, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t readHoldingRegisters(uint16_t u16ReadAddress, uint16_t u16ReadQty)
{
    _u16ReadAddress = u16ReadAddress;
    _u16ReadQty = u16ReadQty;
    return ModbusMasterTransaction(ku8MBReadHoldingRegisters);
}


/**
Modbus function 0x04 Read Input Registers.

This function code is used to read from 1 to 125 contiguous input
registers in a remote device. The request specifies the starting
register address and the number of registers. Registers are addressed
starting at zero.

The register data in the response buffer is packed as one word per
register.

@param u16ReadAddress address of the first input register (0x0000..0xFFFF)
@param u16ReadQty quantity of input registers to read (1..125, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t readInputRegisters(uint16_t u16ReadAddress, uint8_t u16ReadQty)
{
    _u16ReadAddress = u16ReadAddress;
    _u16ReadQty = u16ReadQty;
    return ModbusMasterTransaction(ku8MBReadInputRegisters);
}


/**
Modbus function 0x05 Write Single Coil.

This function code is used to write a single output to either ON or OFF
in a remote device. The requested ON/OFF state is specified by a
constant in the state field. A non-zero value requests the output to be
ON and a value of 0 requests it to be OFF. The request specifies the
address of the coil to be forced. Coils are addressed starting at zero.

@param u16WriteAddress address of the coil (0x0000..0xFFFF)
@param u8State 0=OFF, non-zero=ON (0x00..0xFF)
@return 0 on success; exception number on failure
@ingroup discrete
*/


uint8_t writeSingleCoil(uint16_t u16WriteAddress, uint8_t u8State)
{
    _u16WriteAddress = u16WriteAddress;
    _u16WriteQty = (u8State ? 0xFF00 : 0x0000);
    return ModbusMasterTransaction(ku8MBWriteSingleCoil);
}


/**
Modbus function 0x06 Write Single Register.

This function code is used to write a single holding register in a
remote device. The request specifies the address of the register to be
written. Registers are addressed starting at zero.

@param u16WriteAddress address of the holding register (0x0000..0xFFFF)
@param u16WriteValue value to be written to holding register (0x0000..0xFFFF)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t writeSingleRegister(uint16_t u16WriteAddress, uint16_t u16WriteValue)
{
    _u16WriteAddress = u16WriteAddress;
    _u16WriteQty = 0;
    _u16TransmitBuffer[0] = u16WriteValue;
    return ModbusMasterTransaction(ku8MBWriteSingleRegister);
}


/**
Modbus function 0x0F Write Multiple Coils.

This function code is used to force each coil in a sequence of coils to
either ON or OFF in a remote device. The request specifies the coil
references to be forced. Coils are addressed starting at zero.

The requested ON/OFF states are specified by contents of the transmit
buffer. A logical '1' in a bit position of the buffer requests the
corresponding output to be ON. A logical '0' requests it to be OFF.

@param u16WriteAddress address of the first coil (0x0000..0xFFFF)
@param u16BitQty quantity of coils to write (1..2000, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup discrete
*/
uint8_t writeMultipleCoils(uint16_t u16WriteAddress, uint16_t u16BitQty)
{
    _u16WriteAddress = u16WriteAddress;
    _u16WriteQty = u16BitQty;
    return ModbusMasterTransaction(ku8MBWriteMultipleCoils);
}

uint8_t writeMultipleCoils_0()
{
    _u16WriteQty = u16TransmitBufferLength;
    return ModbusMasterTransaction(ku8MBWriteMultipleCoils);
}


/**
Modbus function 0x10 Write Multiple Registers.

This function code is used to write a block of contiguous registers (1
to 123 registers) in a remote device.

The requested written values are specified in the transmit buffer. Data
is packed as one word per register.

@param u16WriteAddress address of the holding register (0x0000..0xFFFF)
@param u16WriteQty quantity of holding registers to write (1..123, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t writeMultipleRegisters(uint16_t u16WriteAddress, uint16_t u16WriteQty)
{
    _u16WriteAddress = u16WriteAddress;
    _u16WriteQty = u16WriteQty;
    return ModbusMasterTransaction(ku8MBWriteMultipleRegisters);
}

// new version based on Wire.h
uint8_t writeMultipleRegisters_0()
{
    _u16WriteQty = _u8TransmitBufferIndex;
    return ModbusMasterTransaction(ku8MBWriteMultipleRegisters);
}


/**
Modbus function 0x16 Mask Write Register.

This function code is used to modify the contents of a specified holding
register using a combination of an AND mask, an OR mask, and the
register's current contents. The function can be used to set or clear
individual bits in the register.

The request specifies the holding register to be written, the data to be
used as the AND mask, and the data to be used as the OR mask. Registers
are addressed starting at zero.

The function's algorithm is:

Result = (Current Contents && And_Mask) || (Or_Mask && (~And_Mask))

@param u16WriteAddress address of the holding register (0x0000..0xFFFF)
@param u16AndMask AND mask (0x0000..0xFFFF)
@param u16OrMask OR mask (0x0000..0xFFFF)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t maskWriteRegister(uint16_t u16WriteAddress,uint16_t u16AndMask, uint16_t u16OrMask)
{
    _u16WriteAddress = u16WriteAddress;
    _u16TransmitBuffer[0] = u16AndMask;
    _u16TransmitBuffer[1] = u16OrMask;
    return ModbusMasterTransaction(ku8MBMaskWriteRegister);
}


/**
Modbus function 0x17 Read Write Multiple Registers.

This function code performs a combination of one read operation and one
write operation in a single MODBUS transaction. The write operation is
performed before the read. Holding registers are addressed starting at
zero.

The request specifies the starting address and number of holding
registers to be read as well as the starting address, and the number of
holding registers. The data to be written is specified in the transmit
buffer.

@param u16ReadAddress address of the first holding register (0x0000..0xFFFF)
@param u16ReadQty quantity of holding registers to read (1..125, enforced by remote device)
@param u16WriteAddress address of the first holding register (0x0000..0xFFFF)
@param u16WriteQty quantity of holding registers to write (1..121, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t readWriteMultipleRegisters(uint16_t u16ReadAddress, uint16_t u16ReadQty, uint16_t u16WriteAddress, uint16_t u16WriteQty)
{
    _u16ReadAddress = u16ReadAddress;
    _u16ReadQty = u16ReadQty;
    _u16WriteAddress = u16WriteAddress;
    _u16WriteQty = u16WriteQty;
    return ModbusMasterTransaction(ku8MBReadWriteMultipleRegisters);
}
uint8_t readWriteMultipleRegisters_0(uint16_t u16ReadAddress, uint16_t u16ReadQty)
{
    _u16ReadAddress = u16ReadAddress;
    _u16ReadQty = u16ReadQty;
    _u16WriteQty = _u8TransmitBufferIndex;
    return ModbusMasterTransaction(ku8MBReadWriteMultipleRegisters);
}


//PZEM-003 function API
uint8_t calibrateRegister(uint16_t u16ReadAddress, uint8_t u16ReadQty)
{
    _u16ReadAddress = u16ReadAddress;
    _u16ReadQty = u16ReadQty;
    return ModbusMasterTransaction(ku8MBCalibrationRegister);
}

uint8_t resetenergyRegister(uint16_t u16ReadAddress, uint8_t u16ReadQty)
{
    _u16ReadAddress = u16ReadAddress;
    _u16ReadQty = u16ReadQty;
    return ModbusMasterTransaction(ku8MBResetEnergyRegister);
}

/* _____PRIVATE FUNCTIONS____________________________________________________ */
/**
Modbus transaction engine.
Sequence:
  - assemble Modbus Request Application Data Unit (ADU),
    based on particular function called
  - transmit request over selected serial port
  - wait for/retrieve response
  - evaluate/disassemble response
  - return status (success/exception)

@param u8MBFunction Modbus function (0x01..0xFF)
@return 0 on success; exception number on failure
*/

uint8_t ModbusMasterTransaction(uint8_t u8MBFunction)
{
    uint8_t u8ModbusADU[256];
    uint8_t u8ModbusADUSize = 0;
    uint8_t i, u8Qty;
    uint16_t u16CRC;
    uint32_t u32StartTime;
    uint8_t u8BytesLeft = 8;
    uint8_t u8MBStatus = ku8MBSuccess;
    uint32_t u32CurTime;



    // assemble Modbus Request Application Data Unit
    u8ModbusADU[u8ModbusADUSize++] = _u8MBSlave;
    u8ModbusADU[u8ModbusADUSize++] = u8MBFunction;
    if ( u8MBFunction == ku8MBCalibrationRegister )
    {
        u8BytesLeft = 6;
        u8ModbusADU[0] = 0xF8;
        u8ModbusADU[u8ModbusADUSize++] = highByte(_u16ReadAddress);
        u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16ReadAddress);

    }
    else if ( u8MBFunction == ku8MBResetEnergyRegister )
    {
        u8BytesLeft = 4;
    }

    switch(u8MBFunction)
    {
    case ku8MBReadCoils:
    case ku8MBReadDiscreteInputs:
    case ku8MBReadInputRegisters:
    case ku8MBReadHoldingRegisters:
    case ku8MBReadWriteMultipleRegisters:
        u8ModbusADU[u8ModbusADUSize++] = highByte(_u16ReadAddress);
        u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16ReadAddress);
        u8ModbusADU[u8ModbusADUSize++] = highByte(_u16ReadQty);
        u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16ReadQty);
        break;
    }

    switch(u8MBFunction)
    {
    case ku8MBWriteSingleCoil:
    case ku8MBMaskWriteRegister:
    case ku8MBWriteMultipleCoils:
    case ku8MBWriteSingleRegister:
    case ku8MBWriteMultipleRegisters:
    case ku8MBReadWriteMultipleRegisters:
        u8ModbusADU[u8ModbusADUSize++] = highByte(_u16WriteAddress);
        u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteAddress);
        break;
    }

    switch(u8MBFunction)
    {
    case ku8MBWriteSingleCoil:
        u8ModbusADU[u8ModbusADUSize++] = highByte(_u16WriteQty);
        u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteQty);
        break;

    case ku8MBWriteSingleRegister:
        u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[0]);
        u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[0]);
        break;

    case ku8MBWriteMultipleCoils:
        u8ModbusADU[u8ModbusADUSize++] = highByte(_u16WriteQty);
        u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteQty);
        u8Qty = (_u16WriteQty % 8) ? ((_u16WriteQty >> 3) + 1) : (_u16WriteQty >> 3);
        u8ModbusADU[u8ModbusADUSize++] = u8Qty;
        for (i = 0; i < u8Qty; i++)
        {
            switch(i % 2)
            {
            case 0: // i is even
                u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[i >> 1]);
                break;

            case 1: // i is odd
                u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[i >> 1]);
                break;
            }
        }
        break;

    case ku8MBWriteMultipleRegisters:
    case ku8MBReadWriteMultipleRegisters:
        u8ModbusADU[u8ModbusADUSize++] = highByte(_u16WriteQty);
        u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteQty);
        u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteQty << 1);

        for (i = 0; i < lowByte(_u16WriteQty); i++)
        {
            u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[i]);
            u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[i]);
        }
        break;

    case ku8MBMaskWriteRegister:
        u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[0]);
        u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[0]);
        u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[1]);
        u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[1]);
        break;
    }

    // append CRC
    u16CRC = 0xFFFF;
    for (i = 0; i < u8ModbusADUSize; i++)
    {
        u16CRC = crc16_update(u16CRC, u8ModbusADU[i]);
    }
    u8ModbusADU[u8ModbusADUSize++] = lowByte(u16CRC);
    u8ModbusADU[u8ModbusADUSize++] = highByte(u16CRC);
    u8ModbusADU[u8ModbusADUSize] = 0;

//  sysprintf("input -->");
//  for (i=0; i<u8ModbusADUSize;i++)
//      sysprintf(" %x,", u8ModbusADU[i]);
//  sysprintf("\n");
// write data into RS485
// RS485_TX_Addr(&RS485Uart, u8ModbusADU[0]);
    RS485_TX_Data(pUartDevISR, u8ModbusADUSize, u8ModbusADU);

    for (i=0; i<u8ModbusADUSize; i++)
        u8ModbusADU[i] = 0xFF;
    u8ModbusADUSize = 0;


    // loop until we run out of time or bytes, or an error occurs
    u32StartTime = (uint32_t)OS_TimeMS;

    RS485_RX();

    while (u8BytesLeft && !u8MBStatus)
    {
        if (!(inpw(REG_UART_FSR) & 0x00004000))
        {
            u8ModbusADU[u8ModbusADUSize++] = inpb(REG_UART_RBR);
            u8BytesLeft--;
        }

        // evaluate slave ID, function code once enough bytes have been read
        if (u8ModbusADUSize == 5)
        {
            // verify response is for correct Modbus slave
            if (u8ModbusADU[0] != _u8MBSlave)
            {
                u8MBStatus = ku8MBInvalidSlaveID;
                break;
            }

            // verify response is for correct Modbus function code (mask exception bit 7)
            if ((u8ModbusADU[1] & 0x7F) != u8MBFunction)
            {
                u8MBStatus = ku8MBInvalidFunction;
                break;
            }

            // check whether Modbus exception occurred; return Modbus Exception Code
//      if (bitRead(u8ModbusADU[1], 7))
            if ( u8ModbusADU[1] & 0x80)
            {
                u8MBStatus = u8ModbusADU[2];
                break;
            }

            // evaluate returned Modbus function code
            switch(u8ModbusADU[1])
            {
            case ku8MBReadCoils:
            case ku8MBReadDiscreteInputs:
            case ku8MBReadInputRegisters:
            case ku8MBReadHoldingRegisters:
            case ku8MBReadWriteMultipleRegisters:
                u8BytesLeft = u8ModbusADU[2];
                break;

            case ku8MBWriteSingleCoil:
            case ku8MBWriteMultipleCoils:
            case ku8MBWriteSingleRegister:
            case ku8MBWriteMultipleRegisters:
                u8BytesLeft = 3;
                break;

            case ku8MBMaskWriteRegister:
                u8BytesLeft = 5;
                break;
            }
        }

        u32CurTime = (uint32_t)OS_TimeMS;
        if ((u32CurTime - u32StartTime) > ku16MBResponseTimeout)
        {
            u8MBStatus = ku8MBResponseTimedOut;
        }

    }
//  sysprintf("output <-- ");
//  for (i=0; i<u8ModbusADUSize;i++)
//      sysprintf(" %x,", u8ModbusADU[i]);
//  sysprintf("\n");
    // verify response is large enough to inspect further
    if (!u8MBStatus && u8ModbusADUSize >= 5)
    {
        // calculate CRC
        u16CRC = 0xFFFF;
        for (i = 0; i < (u8ModbusADUSize - 2); i++)
        {
            u16CRC = crc16_update(u16CRC, u8ModbusADU[i]);
        }

        // verify CRC
        if (!u8MBStatus && (lowByte(u16CRC) != u8ModbusADU[u8ModbusADUSize - 2] ||
                            highByte(u16CRC) != u8ModbusADU[u8ModbusADUSize - 1]))
        {
            u8MBStatus = ku8MBInvalidCRC;
        }
    }

    // disassemble ADU into words
    if (!u8MBStatus)
    {
        // evaluate returned Modbus function code
        switch(u8ModbusADU[1])
        {
        case ku8MBReadCoils:
        case ku8MBReadDiscreteInputs:
            // load bytes into word; response bytes are ordered L, H, L, H, ...
            for (i = 0; i < (u8ModbusADU[2] >> 1); i++)
            {
                if (i < ku8MaxBufferSize)
                {
                    _u16ResponseBuffer[i] = word(u8ModbusADU[2 * i + 4], u8ModbusADU[2 * i + 3]);
                }

                _u8ResponseBufferLength = i;
            }

            // in the event of an odd number of bytes, load last byte into zero-padded word
            if (u8ModbusADU[2] % 2)
            {
                if (i < ku8MaxBufferSize)
                {
                    _u16ResponseBuffer[i] = word(0, u8ModbusADU[2 * i + 3]);
                }

                _u8ResponseBufferLength = i + 1;
            }
            break;

        case ku8MBReadInputRegisters:
        case ku8MBReadHoldingRegisters:
        case ku8MBReadWriteMultipleRegisters:
            // load bytes into word; response bytes are ordered H, L, H, L, ...
            for (i = 0; i < (u8ModbusADU[2] >> 1); i++)
            {
                if (i < ku8MaxBufferSize)
                {
                    _u16ResponseBuffer[i] = word(u8ModbusADU[2 * i + 3], u8ModbusADU[2 * i + 4]);
                }

                _u8ResponseBufferLength = i;
            }
            break;
        }
    }

    _u8TransmitBufferIndex = 0;
    u16TransmitBufferLength = 0;
    _u8ResponseBufferIndex = 0;
    ModbusDisplayMessage(u8ModbusADU,u8ModbusADUSize);
    return u8MBStatus;
}

void    ModbusDisplayMessage(uint8_t *u8ModbusADU, uint8_t u8ModbusADUSize)
{
    uint8_t number;
    uint8_t *pu8Cur;

    pu8Cur = (uint8_t *)&u8ModbusADU[0];
//  sysprintf("\n\n");
    if ( *pu8Cur != _u8MBSlave )
    {
        if ( *pu8Cur == 0xF8 )
        {
            pu8Cur++;
            if ( *pu8Cur == 0x41 )
            {
                sysprintf("Calibration OK\n");
            }
            else if ( *pu8Cur == 0xC1 )
            {
                sysprintf("Calibration Error\n");
            }
            else
            {
                sysprintf("Error feedback\n");
            }
        }
        else
        {
            sysprintf("Error feedback\n");
        }
        sysprintf("\n\n");
        return;
    }
    pu8Cur++;
    if ( *pu8Cur & 0x80 )
    {
        pu8Cur++;
        switch ( *pu8Cur )
        {
        case 0x01:
            sysprintf("illegal function\n");
            break;
        case 0x02 :
            sysprintf("illegal address\n");
            break;
        case 0x03 :
            sysprintf("illegal data\n");
            break;
        case 0x04 :
            sysprintf("Slave error\n");
            break;
        default:
            sysprintf("Unknown error\n");
            break;
        }
        sysprintf("\n\n");
        return;
    }
    switch ( *pu8Cur++ )
    {
    case 0x03:
        break;
    case 0x04:
        number = *pu8Cur++;
        if (number == 16 )
        {
            DispMsg_PZEM003(pu8Cur,number);
        }
        else if (number == 4)
        {
            DispMsg_Temperature(pu8Cur,number);
        }
        else
        {
            sysprintf("no message\n");
        }

        break;
    }
//   sysprintf("\n\n");
}

extern char g_Menu2Temp1Voltage[20];
extern char g_Menu2Temp1Current[20];
extern char g_Menu2Temp1Power[20];
extern char g_Menu2Temp1Energy[20];

void DispMsg_PZEM003(uint8_t *pu8Cur, uint8_t number)
{
    uint8_t count, i;
    uint16_t hibyte, lobyte,value, div,rem, hivalue;
    uint32_t u32val;
    count = 0;
    for (i=0; i<2; i++)
    {
        hibyte = *pu8Cur++;
        lobyte = *pu8Cur++;
        value = (hibyte << 8) | lobyte;
        if ( count == 0 )
        {
            div = value/100;
            rem = value % 100;
//          sysprintf("Voltage = %u.%02uV\n",div, rem);
            sprintf(g_Menu2Temp1Voltage, "Voltage:%u.%02uV", div, rem);
            count++;
        }
        else if ( count == 1 )
        {
            div = value/100;
            rem = value % 100;
//          sysprintf("Current = %u.%02uA\n",div, rem);
            sprintf(g_Menu2Temp1Current, "Current:%u.%02uA", div, rem);
            count++;
        }
    }
    number -= 4;
    for (i=0; i<2; i++)
    {
        hibyte = *pu8Cur++;
        lobyte = *pu8Cur++;
        value = (hibyte << 8) | lobyte;
        hibyte = *pu8Cur++;
        lobyte = *pu8Cur++;
        hivalue = (hibyte << 8) | lobyte;
        u32val = hivalue;
        u32val <<=16;
        u32val |= value;
        if ( count == 2 )
        {
            div = u32val/10;
            rem = u32val % 10;
//          sysprintf("Power = %u.%01uW\n",div, rem);
            sprintf(g_Menu2Temp1Power, "Power:%u.%01uW", div, rem);
            count++;
        }
        else if ( count == 3 )
        {
//          sysprintf("Energy = %uWh\n",u32val);
            sprintf(g_Menu2Temp1Energy, "Energy:%uWh", u32val);
            count++;
        }
    }
    number -= 8;
    for (i=0; i<number/2; i++)
    {
        hibyte = *pu8Cur++;
        lobyte = *pu8Cur++;
        value = (hibyte << 8) | lobyte;
        if ( count == 4 )
        {
//          sysprintf("High voltage alarm status = %04x\n", value);
            count++;
        }
        else if ( count == 5 )
        {
//          sysprintf("Low voltage alarm status = %04x\n", value);
            count++;
        }
    }
}

int c_div, c_rem, f_div, f_rem;

void DispMsg_Temperature(uint8_t *pu8Cur, uint8_t number)
{

    uint16_t hibyte, lobyte;//, c_div, c_rem, f_div, f_rem;


    hibyte = *pu8Cur++;
    lobyte = *pu8Cur++;
    c_temp = (hibyte << 8) | lobyte;
    hibyte = *pu8Cur++;
    lobyte = *pu8Cur++;
    f_temp = (hibyte << 8) | lobyte;

    c_div = c_temp/100;
    c_rem = abs(c_temp) % 100;
    f_div = f_temp/100;
    f_rem = abs(f_temp) % 100;
//  sysprintf("Temperature is %d.%02d Celsius, %d.%02d Fahrenheit\r\n", c_div,c_rem, f_div, f_rem);

}
