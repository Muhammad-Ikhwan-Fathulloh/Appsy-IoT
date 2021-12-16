////////////////////////////////////////////////////
// Proyek Jaringan Komputer Internet of Things ////
// Kelompok Appsy ////////////////////////////////
/////////////////////////////////////////////////

/*
 * ----------------------------------
 *             MFRC522      Node     
 *             Reader/PCD   MCU      
 * Signal      Pin          Pin      
 * ----------------------------------
 * RST/Reset   RST          D1 (GPIO5)        
 * SPI SS      SDA(SS)      D2 (GPIO4)       
 * SPI MOSI    MOSI         D7 (GPIO13)
 * SPI MISO    MISO         D6 (GPIO12)
 * SPI SCK     SCK          D5 (GPIO14)
 * 3.3V        3.3V         3.3V
 * GND         GND          GND
 */

//Library ESP8266
#include <ESP8266WiFi.h>

//Library Database Realtime Firebase
#include <FirebaseArduino.h>

//Library RFID
#include <SPI.h>//Serial Peripheral Interface
#include <MFRC522.h>

//Library LCD Display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//Library NTP (Network Time Protocol)
#include <NTPClient.h>//Server NTP sebagai Client Wemos D1 R1
#include <WiFiUdp.h>//Port UDP ke Server NTP

//Token Database Firebase
#define FIREBASE_HOST "appsy-298f0-default-rtdb.firebaseio.com" //https://appsy-298f0-default-rtdb.firebaseio.com/ 
#define FIREBASE_AUTH "ymRddxgK1ZSSkJjJh13TJCB9ATZCTUZ2BY7INJoN"

//Koneksi Wifi
#define WIFI_SSID "ICE.OFFICIAL"
#define WIFI_PASSWORD "elektro.indo" 

//Pengaturan pin data dan reset RFID
constexpr uint8_t RST_PIN = 2;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 15;     // Configurable, see typical pin layout above

//RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

int readsuccess;
byte readcard[4];
char str[32] = "";
String StrUID;

//LCD Display
LiquidCrystal_I2C lcd(0x27,16,2);//SDA dan SDL

//NTP (Network Time Protocol)
/*
UTC +07.00 : 7 * 60 * 60 : 25200 Waktu Indonesia Barat (WIB)
UTC +08.00 : 8 * 60 * 60 : 28800 Waktu Indonesia Tengah (WITA)
UTC +09.00 : 9 * 60 * 60 : 32400 Waktu Indonesia Timur (WIT)
*/

const long utcOffsetInSeconds = 25200;  // set offset

char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};  // Indonesian
//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};  // English

// Define NTP Client to get time
/*
Area_____________________________________________________HostName
Worldwide_______________________________________________ pool.ntp.org
Asia____________________________________________________ asia.pool.ntp.org
Europe__________________________________________________ europe.pool.ntp.org
North America___________________________________________ north-america.pool.ntp.org
Oceania_________________________________________________ oceania.pool.ntp.org
South America___________________________________________ south-america.pool.ntp.org
*/

WiFiUDP ntpUDP;// port UDP untuk mengambil data di Network Time Protokol
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", utcOffsetInSeconds);//Dari Server NTP sebagai Client pada Wemos D1 R1

//Mode
int mode1, mode2, mode3, mode4;

//Pengaturan device
void setup() { 
  //Baud Rate ESP8266
  Serial.begin(115200);
  
  //Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  //RFID
  SPI.begin(); // Inisialisasi SPI bus
  mfrc522.PCD_Init(); // Inisialisasi MFRC522
  
  //LCD Display
  lcd.begin();
  lcd.home();
  //lcd.init();                      
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("By Appsy ");
  lcd.setCursor(0,1);
  lcd.print("Koneksikan Alat");
  
  //Menunggu Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin (WIFI_SSID, WIFI_PASSWORD);
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Cetak Lokal IP Address Wemos ESP8266
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: http://");
  Serial.println(WiFi.localIP());//Membaca Alamat IP Lokal
  
  Serial.println("");
  Serial.println("WiFi Connected!");
  
  //NTP (Network Time Protocol)
  timeClient.begin();
  
  Serial.println("Tap Kartu");

  //Mode
  Firebase.setInt("mode/mode1",0);    
  Firebase.setInt("mode/mode2",0);    
  Firebase.setInt("mode/mode3",0);
  Firebase.setInt("mode/mode4",0);                    
}

//Kontrol berulang
void loop() { 
  //NTP (Network Time Protocol)
  timeClient.update();

  //Mode
  mode1=Firebase.getString("mode/mode1").toInt();//Daftar Kartu
  mode2=Firebase.getString("mode/mode2").toInt();//Mode Istirahat
  mode3=Firebase.getString("mode/mode3").toInt();//Pinjam Buku
  mode4=Firebase.getString("mode/mode4").toInt();//Informasi 
  
  if(mode2 == 0){
    if(mode3 == 0){
      if(mode1 == 0){
      //LCD Display
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(daysOfTheWeek[timeClient.getDay()]);
      lcd.print(", ");
      lcd.print(timeClient.getFormattedTime());
      lcd.setCursor(0,1);
      lcd.print("Dekatkan Kartu!");
    
      //Sukses pembacaan id
      readsuccess = getid();
     
      if(readsuccess){
        //Cetak pembacaan id
        Serial.println(StrUID);
        Serial.println("Berhasil"); 
        
        //LCD Display
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ID : ");
        lcd.print(StrUID);
        lcd.setCursor(0,1);
        lcd.print("Menunggu data...");
     
        //Firebase.setString ("id/rfid", StrUID);
    
        //Ubah tipe data id menjadi string
        String id = StrUID;
        delay(500);
        //Mengambil id yang terdaftar
        Serial.println(Firebase.getString("peserta/" + id));
    
        //Data User dari Database
        String id_user = Firebase.getString("peserta/" + id + "/id");
        String fullname_user = Firebase.getString("peserta/" + id + "/fullname");
        String username_user = Firebase.getString("peserta/" + id + "/username");
        String alamat_user = Firebase.getString("peserta/" + id + "/alamat");
        String nohp_user = Firebase.getString("peserta/" + id + "/nohp");
        String pekerjaan_user = Firebase.getString("peserta/" + id + "/pekerjaan");
        String tanggal_user = Firebase.getString("peserta/" + id + "/tanggal");
        String berlaku_user = Firebase.getString("peserta/" + id + "/berlaku");
        
        Serial.println("Data dari Firebase : ");
        Serial.println("");
        
        if(id == id_user.substring(1,9)){ 
          //Status User
          Serial.println("Status : Terdaftar");
          Serial.print("");
          Serial.println("Data User Terdaftar : ");
          Serial.print("ID : ");
          Serial.println(id_user);
          Serial.print("Fullname : ");
          Serial.println(fullname_user);
          Serial.print("Username : ");
          Serial.println(username_user);
          Serial.print("Alamat : ");
          Serial.println(alamat_user);
          Serial.print("No HP : ");
          Serial.println(nohp_user);
          Serial.print("Pekerjaan : ");
          Serial.println(pekerjaan_user);
          Serial.print("Tanggal Daftar : ");
          Serial.println(tanggal_user);
          Serial.print("Masa berlaku : ");
          Serial.println(berlaku_user);
          Serial.println("");
    
          //LCD Display
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Selamat Datang");
          lcd.setCursor(0,1);
          lcd.print(username_user);
          delay(2000);
    
          //LCD Display
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Berlaku sampai :");
          lcd.setCursor(0,1);
          lcd.print(berlaku_user);
          delay(2000);
        } 
        else{
          //Status User
          Serial.println("Status : Belum Terdaftar");
          Serial.print("");
          Serial.print("Silahkan Daftarkan Kartu : ");
          Serial.println(id);
          Serial.println("");
    
          //LCD Display
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Belum Terdaftar");
          lcd.setCursor(0,1);
          lcd.print("Silahkan Daftar");
          delay(2000);
        }
      }
    }
    if(mode1 == 1){
      //LCD Display
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(daysOfTheWeek[timeClient.getDay()]);
      lcd.print(", ");
      lcd.print(timeClient.getFormattedTime());
      lcd.setCursor(0,1);
      lcd.print("Daftarkan Kartu!");
      delay(500);
  
      //Sukses pembacaan id
      readsuccess = getid();
     
      if(readsuccess){
        //Cetak pembacaan id
        Serial.println(StrUID);
        Serial.println("Berhasil");  
        Firebase.setString ("id/rfid", StrUID);
    
        //Ubah tipe data id menjadi string
        String id = StrUID;
  
        //Mengambil data id dari database
        String id_user = Firebase.getString("peserta/" + id + "/id");
        String username_user = Firebase.getString("peserta/" + id + "/username");
  
        if(id == id_user.substring(1,9)){ 
          //LCD Display
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Sudah Terdaftar!");
          lcd.setCursor(0,1);
          lcd.print(username_user);
          delay(2000);
        }
      }
    }
    }
    if(mode3 == 1){
      //LCD Display
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(daysOfTheWeek[timeClient.getDay()]);
      lcd.print(", ");
      lcd.print(timeClient.getFormattedTime());
      lcd.setCursor(0,1);
      lcd.print("Pinjam Buku!");
      delay(500);
  
      //Sukses pembacaan id
      readsuccess = getid();
     
      if(readsuccess){
        //Cetak pembacaan id
        Serial.println(StrUID);
        Serial.println("Berhasil");  
    
        //Ubah tipe data id menjadi string
        String id = StrUID;
  
        //Mengambil data id dari database
        String id_user = Firebase.getString("peserta/" + id + "/id");
        String username_user = Firebase.getString("peserta/" + id + "/username");
  
        String id_peminjam = Firebase.getString("peminjaman/" + id);
        Serial.println(id_peminjam);
  
        if(id == id_user.substring(1,9)){ 
          //Kirim ID jika sudah terdaftar sebagai anggota
          Firebase.setString ("id/rfid", StrUID);
          
          //LCD Display
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(username_user);
          lcd.setCursor(0,1);
          lcd.print("Pinjam Buku?");
          delay(2000);
        }
        else{
          //LCD Display
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Belum Terdaftar");
          lcd.setCursor(0,1);
          lcd.print("Daftar dulu");
          delay(2000);
        }
      }
    }
  }
  if(mode2 == 1){
    if(mode4 == 0){
      //LCD Display
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(daysOfTheWeek[timeClient.getDay()]);
      lcd.print(", ");
      lcd.print(timeClient.getFormattedTime());
      lcd.setCursor(0,1);
      lcd.print("Waktu Istirahat");
    }
    if(mode4 == 1){
      //Informasi
      String informasi1 = Firebase.getString("informasi/info1");
      String informasi2 = Firebase.getString("informasi/info2");
      
      //LCD Display
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(informasi1);
      lcd.setCursor(0,1);
      lcd.print(informasi2);
//      for(int s = 0; s < 20; s++){
//        lcd.scrollDisplayLeft();
//        delay(250);
//      }
    }
  }
}

//Ambil id untuk verifikasi
int getid(){ 
  if(!mfrc522.PICC_IsNewCardPresent()){
    return 0;
  }
  if(!mfrc522.PICC_ReadCardSerial()){
    return 0;
  }
  
  Serial.println("Data Kartu:");
  
  for(int i=0;i<4;i++){
    readcard[i]=mfrc522.uid.uidByte[i]; //Kirim id card yang dibaca
    array_to_string(readcard, 4, str);
    StrUID = str;// id yang telah diolah
  }
  mfrc522.PICC_HaltA();
  return 1;
}
// --------------------------------------------------------------------
//Mengolah frekuensi tag kartu
void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}
