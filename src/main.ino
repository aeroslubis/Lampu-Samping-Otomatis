#include <TimerOne.h>      // Library timer untuk blink lampu
#include <MPU6050_tockn.h> // Library MPU-6050
#include <Wire.h>          // Library pembantu I2C untuk MPU-6050

int lampu_kanan = 7;   // lampu kanan
int lampu_kiri = 6;    // lampu kiri
int button_kanan = 10; // Tombol belok kanan
int button_kiri = 12;  // Tombol belok kiri
int button_stop = 11;  // Tombol stop
int alaram = 5;        // Buzzer

bool alaram_aktif = false;
float angleZ;
int lampu_hidup;

MPU6050 mpu6050(Wire); // Aktifkan sensor MPU-6050

void setup() {
    // Setting pin-pin lampu dan buzzer sebagai output
    pinMode(lampu_kanan, OUTPUT);
    pinMode(lampu_kiri, OUTPUT);
    pinMode(alaram, OUTPUT);

    // Setting pin-pin push button sebagai input
    pinMode(button_kanan, INPUT_PULLUP);
    pinMode(button_kiri, INPUT_PULLUP);
    pinMode(button_stop, INPUT_PULLUP);

    Timer1.initialize(300000); // Inisialisasi timer untuk blink lampu kiri dan kanan

    Serial.begin(115200);
    Wire.begin();
    mpu6050.begin(); // Hidupkan sensor MPU-6050
    mpu6050.calcGyroOffsets(); // Kalkulasi offset sudut sepeda motor

    // Idikasi untuk pengguna sepeda motor bahwasanya alat siap digunakan
    tone(alaram, 2000);
    delay(500);
    noTone(alaram);
}

void loop() {
    // Jika tombol kanan ditekan maka panggil fungsi belok()
    if (digitalRead(button_kanan) == LOW) {
        belok(true); // true untuk belok kanan
    }
    // Jika tombol kanan ditekan maka panggil fungsi belok()
    else if (digitalRead(button_kiri) == LOW) { // Jika tombol kiri ditekan
        belok(false); // false untuk belok kiri
    }
}

// Fungsi untuk menghidupkan lampu dan alaram
void hidupkanLed() {
    bool kondisi_lampu = digitalRead(lampu_hidup); // baca kondisi lampu saat ini
    if (kondisi_lampu == HIGH) { // jika kondisi lampu hidup maka matikan lampu
        digitalWrite(lampu_hidup, LOW);
        if (alaram_aktif == true) { // matikan alaram
            noTone(alaram);
        }
    } else { // jika kondisi lampu mati maka hidupkan lampu
        if (alaram_aktif == true) { // hidupkan alaram
            tone(alaram, 2000);
        }
        digitalWrite(lampu_hidup, HIGH);
    }
}

// Fungsi untuk mematikan lampu dan alaram
void matikanLampu() {
    Timer1.detachInterrupt();
    digitalWrite(lampu_kiri, LOW);
    digitalWrite(lampu_kanan, LOW);
    noTone(alaram); // Matikan alaram
    alaram_aktif = false;
}

// Funsi pada saat push button di tekan
void belok(bool arah) {
    // Variabel untuk menghitung mundur batas waktu sebelum lampu samping dimatikan
    unsigned long timeout = 0;
    unsigned long currentMillis = 0;
    unsigned long previousMillis = millis();

    mpu6050.update(); // Ambil data dari sensor mpu-6050
    angleZ = mpu6050.getAngleZ();

    if (arah == true) {
        lampu_hidup = lampu_kanan; // Hidupkan lampu sebelah kanan
        Serial.println("Belok ke arah kanan");
    } else {
        lampu_hidup = lampu_kiri; // Hidupkan lampu sebelah kiri
        Serial.println("Belok ke arah kiri");
    }

    Timer1.attachInterrupt(hidupkanLed);

    // Atur batas waktu untuk mematikan lampu samping ke 10 detik
    while (timeout < 10000) {
        mpu6050.update(); // Ambil data dari sensor mpu-6050
        /* Serial.print("Z : "); Serial.print(mpu6050.getAngleZ()); */
        /* Serial.print("\tZn : "); Serial.println(mpu6050.getAngleZ() - angleZ); */

        // Apabila lampu yang dihidupkan oleh pengguna adalah lampu sebelah kiri
        // Jika sepeda motor bergerak melebihi sudut 8 derajat atau bergerak ke arah kanan
        // Maka perbaiki arah hidup lampu samping yaitu hidupkan lampu samping sebelah kanan
        if (mpu6050.getAngleZ() - angleZ >= 8) {
            digitalWrite(lampu_kanan, LOW);
            lampu_hidup = lampu_kiri; // Hidupkan lampu sebelah kiri

            if (arah == true) { // Jika arah belok tidak sama dengan tombol lampu sampin hidupkan alaram
                alaram_aktif = true;
            } else { // Matikan alaram
                alaram_aktif = false;
                noTone(alaram);
            }
        }
        // Apabila lampu yang dihidupkan oleh pengguna adalah lampu sebelah kanan
        // Jika sepeda motor bergerak melebihi sudut -8 derajat atau bergerak ke arah kiri
        // Maka perbaiki arah hidup lampu samping yaitu hidupkan lampu samping sebelah kiri
        else if (mpu6050.getAngleZ() - angleZ <= -8) { // Jika bergerak ke arah kanan
            digitalWrite(lampu_kiri, LOW);
            lampu_hidup = lampu_kanan; // Hidupkan lampu sebelah kanan

            if (arah == false) { // Jika arah belok tidak sama dengan tombol lampu samping hidupkan alaram
                alaram_aktif = true;
            } else { // Matikan alaram
                alaram_aktif = false;
                noTone(alaram);
            }
        }

        if (digitalRead(button_stop) == LOW) { // Matikan lampu samping jikan tombol stop ditekan
            break;
        }

        // Kalkulasi batas timeout untuk mematikan lampu
        currentMillis = millis();
        timeout = timeout + (currentMillis - previousMillis);
        previousMillis = currentMillis;
        delayMicroseconds(20);
    }
    timeout = 0;
    Serial.println("lampu mati");

    matikanLampu(); // Matikan lampu samping
}
