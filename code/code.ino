#include <Servo2.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define MOTOR_A_a 3      // 모터 A의 +출력 핀
#define MOTOR_A_b 11     // 모터 A의 -출력 핀
#define MOTOR_B_a 5      // 모터 B의 +출력 핀
#define MOTOR_B_b 6      // 모터 B의 -출력 핀
#define MOTOR_SPEED 100  // 모터 기준 속력
#define LINESENS_L 7     // 왼쪽 라인 센서 입력 핀
#define LINESENS_R 8     // 오른쪽 라인 센서 입력 핀

#define TAIL_SERVO_PIN 9     // 꼬리 서보 모터 연결 핀
#define HEAD_SERVO_PIN 10    // 머리 서보 모터 연결 핀

#define SERVO_SPEED 1        // 서보 모터 회전 속도
#define TAIL_ANGLE_MIN 50    // 꼬리 서보 회전 최소값
#define TAIL_ANGLE_MAX 130   // 꼬리 서보 회전 최대값
#define HEAD_ANGLE_MIN 50    // 머리 서보 회전 최소값
#define HEAD_ANGLE_MAX 130   // 머리 서보 회전 최대값

#define DF_PLAYER_RX A1      // MP3 모듈 RX 핀
#define DF_PLAYER_TX A2      // MP3 모듈 TX 핀

#define MP3_DUCK_FORWARD 1  // 전진 시 재생할 MP3 번호
#define MP3_DUCK_LEFT 2     // 좌회전 시 재생할 MP3 번호
#define MP3_DUCK_RIGHT 3    // 우회전 시 재생할 MP3 번호
#define MP3_DUCK_STOP 4     // 정지 시 재생할 MP3 번호

#define LED_R_PIN 4
#define LED_G_PIN 13
#define LED_B_PIN 12

unsigned char m_a_spd = 0, m_b_spd = 0; // 모터 속력 변수
boolean m_a_dir = 0, m_b_dir = 0;       // 모터 방향 변수
boolean line_l = 0;
boolean line_r = 0;

int head_servo_angle = 90;              // 머리 서보 각도 변수
int tail_servo_angle = 90;              // 꼬리 서보 각도 변수

Servo head_servo;
Servo tail_servo;

SoftwareSerial softwareSerial(DF_PLAYER_RX, DF_PLAYER_TX);
DFRobotDFPlayerMini dfPlayer;

int before_state = 0;
unsigned long servoTime;

void setup()  
{
  TCCR1B = TCCR1B & 0b11111000 | 0x05;  //저속주행이 가능하도록 모터A PWM 주파수 변경
  TCCR2B = TCCR2B & 0b11111000 | 0x07;  //저속주행이 가능하도록 모터B PWM 주파수 변경
  
  softwareSerial.begin(9600);
  // 
  dfPlayer.begin(softwareSerial);
  dfPlayer.volume(50);

  // 모터 제어 핀 출력 설정
  pinMode(MOTOR_A_a, OUTPUT);
  pinMode(MOTOR_A_b, OUTPUT);
  pinMode(MOTOR_B_a, OUTPUT);
  pinMode(MOTOR_B_b, OUTPUT);

  // 머리, 꼬리 서보 모터 연결
  tail_servo.attach(TAIL_SERVO_PIN);
  head_servo.attach(HEAD_SERVO_PIN);

  // 머리, 꼬리 서보 각도 초기화 ( 90도 )
  tail_servo.write(tail_servo_angle);
  head_servo.write(head_servo_angle);

  // 라인 센서 핀 입력 설정
  pinMode(LINESENS_L, INPUT);
  pinMode(LINESENS_R, INPUT);

  // 눈 LED 핀 출력 설정
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);

  delay(500);

  servoTime = millis();
}

void loop()
{
  line_l = digitalRead(LINESENS_L);
  line_r = digitalRead(LINESENS_R);


  if (line_l == 0 && line_r == 0) {
    // dfPlayer.play(1);
    while (line_l == 0 && line_r == 0) {
      digitalWrite(MOTOR_A_a, LOW);     //모터A+ LOW
      analogWrite(MOTOR_A_b, 100);  //모터A-의 속력을 PWM 출력

      analogWrite(MOTOR_B_a, 125);  //모터B+의 속력을 PWM 출력
      digitalWrite(MOTOR_B_b, LOW);

      if (tail_servo_angle < 90) {
        tail_servo_angle = constrain(tail_servo_angle + SERVO_SPEED, TAIL_ANGLE_MIN, 90);
      } else if (tail_servo_angle > 90) {
        tail_servo_angle = constrain(tail_servo_angle - SERVO_SPEED, 90, TAIL_ANGLE_MAX);
      }

      if (head_servo_angle < 90) {  
        head_servo_angle = constrain(head_servo_angle + SERVO_SPEED, HEAD_ANGLE_MIN, 90);
      } else if (head_servo_angle > 90) {
        head_servo_angle = constrain(head_servo_angle - SERVO_SPEED, 90, HEAD_ANGLE_MAX);
      }

      led_red();

      line_l = digitalRead(LINESENS_L);
      line_r = digitalRead(LINESENS_R);
      // delay(50);
    }
  }

  else if (line_l == 1 && line_r == 0) {
    while (line_l == 1 && line_r == 0) {
      analogWrite(MOTOR_A_a, 160);  //모터A+의 속력을 PWM 출력
      digitalWrite(MOTOR_A_b, LOW);     //모터A- LOW

      analogWrite(MOTOR_B_a, 180);  //모터B+의 속력을 PWM 출력
      digitalWrite(MOTOR_B_b, LOW);

      tail_servo_angle = constrain(tail_servo_angle + SERVO_SPEED, TAIL_ANGLE_MIN, TAIL_ANGLE_MAX);
      head_servo_angle = constrain(head_servo_angle + SERVO_SPEED, HEAD_ANGLE_MIN, HEAD_ANGLE_MAX);

      line_l = digitalRead(LINESENS_L);
      line_r = digitalRead(LINESENS_R);

      led_green();
      // dfPlayer.play(2);
      // delay(1);

      // delay(50);

    }
  }

  else if (line_l == 0 && line_r == 1) {
    while (line_l == 0 && line_r == 1) {
      digitalWrite(MOTOR_A_a, LOW);
      analogWrite(MOTOR_A_b, 160);

      digitalWrite(MOTOR_B_a, LOW);
      analogWrite(MOTOR_B_b, 180);

      tail_servo_angle = constrain(tail_servo_angle - SERVO_SPEED, TAIL_ANGLE_MIN, TAIL_ANGLE_MAX);
      head_servo_angle = constrain(head_servo_angle - SERVO_SPEED, HEAD_ANGLE_MIN, HEAD_ANGLE_MAX);

      line_l = digitalRead(LINESENS_L);
      line_r = digitalRead(LINESENS_R);

      led_blue();

      // delay(50);

    }
  }

  else {
    digitalWrite(MOTOR_A_a, LOW);     //모터A+ LOW
    analogWrite(MOTOR_A_b, 0);  //모터A-의 속력을 PWM 출력

    analogWrite(MOTOR_B_a, 0);  //모터B+의 속력을 PWM 출력
    digitalWrite(MOTOR_B_b, LOW);

    tail_servo_angle = 90;
    head_servo_angle = 90;

    led_white();

    delay(1000);

    digitalWrite(MOTOR_A_a, LOW);
    analogWrite(MOTOR_A_b, 160);

    digitalWrite(MOTOR_B_a, LOW);
    analogWrite(MOTOR_B_b, 80);
    delay(300);

    while (line_l == 1 && line_r == 1) {
      digitalWrite(MOTOR_A_a, LOW);
      analogWrite(MOTOR_A_b, 160);

      digitalWrite(MOTOR_B_a, LOW);
      analogWrite(MOTOR_B_b, 160);

      line_l = digitalRead(LINESENS_L);
      line_r = digitalRead(LINESENS_R);

      digitalWrite(LED_R_PIN, HIGH);
      digitalWrite(LED_G_PIN, HIGH);
      digitalWrite(LED_B_PIN, HIGH);
    }
  
  }

  // digitalWrite(MOTOR_A_a, LOW);     //모터A+ LOW
  // analogWrite(MOTOR_A_b, 0);  //모터A-의 속력을 PWM 출력
// 
  // analogWrite(MOTOR_B_a, 0);  //모터B+의 속력을 PWM 출력
  // digitalWrite(MOTOR_B_b, LOW);
  // delay(10);
  
  if (millis() - servoTime > 1000) {
    dfPlayer.play(1);
    tail_servo.write(tail_servo_angle);
    head_servo.write(head_servo_angle);
    servoTime = millis();
  }
}

void led_red() {
  digitalWrite(LED_R_PIN, LOW);
  digitalWrite(LED_G_PIN, HIGH);
  digitalWrite(LED_B_PIN, HIGH);
}

void led_green() {
  digitalWrite(LED_R_PIN, HIGH);
  digitalWrite(LED_G_PIN, LOW);
  digitalWrite(LED_B_PIN, HIGH);
}

void led_blue() {
  digitalWrite(LED_R_PIN, HIGH);
  digitalWrite(LED_G_PIN, HIGH);
  digitalWrite(LED_B_PIN, LOW);
}

void led_white() {
  digitalWrite(LED_R_PIN, LOW);
  digitalWrite(LED_G_PIN, LOW);
  digitalWrite(LED_B_PIN, LOW);
}