#include "mks_wifi_sd.h"

#include "../../MarlinCore.h"
#include "../../lcd/marlinui.h"
#include "../../libs/buzzer.h"  
#include "../temperature.h"

#ifdef MKS_WIFI

#ifdef STM32F4
#include "uart.h"
#include "mks_wifi_hal_f4.h"
#endif

#ifdef STM32F1
#include "mks_wifi_hal_f1.h"

#ifndef MAPLE_STM32F1
#include "uart.h"
#endif

#endif

#if ENABLED(TFT_480x320) || ENABLED(TFT_480x320_SPI)
#include "mks_wifi_ui.h"
#endif

volatile uint8_t *file_buff=shared_mem;
volatile uint8_t *file_buff_pos;
volatile uint16_t file_data_size;

extern volatile uint8_t *buff;
extern volatile uint8_t buffer_ready;
extern volatile uint8_t dma_stopped;

extern volatile uint8_t *dma_buff[2];
extern volatile uint8_t dma_buff_index;

void mks_wifi_start_file_upload(ESP_PROTOC_FRAME *packet) {
   int16_t bytes_writen = 0;
	uint32_t file_size, file_inc_size, file_size_writen;

   uint16_t in_sector;
   uint16_t last_sector;

   uint32_t dma_timeout;
   uint16_t data_size;
   int16_t save_bed,save_e0;

   uint32_t data_to_write = 0;
   uint8_t *data_packet;
   char file_name[128];

   WRITE(MKS_WIFI_IO4, HIGH); //Остановить передачу от ESP

   dma_buff[0] = file_buff + FILE_BUFFER_SIZE;
   dma_buff[1] = file_buff + FILE_BUFFER_SIZE + ESP_PACKET_SIZE;

   save_bed=thermalManager.degTargetBed();
   save_e0=thermalManager.degTargetHotend(0);
   
   MKS_WIFI_DEBUG("Saved target temp E0 %d Bed %d", save_e0, save_bed);
   
   thermalManager.setTargetBed(0);
   thermalManager.setTargetHotend(0,0);
   thermalManager.task();
 	//Установить имя файла. Смещение на 3 байта, чтобы добавить путь к диску
   //file_name[0]='0';
   //file_name[1]=':';
   //file_name[2]='/';

   memcpy((uint8_t *)file_name/*+3*/,(uint8_t *)&packet->data[5],(packet->dataLen - 5));
   file_name[packet->dataLen - 5/* + 3*/] = 0; 

   file_size = (packet->data[4] << 24) | (packet->data[3] << 16) | (packet->data[2] << 8) | packet->data[1];
   MKS_WIFI_DEBUG("Start file %s size %d", file_name, file_size);
   
   if(!card.isMounted()) {
      MKS_WIFI_DEBUG("[ERROR] Error SD mount");
      ui.set_status((const char *)"Error disk mount", true);
      return;
   }

   SdFile file, *dir;
   
   MKS_WIFI_DEBUG("Open file");

   card.cdroot();
   const char * const fname = card.diveToFile(false, dir, file_name);

   //открыть файл для записи
   if (!fname || !file.open(dir, fname, O_CREAT | O_APPEND | O_WRITE | O_TRUNC)) {
      MKS_WIFI_DEBUG("[ERROR] File open error %s", file_name);
      ui.set_status((const char *)"File open error", true);
      return;
   }

   #if ENABLED(TFT_480x320) || ENABLED(TFT_480x320_SPI)
      #ifdef SHOW_PROGRESS
         mks_update_status(file_name+3,0,file_size);
      #else
         mks_upload_screen();
      #endif
   #endif   

   dma_buff_index = 0;
   file_inc_size = 0; //Счетчик принятых данных, для записи в файл
   file_size_writen = 0; //Счетчик записанных в файл данных
   file_data_size = 0;
   dma_timeout = DMA_TIMEOUT; //Тайм-аут, на случай если передача зависла.
   last_sector = 0;
   buffer_ready = 0;
   
   mks_wifi_empty_uart();
   mks_wifi_hw_prepare((unsigned int)dma_buff[dma_buff_index], ESP_PACKET_SIZE);

   TERN_(USE_WATCHDOG, wd_reset());
   
   MKS_WIFI_DEBUG("DMA1 buff: %0X", dma_buff[0]);
   MKS_WIFI_DEBUG("DMA2 buff: %0X", dma_buff[1]);
   MKS_WIFI_DEBUG("File buff: %0X size %d (%0X)", file_buff, FILE_BUFFER_SIZE, FILE_BUFFER_SIZE);

   WRITE(FAN_PIN, HIGH);
   WRITE(HEATER_0_PIN, LOW);
   WRITE(HEATER_BED_PIN, LOW);

   #if HOTENDS == 2
      WRITE(HEATER_1_PIN,LOW);
   #endif

   data_packet = 0;
   buffer_ready = 0;

   uint32_t dma_err = 0;

   WRITE(MKS_WIFI_IO4, LOW); 

   while(--dma_timeout > 0) {
      if(buffer_ready > 0) {
         if(data_packet != buff) {
            data_packet = (uint8_t*)buff;
         } else {
            MKS_WIFI_DEBUG("Change");
            if(dma_buff_index) {
               data_packet = (uint8_t*)dma_buff[1];
            } else {
               data_packet = (uint8_t*)dma_buff[0];
            }
         }

         if(*data_packet != ESP_PROTOC_HEAD) {
            MKS_WIFI_DEBUG("[ERROR] Wrong packet head %0X at %0X", *data_packet, data_packet);
            break;
         }

         in_sector = (*(data_packet + 5) << 8) | *(data_packet + 4);
         
         if((in_sector - last_sector) > 1) {
            MKS_WIFI_DEBUG("[ERROR] IN Sec: %d Prev sec: %d", in_sector, last_sector);
            dma_stopped = 2;
            break;
         } else {
            last_sector = in_sector;
         }

         data_size = (*(data_packet + 3) << 8) | *(data_packet + 2);
         data_size -= 4; //4 байта с номером сегмента и флагами

         memcpy((uint8_t*)file_buff + file_data_size, (uint8_t*)(data_packet + 8), data_size);
         file_data_size += data_size;

         //Если буфер полон и писать некуда, запись в файл
         if((file_data_size + ESP_PACKET_SIZE) > FILE_BUFFER_SIZE) {
            data_to_write = file_data_size / 512;
            data_to_write = data_to_write * 512;

            file_inc_size += data_to_write; 
            MKS_WIFI_DEBUG("%d [%d]Save %d bytes (%d of %d) ", buffer_ready, in_sector, data_to_write, file_inc_size, file_size);

            bytes_writen = file.write((const void*)file_buff, data_to_write);

            if(file.writeError) {
               MKS_WIFI_DEBUG("[ERROR] Write error");
               break;
            }

            file_size_writen += bytes_writen;
            file_data_size -= data_to_write;
            
            memcpy((uint8_t*)file_buff, (uint8_t*)(file_buff + data_to_write), file_data_size);
         }

         if(in_sector == 0) {
            if(data_size == file_size) {
               MKS_WIFI_DEBUG("1-packet file");
               *(data_packet + 7) = 0x80;
            }
         }

         if(*(data_packet + 7) == 0x80) { //Последний пакет с данными
            WRITE(MKS_WIFI_IO4, HIGH); //Остановить передачу от ESP
            MKS_WIFI_DEBUG("Last packet");

            if(file_data_size != 0 ) {
               file_inc_size += file_data_size; 

               MKS_WIFI_DEBUG("Save last %d bytes from buffer (%d of %d)", file_data_size, file_inc_size, file_size);

               bytes_writen = file.write((const void*)file_buff, file_data_size);

               if(file.writeError) {
                  MKS_WIFI_DEBUG("[ERROR] Write error");
                  break;
               }

               file_size_writen += bytes_writen;
            }

            break;
         }

         if(buffer_ready > 0) {
            --buffer_ready;
         };

         if((buffer_ready == 0) && (dma_stopped == 1)) {
               MKS_WIFI_DEBUG("Start");
               mks_wifi_empty_uart();
               WRITE(MKS_WIFI_IO4, LOW);
               dma_stopped = 0;
         }

         if(dma_stopped == 2) {
            break;
         }

         dma_timeout = DMA_TIMEOUT;
      } else {
         TERN_(USE_WATCHDOG, wd_reset());
      }
   }
   
   mks_wifi_hw_restore();   
   
   if((dma_timeout == 0) || (dma_stopped == 2)) {
      dma_err = 10*dma_stopped + (dma_timeout == 0 ? 1 : 0);
      //Restart ESP8266
      mks_wifi_send_exception(0x01);
      WRITE(MKS_WIFI_IO_RST, LOW);
      delay(200);	
      WRITE(MKS_WIFI_IO_RST, HIGH);
   }
   
   //Выключить DMA
   mks_wifi_disable_dma();

   MSERIAL(MKS_WIFI_SERIAL_NUM).begin(MKS_WIFI_SERIAL_BAUDRATE);
   
   TERN_(USE_WATCHDOG, wd_reset());

   file.close();
   MKS_WIFI_DEBUG("File closed");

   if( (file_size == file_inc_size) && (file_size == file_size_writen) ){
         TERN_(USE_WATCHDOG, wd_reset());

#if ENABLED(TFT_480x320) || ENABLED(TFT_480x320_SPI)
         mks_end_transmit();
#endif
         ui.set_status((const char *)file_name,true);
         MKS_WIFI_DEBUG("Uploaded sucessfully");
         mks_wifi_send_exception(0x02);
         BUZZ(128, 1024);
   } else {
         TERN_(USE_WATCHDOG, wd_reset());
#if ENABLED(TFT_480x320) || ENABLED(TFT_480x320_SPI)
         mks_end_transmit();
#endif
         ui.set_status((const char *)"Upload error", true);
         MKS_WIFI_DEBUG("[ERROR] Upload failed! DMA Error: %d, File size: %d, Recieve %d; SD write %d", dma_err, file_size, file_inc_size, file_size_writen);

         //MKS_WIFI_DEBUG("Rename file %s", file_name);
         //file.rename(dir, "file_failed.gcode");

         mks_wifi_send_exception(0x01);
         BUZZ(218, 512);
         BUZZ(54,0);
         BUZZ(218, 512);
         BUZZ(54,0);
         BUZZ(218, 512);
   }

   WRITE(MKS_WIFI_IO4, LOW); //Включить передачу от ESP 

   TERN_(USE_WATCHDOG, wd_reset());

   thermalManager.setTargetBed(save_bed);
   thermalManager.setTargetHotend(save_e0,0);
   
   MKS_WIFI_DEBUG("Restore thermal settings E0:%d Bed:%d", save_bed, save_e0);
}


#endif