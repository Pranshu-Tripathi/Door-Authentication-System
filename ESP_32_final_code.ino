#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"               // SD Card ESP32
#include "SD_MMC.h"           // SD Card ESP32
#include "soc/soc.h"          // Disable brownour problems
#include "soc/rtc_cntl_reg.h" // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h> // read and write from flash memory
#include "SD.h"
#include "ESP32_MailClient.h"
#include "SPIFFS.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "driver/rtc_io.h"

#include <WiFi.h>

int led = 13;
int red = 14;
int internal_led = 33;
const char *ssid = "PHMT";
const char *password = "tlmt9452";

#define emailSenderAccount "archits725@gmail.com"
#define emailSenderPassword "qwerty123@"
#define emailRecipient "mtpranshu2001@gmail.com"
#define smtpServer "smtp.gmail.com"
#define smtpServerPort 465 // 587 //465
#define emailSubject "URGENT : INTRUDER ALERT!"

SMTPData smtpData;

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define TIME_TO_SLEEP 10          // time ESP32 will go to sleep (in seconds)
#define uS_TO_S_FACTOR 1000000ULL // conversion factor for micro seconds to seconds */

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    pinMode(led, OUTPUT);
    pinMode(red, INPUT);
    pinMode(internal_led, OUTPUT);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    digitalWrite(internal_led, LOW);
    digitalWrite(red, LOW);
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");

    delay(500);
    Serial.println("WELLCOME");
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        ESP.restart();
    }
    else
    {
        Serial.println("SPIFFS mounted successfully");
    }
    SPIFFS.format();

    EEPROM.begin(400);

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // gpio_pullup_en(GPIO_NUM_12);  // ajout
    //  Pin(14, Pin.IN, Pin.PULL_UP)

    if (psramFound())
    {
        config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    // Init Camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }else {
        Serial.println("Setup completed!");
    }
}

void loop()
{
    //delay(4000);
    // put your main code here, to run repeatedly:
    if (digitalRead(red) == HIGH)
    {
        Serial.println("\nRecieved Signal\n");
        //aptureAndSendImage();
        captureImage();
        buzz();
        sendEmail();
    }
    //else{
//      Serial.println("No signal");
//    }
    
}

void buzz(){
    
        for(int i=0;i<20;i++){
           // Serial.println("INPUT");
        digitalWrite(led, HIGH); //Turn on
        delay (750); //Wait 1 sec
        digitalWrite(led, LOW); //Turn off
        delay (750); //Wait 1 sec
        //Serial.println("blynk");
        }
}

void sendCallback(SendStatus msg)
{
    // Print the current status
    Serial.println(msg.info());

    // Do something when complete
    if (msg.success())
    {
        Serial.println("----------------");
    }
}

void captureImage()
{
    camera_fb_t *fb = NULL;
    digitalWrite(4, HIGH);

    // Take Picture with Camera
    fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Camera capture failed");
        return;
    }
    String path = "/Sight.jpg";
    File file = SPIFFS.open(path, FILE_WRITE);
    // Path where new picture will be saved in SD Card

    if (!file)
    {
        Serial.println("Failed to open file in writing mode");
    }
    else
    {
        file.write(fb->buf, fb->len); // payload (image), payload length
        Serial.printf("Saved file to path: %s\n", path);
        //    EEPROM.write(0, pictureNumber);      // Not used
        //    EEPROM.commit();
    }
    file.close();
    esp_camera_fb_return(fb);

    // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);
    rtc_gpio_hold_en(GPIO_NUM_4);
    //*************************************************************************************************************************

   
}

void sendEmail() {
     Serial.println("Preparing to send email");
    Serial.println();

    // Set the SMTP Server Email host, port, account and password
    smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

    // For library version 1.2.0 and later which STARTTLS protocol was supported,the STARTTLS will be
    // enabled automatically when port 587 was used, or enable it manually using setSTARTTLS function.
    // smtpData.setSTARTTLS(true);

    // Set the sender name and Email
    smtpData.setSender("Door Authentication", emailSenderAccount);

    // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
    smtpData.setPriority("High");

    // Set the subject
    smtpData.setSubject(emailSubject);

    // Set the message with HTML format
    smtpData.setMessage("<div style=\"color:#2f4468;\"><h1>Find a thing !</h1><p>- Sent from ESP32 board</p></div>", true);
    // Set the email message in text format (raw)
    // smtpData.setMessage("Thing on position #00", false);

    // Add recipients, you can add more than one recipient
    smtpData.addRecipient(emailRecipient);
    // smtpData.addRecipient("YOUR_OTHER_RECIPIENT_EMAIL_ADDRESS@EXAMPLE.com");
    smtpData.setFileStorageType(MailClientStorageType::SPIFFS);
    smtpData.addAttachFile("/Sight.jpg");

    // smtpData.setFileStorageType(MailClientStorageType::SD);

    smtpData.setSendCallback(sendCallback);

    // Start sending Email, can be set callback function to track the status
    if (!MailClient.sendMail(smtpData))
        Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    else
        Serial.println("----------------Mail Sent-----------");

    // Clear all data from Email object to free memory
    smtpData.empty();
    //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    //esp_deep_sleep_start();
}
