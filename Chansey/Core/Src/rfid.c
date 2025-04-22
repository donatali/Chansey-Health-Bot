#include "rfid.h"

//SPI transfer

uint8_t RC522_SPI_Transfer(uint8_t data){
	uint8_t rx_data;
	HAL_SPI_TransmitReceive(&hspi1, &data, &rx_data, 1, 100);
	return rx_data;
}

void Write_MFRC522(uint8_t addr, uint8_t val){
	//cs low to select the device
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

	//select addr
	RC522_SPI_Transfer((addr << 1) & 0x7E);

	//write to addr
	RC522_SPI_Transfer(val);

	//set back to high
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

uint8_t Read_MFRC522(uint8_t addr){
	uint8_t val;

	//cs low to select SPI device
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

	//select addr
	RC522_SPI_Transfer(((addr << 1) & 0x7E) | 0x80);

	//read addr
	val = RC522_SPI_Transfer(0x00);

	//set back to high
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	return val;
}

//set bits in a MFRC522 register
void SetBitMask(uint8_t reg, uint8_t mask){
	uint8_t tmp;
	tmp = Read_MFRC522(reg);
	Write_MFRC522(reg, tmp | mask);
}

//reset bits in register
void ClearBitMask(uint8_t reg, uint8_t mask){
	uint8_t tmp;
	tmp = Read_MFRC522(reg);
	Write_MFRC522(reg, tmp & (~mask));
}

void AntennaOn(void){
	Read_MFRC522(TxControlReg);
	SetBitMask(TxControlReg, 0x03);
}

void AntennaOff(void){
	ClearBitMask(TxControlReg, 0x03);
}

//reset RC522
void MFRC522_Reset(void){
	Write_MFRC522(CommandReg, PCD_RESETPHASE);
}

//start RC522
void MFRC522_Init(void){

	//CS pin A4
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

	//reset pin A8
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

	MFRC522_Reset();

	Write_MFRC522(TModeReg, 0x8D);
	Write_MFRC522(TPrescalerReg, 0x3E);
	Write_MFRC522(TReloadRegL, 30);
	Write_MFRC522(TReloadRegH, 0);

	Write_MFRC522(TxAutoReg, 0x40);
	Write_MFRC522(ModeReg, 0x3D);

	AntennaOn();
}

uint8_t MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint *backLen){
	uint8_t status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRQ = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint i;

	switch (command){
		case PCD_AUTHENT:
		{
			irqEn = 0x12;
			waitIRQ = 0x10;
			break;
		}
		case PCD_TRANSCEIVE:
		{
			irqEn = 0x77;
			waitIRQ = 0x30;
			break;
		}
		default:
			break;
	}

	Write_MFRC522(CommIEnReg, irqEn|0x80);
	ClearBitMask(CommIrqReg, 0x80);
	SetBitMask(FIFOLevelReg, 0x80);

	Write_MFRC522(CommandReg, PCD_IDLE);

	for(i = 0; i < sendLen; ++i){
		Write_MFRC522(FIFODataReg, sendData[i]);
	}

	Write_MFRC522(CommandReg, command);
	if(command == PCD_TRANSCEIVE){
		SetBitMask(BitFramingReg, 0x80);
	}

	i = 2000;
	do{
		n = Read_MFRC522(CommIrqReg);
		--i;
	}
	while((i != 0) && !(n & 0x01) && !(n & waitIRQ));

	ClearBitMask(BitFramingReg, 0x80);

	if(i != 0){
		if(!(Read_MFRC522(ErrorReg) & 0x1B)){
			status = MI_OK;
			if(n & irqEn & 0x01){
				status = MI_NOTAGERR;
			}

			if(command == PCD_TRANSCEIVE){
				n = Read_MFRC522(FIFOLevelReg);
				lastBits = Read_MFRC522(ControlReg) & 0x07;
				if(lastBits){
					*backLen = (n-1)*8 + lastBits;
				}
				else{
					*backLen = n*8;
				}

				if(n == 0){
					n = 1;
				}
				if(n > MAX_LEN){
					n = MAX_LEN;
				}

				for(i = 0; i < n; ++i){
					backData[i] = Read_MFRC522(FIFODataReg);
				}
			}
		}
		else{
			status = MI_ERR;
		}
	}
	return status;
}

uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType){
	uint8_t status;
	uint backBits;

	Write_MFRC522(BitFramingReg, 0x07);

	TagType[0] = reqMode;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	if((status != MI_OK) || (backBits != 0x10)){
		status = MI_ERR;
	}

	return status;
}


uint8_t MFRC522_Anticoll(uint8_t *serNum){
	uint8_t status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint unLen = 0;

	Write_MFRC522(BitFramingReg, 0x00);

	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if(status == MI_OK){
		for(i = 0; i < 4; ++i){
			serNumCheck ^= serNum[i];
		}
		if(serNumCheck != serNum[i]){
			status = MI_ERR;
		}
	}

	return status;
}

void CalculateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData){
	uint8_t i, n;

	ClearBitMask(DivIrqReg, 0x04);
	SetBitMask(FIFOLevelReg, 0x80);

	for(i = 0; i < len; ++i){
		Write_MFRC522(FIFODataReg, *(pIndata+i));
	}

	Write_MFRC522(CommandReg, PCD_CALCCRC);

	i = 0xFF;
	do{
		n = Read_MFRC522(DivIrqReg);
		i--;
	}
	while((i != 0) && !(n & 0x04));

	pOutData[0] = Read_MFRC522(CRCResultRegL);
	pOutData[1] = Read_MFRC522(CRCResultRegM);
}

uint8_t MFRC522_SelectTag(uint8_t *serNum){
	uint8_t i;
	uint8_t status;
	uint8_t size;
	uint recvBits;
	uint8_t buffer[9];

	buffer[0] = PICC_SElECTTAG;
	buffer[1] = 0x70;

	for(i = 0; i < 5; ++i){
		buffer[i + 2] = *(serNum + i);
	}

	CalculateCRC(buffer, 7, &buffer[7]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if((status == MI_OK) && (recvBits == 0x18)){
		size = buffer[0];
	}
	else{
		size = 0;
	}

	return size;
}

uint8_t MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t *Sectorkey, uint8_t *serNum){
	uint8_t status;
	uint recvBits;
	uint8_t i;
	uint8_t buff[12];

	buff[0] = authMode;
	buff[1] = BlockAddr;

	for(i = 0; i < 6; ++i){
		buff[i + 2] = *(Sectorkey + i);
	}

	for(i = 0; i < 4; ++i){
		buff[i + 8] = *(serNum + i);
	}

	status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

	if((status != MI_OK) || (!(Read_MFRC522(Status2Reg & 0x08)))){
		status = MI_ERR;
	}

	return status;
}

uint8_t MFRC522_Read(uint8_t blockAddr, uint8_t *recvData)
{
    uint8_t status;
    uint unLen;

    recvData[0] = PICC_READ;
    recvData[1] = blockAddr;
    CalculateCRC(recvData,2, &recvData[2]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

    if ((status != MI_OK) || (unLen != 0x90))
    {
        status = MI_ERR;
    }

    return status;
}

uint8_t MFRC522_Write(uint8_t blockAddr, uint8_t *writeData)
{
    uint8_t status;
    uint recvBits;
    uint8_t i;
	uint8_t buff[18];

    buff[0] = PICC_WRITE;
    buff[1] = blockAddr;
    CalculateCRC(buff, 2, &buff[2]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
    {
		status = MI_ERR;
	}

    if (status == MI_OK)
    {
        for (i=0; i<16; i++)		//16 FIFO bytes recorded
        {
        	buff[i] = writeData[i];
        }
        CalculateCRC(buff, 16, &buff[16]);
        status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
        {
			status = MI_ERR;
		}
    }

    return status;
}

void MFRC522_Halt(void)
{
	uint unLen;
	uint8_t buff[4];

	buff[0] = PICC_HALT;
	buff[1] = 0;
	CalculateCRC(buff, 2, &buff[2]);

	MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff,&unLen);
}


uint8_t getSerNum(uint8_t *serNum){
	MFRC522_Reset();
	MFRC522_Init();
	HAL_Delay(10);
	MFRC522_Request(PICC_REQIDL, serNum);
	return MFRC522_Anticoll(serNum);
}

uint16_t returnTrainer(uint8_t *serNum){
	uint8_t ABE[5] = {136, 4, 75, 69, 130};
	uint8_t abeMatch = 0;

	uint8_t ALEC[5] = {136, 4, 67, 69, 138};
	uint8_t alecMatch = 0;

	uint8_t ANNA[5] = {136, 4, 55, 69, 254};
	uint8_t annaMatch = 0;

	uint8_t AVA[5] = {136, 4, 74, 69, 131};
	uint8_t avaMatch = 0;

	uint8_t JAMES[5] = {136, 4, 66, 69, 139};
	uint8_t jamesMatch = 0;

	uint8_t JOHN[5] = {136, 4, 95, 69, 150};
	uint8_t johnMatch = 0;

	uint8_t JOSEPH[5] = {136, 4, 94, 69, 151};
	uint8_t josephMatch = 0;

	uint8_t JUNYI[5] = {136, 4, 97, 69, 168};
	uint8_t junyiMatch = 0;

	uint8_t KRIS[5] = {136, 4, 103, 69, 174};
	uint8_t krisMatch = 0;

	uint8_t MATT[5] = {136, 4, 102, 69, 175};
	uint8_t mattMatch = 0;

	uint8_t NATALIE[5] = {136, 4, 65, 69, 136};
	uint8_t natalieMatch = 0;



	for(int i = 0; i < 5; ++i){
		if(serNum[i] == ABE[i]){
			abeMatch++;
		}
		if(serNum[i] == ALEC[i]){
			alecMatch++;
		}
		if(serNum[i] == ANNA[i]){
			annaMatch++;
		}
		if(serNum[i] == AVA[i]){
			avaMatch++;
		}
		if(serNum[i] == JAMES[i]){
			jamesMatch++;
		}
		if(serNum[i] == JOHN[i]){
			johnMatch++;
		}
		if(serNum[i] == JOSEPH[i]){
			josephMatch++;
		}
		if(serNum[i] == JUNYI[i]){
			junyiMatch++;
		}
		if(serNum[i] == KRIS[i]){
			krisMatch++;
		}
		if(serNum[i] == MATT[i]){
			mattMatch++;
		}
		if(serNum[i] == NATALIE[i]){
			natalieMatch++;
		}
	}

	if(abeMatch == 5){
		return 1;
	}
	else if(alecMatch == 5){
		return 2;
	}
	else if(annaMatch == 5){
		return 3;
	}
	else if(avaMatch == 5){
		return 4;
	}
	else if(jamesMatch == 5){
		return 5;
	}
	else if(johnMatch == 5){
		return 6;
	}
	else if(josephMatch == 5){
		return 7;
	}
	else if(junyiMatch == 5){
		return 8;
	}
	else if(krisMatch == 5){
		return 9;
	}
	else if(mattMatch == 5){
		return 10;
	}
	else if(natalieMatch == 5){
		return 11;
	}
	else{
		return 0;
	}
}

uint8_t readCard(uint8_t blockNum, uint8_t *readData){
	uint8_t *serNum;
	uint8_t KEY[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	 uint8_t status = getSerNum(serNum);
	 if(status == MI_OK){
			MFRC522_SelectTag(serNum);
			MFRC522_Auth(PICC_AUTHENT1A, blockNum, KEY, serNum);
			return MFRC522_Read(blockNum, readData);
	 }
}

uint8_t writeCard(uint8_t blockNum, uint8_t *writeData){
	//never write to 0, OR 3, 7, 11, etc
	uint8_t *serNum;
	uint8_t KEY[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t status = getSerNum(serNum);
	if(status == MI_OK){
		MFRC522_SelectTag(serNum);
		MFRC522_Auth(PICC_AUTHENT1A, blockNum, KEY, serNum);
		return MFRC522_Write(blockNum, writeData);
	}
}












