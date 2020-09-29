#include <M5Sys2.h>
#include <driver/i2s.h>
#include <fft.h>

#define MODE_MIC                0
#define MODE_SPK                1
#define CONFIG_I2S_BCK_PIN      12
#define CONFIG_I2S_LRCK_PIN     0
#define CONFIG_I2S_DATA_PIN     2
#define CONFIG_I2S_DATA_IN_PIN  34
#define SPEAKER_I2S_NUMBER      I2S_NUM_0

typedef struct {
    uint8_t   state;
    void*     audioPtr;
    uint32_t  audioSize;
} i2sQueueMsg_t;


static QueueHandle_t  fftvalueQueue = nullptr;
static QueueHandle_t  i2sstateQueue = nullptr;
TFT_eSprite           DisFFTbuff    = TFT_eSprite(&M5.Lcd);


bool InitI2SSpeakerOrMic(int mode) {
    esp_err_t err = ESP_OK;

    i2s_driver_uninstall(SPEAKER_I2S_NUMBER);
    i2s_config_t i2s_config     = {
        .mode                   = (i2s_mode_t)(I2S_MODE_MASTER),
        .sample_rate            = 44100,
        .bits_per_sample        = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
        .channel_format         = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format   = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags       = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count          = 2,
        .dma_buf_len            = 128,
    };
    if (mode == MODE_MIC) {
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
    }
    else {
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
        i2s_config.use_apll             = false;
        i2s_config.tx_desc_auto_clear   = true;
    }

    err += i2s_driver_install(SPEAKER_I2S_NUMBER, &i2s_config, 0, NULL);
    i2s_pin_config_t tx_pin_config;

    tx_pin_config.bck_io_num    = CONFIG_I2S_BCK_PIN;
    tx_pin_config.ws_io_num     = CONFIG_I2S_LRCK_PIN;
    tx_pin_config.data_out_num  = CONFIG_I2S_DATA_PIN;
    tx_pin_config.data_in_num   = CONFIG_I2S_DATA_IN_PIN;

    err += i2s_set_pin(SPEAKER_I2S_NUMBER, &tx_pin_config);
    err += i2s_set_clk(SPEAKER_I2S_NUMBER, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
    return true;
}


static void i2sMicroFFTtask(void *arg) {
    uint8_t   FFTDataBuff[128];
    uint8_t   FFTValueBuff[24];
    uint8_t*  microRawData = (uint8_t*)calloc(2048,sizeof(uint8_t));
    size_t    bytesread;
    int16_t*  buffptr;
    double    data = 0;
    float     adc_data;
    uint16_t  ydata;
    uint32_t  subData;

    uint8_t state = MODE_MIC;
    i2sQueueMsg_t QueueMsg;
    while(true) {
        if(xQueueReceive(i2sstateQueue, &QueueMsg, (TickType_t)0) == pdTRUE) {
            //Serial.println("Queue Now");
            if(QueueMsg.state == MODE_MIC) {
                InitI2SSpeakerOrMic(MODE_MIC);
                state = MODE_MIC;
            }
            else {
                //Serial.printf("Length:%d",QueueMsg.audioSize);
                InitI2SSpeakerOrMic(MODE_SPK);
                size_t written = 0;
                i2s_write(SPEAKER_I2S_NUMBER, (unsigned char*)QueueMsg.audioPtr, QueueMsg.audioSize, &written, portMAX_DELAY);
                state = MODE_SPK;
            }
        }
        else if(state == MODE_MIC) {
            fft_config_t *real_fft_plan = fft_init(1024, FFT_REAL, FFT_FORWARD, NULL, NULL);
            i2s_read(I2S_NUM_0, (char *)microRawData, 2048, &bytesread, (100 / portTICK_RATE_MS));
            buffptr = (int16_t*)microRawData;

            for (int count_n = 0; count_n < real_fft_plan->size; count_n++) {
                adc_data = (float)map(buffptr[count_n], INT16_MIN, INT16_MAX, -2000, 2000);
                real_fft_plan->input[count_n] = adc_data;
            }
            fft_execute(real_fft_plan);

            for (int count_n = 1; count_n < real_fft_plan->size / 4; count_n++) {
                data = sqrt(real_fft_plan->output[2 * count_n] * real_fft_plan->output[2 * count_n] +
                            real_fft_plan->output[2 * count_n + 1] * real_fft_plan->output[2 * count_n + 1]);
                if ((count_n - 1) < 128) {
                    data  = (data > 2000) ? 2000 : data;
                    ydata = map(data, 0, 2000, 0, 255);
                    FFTDataBuff[128 - count_n] = ydata;
                }
            }

            for(int count = 0; count < 24; count++) {
                subData = 0;
                for(int count_i = 0; count_i < 5; count_i++) {
                    subData += FFTDataBuff[count * 5 + count_i ];
                }
                subData /= 5;
                FFTValueBuff[count] = map(subData, 0, 255, 0, 8);
            }
            xQueueSend(fftvalueQueue, (void*)&FFTValueBuff, 0);
            fft_destroy(real_fft_plan);
            //Serial.printf("mmp\r\n");
        }
        else {
            delay(10);
        }
    }
}



void MicroPhoneFFT() {
    uint8_t FFTValueBuff[24];
    xQueueReceive(fftvalueQueue, (void*)&FFTValueBuff, portMAX_DELAY);
    DisFFTbuff.fillRect(0, 0, 312, 128, BLACK);
    uint32_t colorY = DisFFTbuff.color565(0xff, 0x9c, 0x00);
    uint32_t colorG = DisFFTbuff.color565(0x66, 0xff, 0x00);
    uint32_t colorRect;
    for(int x = 0; x < 24; x++) {
        for( int y = 0; y < 9; y++) {
            if(y < FFTValueBuff[23-x]) {
                colorRect = colorY;
            }
            else if(y == FFTValueBuff[23-x]) {
                colorRect = colorG;
            }
            else {
                continue;
            }
            DisFFTbuff.fillRect(x * 13, 128 - y * 13 - 13, 11, 11, colorRect);
        }
    }
    DisFFTbuff.pushSprite(4, 92);
}


void microPhoneSetup() {
    fftvalueQueue = xQueueCreate(5, 24 * sizeof(uint8_t));
    if( fftvalueQueue == 0 ) return;

    i2sstateQueue = xQueueCreate(5, sizeof(i2sQueueMsg_t));
    if( i2sstateQueue == 0 ) return;

    InitI2SSpeakerOrMic(MODE_MIC);
    xTaskCreatePinnedToCore(i2sMicroFFTtask, "microPhoneTask", 4096, NULL, 3, NULL, 0);
    DisFFTbuff.createSprite(312, 128);
}


void setup() {
  sys.begin("SoundMon");
  M5.Lcd.setTextFont(2);
  M5.Lcd.setTextSize(1);
  M5.Axp.SetSpkEnable(false);
  InitI2SSpeakerOrMic(MODE_MIC);
  microPhoneSetup();
}


void loop() {
  MicroPhoneFFT();
  delay(5);
}