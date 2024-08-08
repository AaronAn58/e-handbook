#ifndef SDMODEL_H
#define SDMODEL_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "common.h"
#include "driver/spi_master.h"
#include "readerPosRecord.h"

#define SD_SCK 18
#define SD_MISO 15
#define SD_MOSI 13
#define SD_CS 14

typedef struct FileInfo
{
    const char *name;
    size_t size;
    FileInfo *next;
} FileInfo;

struct CharWithPos
{
    char *str;
    int start_pos;
};
class SDModel
{
public:
    SDModel();
    ~SDModel();
    void SDSpiAndSetup(SPIClass *spi);
    File sd_file_write_ready(String);
    bool sd_file_exists(String);
    void sd_files_dir(String);
    void freeFilesInfo();
    void record_book_read_pos_single(const char *file_name);
    long book_recorder_read_pos_single(const char *file_name);
    long book_recorder_pos_and_write_eep(const char *file_name);
    char *read_book_content_from_last_pos(const char *file_path, uint16_t read_size, long his_read_pos);
    CharWithPos reverse_read_book_content_from_last_pos(const char *file_path, uint16_t read_size, long his_read_pos, uint16_t page_chars);
    void del_book(const char *file_name);
extern FileInfo *file_list;

};

#endif