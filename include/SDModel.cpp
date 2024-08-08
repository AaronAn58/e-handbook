#include "SDModel.h"

SDModel::SDModel()
{
};

SDModel::~SDModel()
{
};

void SDModel::SDSpiAndSetup(SPIClass *spi)
{
    spi -> begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    SD.setSPI(spi);
    SD.begin(SD_CS_PIN);
}