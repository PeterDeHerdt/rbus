/*
 * If not stated otherwise in this file or this component's Licenses.txt file
 * the following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include <rbus.h>
#include "rbus_buffer.h"
#include <endian.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_BLOCK_SIZE 64

#define rbusHostToLittleInt16(n) htole16(n)
#define rbusLittleToHostInt16(n) le16toh(n)
#define rbusHostToLittleInt32(n) htole32(n)
#define rbusLittleToHostInt32(n) le32toh(n)
#define rbusHostToLittleInt64(n) htole64(n)
#define rbusLittleToHostInt64(n) le64toh(n)

void rbusBuffer_Create(rbusBuffer_t* buff)
{
    (*buff) = malloc(sizeof(struct _rbusBuffer));
    (*buff)->lenAlloc = BUFFER_BLOCK_SIZE;
    (*buff)->posWrite = 0;
    (*buff)->posRead = 0;
    (*buff)->data = (*buff)->block1;
}

void rbusBuffer_Destroy(rbusBuffer_t buff)
{
    if(buff->data != buff->block1)
    {
        assert(buff->lenAlloc > BUFFER_BLOCK_SIZE);
        free(buff->data);
    }
    free(buff);
}

void rbusBuffer_Write(rbusBuffer_t buff, void const* data, int len)
{
    assert(buff->data);
    int posNext = buff->posWrite+len;
    if(posNext > buff->lenAlloc)
    {
        buff->lenAlloc = (posNext/BUFFER_BLOCK_SIZE+1)*BUFFER_BLOCK_SIZE;
        if(buff->data == buff->block1)
        {
            buff->data = malloc(buff->lenAlloc);
            if(buff->posWrite > 0)
                memcpy(buff->data, buff->block1, buff->posWrite);
        }
        else
        {
            buff->data = realloc(buff->data, buff->lenAlloc);
        }
    }
    memcpy(buff->data + buff->posWrite, data, len);
    buff->posWrite = posNext;
}

void rbusBuffer_WriteTypeLengthValue(
  rbusBuffer_t      buff,
  rbusValueType_t   type,
  uint16_t          length,
  void const*       value)
{
  uint16_t letype = rbusHostToLittleInt16(type);
  uint16_t lelength = rbusHostToLittleInt16(length);
  rbusBuffer_Write(buff, &letype, sizeof(uint16_t));
  rbusBuffer_Write(buff, &lelength, sizeof(uint16_t));
  rbusBuffer_Write(buff, value, length);
}

void rbusBuffer_WriteStringTLV(rbusBuffer_t buff, char const* s, int len)
{
    /*len should be strlen(s)+1 for null terminator*/
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_STRING, len, s);
}

void rbusBuffer_WriteBooleanTLV(rbusBuffer_t buff, bool b)
{
    assert(sizeof(bool) == 1);/*otherwise we need to handle endian*/    
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_BOOLEAN, sizeof(bool), &b);
}

void rbusBuffer_WriteCharTLV(rbusBuffer_t buff, char c)
{
    assert(sizeof(char) == 1);/*otherwise we need to handle endian*/    
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_CHAR, sizeof(char), &c);
}

void rbusBuffer_WriteByteTLV(rbusBuffer_t buff, unsigned char u)
{
    assert(sizeof(unsigned char) == 1);/*otherwise we need to handle endian*/    
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_BYTE, sizeof(unsigned char), &u);
}

void rbusBuffer_WriteInt8TLV(rbusBuffer_t buff, int8_t i8)
{
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_INT8, sizeof(int8_t), &i8);
}

void rbusBuffer_WriteUInt8TLV(rbusBuffer_t buff, uint8_t u8)
{
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_UINT8, sizeof(uint8_t), &u8);
}

void rbusBuffer_WriteInt16TLV(rbusBuffer_t buff, int16_t i16)
{
    int16_t temp = rbusHostToLittleInt16(i16);
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_INT16, sizeof(int16_t), &temp);
}

void rbusBuffer_WriteUInt16TLV(rbusBuffer_t buff, uint16_t u16)
{
    uint16_t temp = rbusHostToLittleInt16(u16);
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_UINT16, sizeof(uint16_t), &temp);
}

void rbusBuffer_WriteInt32TLV(rbusBuffer_t buff, int32_t i32)
{
    int32_t temp = rbusHostToLittleInt32(i32);
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_INT32, sizeof(int32_t), &temp);
}

void rbusBuffer_WriteUInt32TLV(rbusBuffer_t buff, uint32_t u32)
{
    uint32_t temp = rbusHostToLittleInt32(u32);
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_UINT32, sizeof(uint32_t), &temp);
}

void rbusBuffer_WriteInt64TLV(rbusBuffer_t buff, int64_t i64)
{
    int64_t temp = rbusHostToLittleInt64(i64);
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_INT64, sizeof(int64_t), &temp);
}

void rbusBuffer_WriteUInt64TLV(rbusBuffer_t buff, uint64_t u64)
{
    uint64_t temp = rbusHostToLittleInt64(u64);
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_UINT64, sizeof(uint64_t), &temp);
}

void rbusBuffer_WriteSingleTLV(rbusBuffer_t buff, float f32)
{
    float temp = rbusHostToLittleInt32(f32);
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_SINGLE, sizeof(float), &temp);
}

void rbusBuffer_WriteDoubleTLV(rbusBuffer_t buff, double f64)
{
    double temp = (double)rbusHostToLittleInt64((uint64_t)f64);
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_DOUBLE, sizeof(double), &temp);
}

void rbusBuffer_WriteDateTimeTLV(rbusBuffer_t buff, struct timeval const* tv)
{
    struct timeval temp;
    temp.tv_sec = rbusHostToLittleInt64(tv->tv_sec);
    temp.tv_usec = rbusHostToLittleInt64(tv->tv_usec);
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_DATETIME, sizeof(temp), &temp);
}

void rbusBuffer_WriteBytesTLV(rbusBuffer_t buff, uint8_t* bytes, int len)
{
    rbusBuffer_WriteTypeLengthValue(buff, RBUS_BYTES, len, bytes);
}

void rbusBuffer_Read(rbusBuffer_t const buff, void* data, int len)
{
    assert(buff->data);
    assert(buff->posRead + len < buff->lenAlloc);
    memcpy(data, buff->data + buff->posRead, len);
    buff->posRead += len;
}

void rbusBuffer_ReadString(rbusBuffer_t const buff, char** s, int* len)
{
    *len = buff->posRead;
    *s = malloc(buff->posRead+1);/*TODO is +1 needed?*/
    rbusBuffer_Read(buff, *s, *len);
}

void rbusBuffer_ReadBoolean(rbusBuffer_t const buff, bool* b)
{
    assert(sizeof(bool) == 1);/*otherwise we need to handle endian*/
    rbusBuffer_Read(buff, b, sizeof(bool));
}

void rbusBuffer_ReadChar(rbusBuffer_t const buff, char* c)
{
    assert(sizeof(char) == 1);/*otherwise we need to handle endian*/
    rbusBuffer_Read(buff, c, sizeof(char));
}

void rbusBuffer_ReadByte(rbusBuffer_t const buff, unsigned char* u)
{
    assert(sizeof(unsigned char) == 1);/*otherwise we need to handle endian*/
    rbusBuffer_Read(buff, u, sizeof(unsigned char));
}

void rbusBuffer_ReadInt8(rbusBuffer_t const buff, int8_t* i8)
{
    rbusBuffer_Read(buff, i8, sizeof(int8_t));
}

void rbusBuffer_ReadUInt8(rbusBuffer_t const buff, uint8_t* u8)
{
    rbusBuffer_Read(buff, u8, sizeof(uint8_t));
}

void rbusBuffer_ReadInt16(rbusBuffer_t const buff, int16_t* i16)
{
    int16_t temp;
    rbusBuffer_Read(buff, &temp, sizeof(int16_t));
    *i16 = rbusLittleToHostInt16(temp);
}

void rbusBuffer_ReadUInt16(rbusBuffer_t const buff, uint16_t* u16)
{
    uint16_t temp;
    rbusBuffer_Read(buff, &temp, sizeof(uint16_t));
    *u16 = rbusLittleToHostInt16(temp);
}

void rbusBuffer_ReadInt32(rbusBuffer_t const buff, int32_t* i32)
{
    int32_t temp;
    rbusBuffer_Read(buff, &temp, sizeof(int32_t));
    *i32 = rbusLittleToHostInt32(temp);
}

void rbusBuffer_ReadUInt32(rbusBuffer_t const buff, uint32_t* u32)
{
    uint32_t temp;
    rbusBuffer_Read(buff, &temp, sizeof(uint32_t));
    *u32 = rbusLittleToHostInt32(temp);
}

void rbusBuffer_ReadInt64(rbusBuffer_t const buff, int64_t* i64)
{
    int64_t temp;
    rbusBuffer_Read(buff, &temp, sizeof(int64_t));
    *i64 = rbusLittleToHostInt64(temp);
}

void rbusBuffer_ReadUInt64(rbusBuffer_t const buff, uint64_t* u64)
{
    uint64_t temp;
    rbusBuffer_Read(buff, &temp, sizeof(uint64_t));
    *u64 = rbusLittleToHostInt64(temp);
}

void rbusBuffer_ReadSingle(rbusBuffer_t const buff, float* f32)
{
    float temp;
    rbusBuffer_Read(buff, &temp, sizeof(float));
    *f32 = rbusLittleToHostInt32(temp);
}

void rbusBuffer_ReadDouble(rbusBuffer_t const buff, double* f64)
{
    double temp;
    rbusBuffer_Read(buff, &temp, sizeof(double));
    *f64 = (double)rbusLittleToHostInt64((uint64_t)temp);
}

void rbusBuffer_ReadDateTime(rbusBuffer_t const buff, struct timeval* tv)
{
    struct timeval temp;
    rbusBuffer_Read(buff, &temp, sizeof(struct timeval));
    tv->tv_sec = rbusLittleToHostInt64(temp.tv_sec);
    tv->tv_usec = rbusLittleToHostInt64(temp.tv_usec);
}

void rbusBuffer_ReadBytes(rbusBuffer_t const buff, uint8_t** bytes, int* len)
{
    *len = buff->posWrite;
    *bytes = malloc(buff->posWrite);
    rbusBuffer_Read(buff, bytes, buff->posWrite);
}


