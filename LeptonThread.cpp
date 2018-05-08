#include "LeptonThread.h"
#include "stdio.h"
#include "Palettes.h"
#include "SPI.h"
#include "Lepton_I2C.h"

#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME 60
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)
#define FPS 27

#define WIDTH 80
#define HEIGHT 60

#define HAND_TEMP_THRESHOLD 8050

int readyToToggle = 1;

LeptonThread::LeptonThread() : QThread()
{
}

LeptonThread::~LeptonThread() {
}

void LeptonThread::run()
{
	//create the initial image
	myImage = QImage(WIDTH, HEIGHT, QImage::Format_RGB888);

	//open spi port
	SpiOpenPort(0);

	while(true) {
		//read data packets from lepton over SPI
		int resets = 0;
		for(int j=0;j<PACKETS_PER_FRAME;j++) {
			//if it's a drop packet, reset j to 0, set to -1 so he'll be at 0 again loop
			read(spi_cs0_fd, result+sizeof(uint8_t)*PACKET_SIZE*j, sizeof(uint8_t)*PACKET_SIZE);
			int packetNumber = result[j*PACKET_SIZE+1];
			if(packetNumber != j) {
				j = -1;
				resets += 1;
				usleep(1000);
				//Note: we've selected 750 resets as an arbitrary limit, since there should never be 750 "null" packets between two valid transmissions at the current poll rate
				//By polling faster, developers may easily exceed this count, and the down period between frames may then be flagged as a loss of sync
				if(resets == 750) {
					SpiClosePort(0);
					usleep(750000);
					SpiOpenPort(0);
				}
			}
		}
		if(resets >= 30) {
			qDebug() << "done reading, resets: " << resets;
		}

		frameBuffer = (uint16_t *)result;
		int row, column;
		uint16_t value;
		uint16_t minValue = 65535;
		uint16_t maxValue = 0;

		uint32_t totalCounts = 0;
		
		for(int i=0;i<FRAME_SIZE_UINT16;i++) {
			//skip the first 2 uint16_t's of every packet, they're 4 header bytes
			if(i % PACKET_SIZE_UINT16 < 2) {
				continue;
			}
			
			//flip the MSB and LSB at the last second
			int temp = result[i*2];
			result[i*2] = result[i*2+1];
			result[i*2+1] = temp;
			
			value = frameBuffer[i];
			totalCounts += value;
			if(value > maxValue) {
				maxValue = value;
			}
			if(value < minValue) {
				minValue = value;
			}
			column = i % PACKET_SIZE_UINT16 - 2;
			row = i / PACKET_SIZE_UINT16 ;
		}
		/*
		//CODE FOR HAND DETECTION
		float totalCountsFloat = totalCounts;
		totalCountsFloat = totalCountsFloat/4800;
		if(totalCountsFloat >= HAND_TEMP_THRESHOLD){
		  if(readyToToggle == 1){
		    system("sudo python toggleLED.py");
		    printf("HAND DETECTED\n");
		    readyToToggle = 0;
		  }
		}
		else{
		  if(readyToToggle == 0){
		    printf("No hand.\n");
		    readyToToggle = 1;
		  }
		}
		*/
		//printf("totalCountsFloat %f\n",totalCountsFloat);

		float diff = maxValue - minValue;
		float scale = 255/diff;
		QRgb color;
		//Find radiometric data from spot meter in 3x3 pixel box around the center of the image
		float radValue = 0;
		for(int i=0;i<3;i++){
			for(int j=0;j<3;j++){
				radValue += frameBuffer[((HEIGHT/2 + i) * PACKET_SIZE_UINT16)+ PACKET_SIZE_UINT16/2 - 1 + j];
			}
		}
		radValue = radValue / 9;
		//Radiometry values are the temperature in Kelvin * 100.
		float tempK = radValue/100;
		float tempC = tempK - 273.15;
		//float tempF = tempC * 1.8 + 32;
		
		//Update the UI with the new temperature info
		QString s;
		s.sprintf("%.2f C", tempC);
		emit updateRadiometry(s);

		for(int i=0;i<FRAME_SIZE_UINT16;i++) {
			if(i % PACKET_SIZE_UINT16 < 2) {
				continue;
			}
			value = (frameBuffer[i] - minValue) * scale;
			const int *colormap = colormap_ironblack;
			color = qRgb(colormap[3*value], colormap[3*value+1], colormap[3*value+2]);
			column = (i % PACKET_SIZE_UINT16 ) - 2;
			row = i / PACKET_SIZE_UINT16;
			myImage.setPixel(column, row, color);
		}

		//Draw crosshairs in the middle of the image
		for(int j = 0; j < 5; j++){
			myImage.setPixel(WIDTH/2-1, HEIGHT/2-3+j, 0);
			myImage.setPixel(WIDTH/2-3+j, HEIGHT/2-1, 0);
		}

		//emit the signal for update
		emit updateImage(myImage);

	}
	
	//finally, close SPI port
	SpiClosePort(0);
}

void LeptonThread::performFFC() {
	//perform FFC
	lepton_perform_ffc();
}
