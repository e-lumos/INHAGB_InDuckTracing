#include <Servo.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define MOTOR_A_a 3      // 모터 A의 +출력 핀
#define MOTOR_A_b 11     // 모터 A의 -출력 핀
#define MOTOR_B_a 5      // 모터 B의 +출력 핀
#define MOTOR_B_b 6      // 모터 B의 -출력 핀
#define MOTOR_SPEED 255  // 모터 기준 속력
#define LINESENS_L 7     // 왼쪽 라인 센서 입력 핀
#define LINESENS_R 8     // 오른쪽 라인 센서 입력 핀

#define TAIL_SERVO_PIN 9     // 꼬리 서보 모터 연결 핀
#define HEAD_SERVO_PIN 10    // 머리 서보 모터 연결 핀

#define SERVO_SPEED 5        // 서보 모터 회전 속도
#define TAIL_ANGLE_MIN 30    // 꼬리 서보 회전 최소값
#define TAIL_ANGLE_MAX 150   // 꼬리 서보 회전 최대값
#define HEAD_ANGLE_MIN 30    // 머리 서보 회전 최소값
#define HEAD_ANGLE_MAX 150   // 머리 서보 회전 최대값

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

int head_servo_angle = 90;              // 머리 서보 각도 변수
int tail_servo_angle = 90;              // 꼬리 서보 각도 변수

Servo head_servo;
Servo tail_servo;

SoftwareSerial softwareSerial(DF_PLAYER_RX, DF_PLAYER_TX);
DFRobotDFPlayerMini dfPlayer;

void setup()  
{
  TCCR1B = TCCR1B & 0b11111000 | 0x05;  //저속주행이 가능하도록 모터A PWM 주파수 변경
  TCCR2B = TCCR2B & 0b11111000 | 0x07;  //저속주행이 가능하도록 모터B PWM 주파수 변경
  
  softwareSerial.begin(9600);
  dfPlayer.volume(20);

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
}

void loop()
{
  linetrace_val();  //입력된 데이터에 따라 모터에 입력될 변수를 조정하는 함수

  tail_servo.write(tail_servo_angle);
  head_servo.write(head_servo_angle);

  motor_drive();    //모터를 구동하는 함수
}

void linetrace_val() //입력된 데이터에 따라 모터에 입력될 변수를 조정하는 함수
{
  boolean line_l = digitalRead(LINESENS_L), line_r = digitalRead(LINESENS_R); //왼쪽과 오른쪽 라인센서의 감지값을 변수에 저장합니다.
  
  if(line_l == 0 && line_r == 0)    // L, R Sensor 감지 X -> 전진
  {
    m_a_dir = 0;  //모터A 정방향
    m_b_dir = 0;  //모터B 정방향
    m_a_spd = MOTOR_SPEED;  //모터A의 속력값 조정
    m_b_spd = MOTOR_SPEED;  //모터B의 속력값 조정

    // TAIL, HEAD 서보 모터 90도 위치로 이동
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

    dfPlayer.play(MP3_DUCK_FORWARD);

    // TODO : LED 수정 아이디어
    led_red();
  }

  else if(line_l == 1 && line_r == 0) // L Sensor 감지시 왼쪽 회전
  {
    m_a_dir = 1;  //모터A 역방향
    m_b_dir = 0;  //모터B 정방향
    m_a_spd = 0;  //모터A의 정지
    m_b_spd = constrain(MOTOR_SPEED*2, 0, 255);  //모터B의 속력값 조정

    tail_servo_angle = constrain(tail_servo_angle + SERVO_SPEED, TAIL_ANGLE_MIN, TAIL_ANGLE_MAX);
    head_servo_angle = constrain(head_servo_angle + SERVO_SPEED, HEAD_ANGLE_MIN, HEAD_ANGLE_MAX);

    dfPlayer.play(MP3_DUCK_LEFT);

    // TODO : LED 수정 아이디어
    led_blue();
  }

  else if(line_l == 0 && line_r == 1)  //오른쪽 센서 감지시 오른쪽으로 전진
  {
    m_a_dir = 0;  //모터A 정방향
    m_b_dir = 1;  //모터B 역방향
    m_a_spd = constrain(MOTOR_SPEED*2, 0, 255);  //모터A의 속력값 조정
    m_b_spd = 0;  //모터B의 정지

    tail_servo_angle = constrain(tail_servo_angle - SERVO_SPEED, TAIL_ANGLE_MIN, TAIL_ANGLE_MAX);
    head_servo_angle = constrain(head_servo_angle - SERVO_SPEED, HEAD_ANGLE_MIN, HEAD_ANGLE_MAX);

    dfPlayer.play(MP3_DUCK_RIGHT);

    // TODO : LED 수정 아이디어
    led_green();
  }
  
  else
  {
    m_a_dir = 0;  // 모터A 정방향
    m_b_dir = 0;  // 모터B 정방향
    m_a_spd = 0;  // 모터A 정지
    m_b_spd = 0;  // 모터B 정지

    tail_servo_angle = 0;
    head_servo_angle = 0;

    dfPlayer.play(MP3_DUCK_STOP);

    // TODO : 정지 시 LED 상태
    led_white();
  }
}

void motor_drive()  //모터를 구동하는 함수
{
  if(m_a_dir == 0)
  {
    digitalWrite(MOTOR_A_a, LOW);     //모터A+ LOW
    analogWrite(MOTOR_A_b, m_a_spd);  //모터A-의 속력을 PWM 출력
  }
  else
  {
    analogWrite(MOTOR_A_a, m_a_spd);  //모터A+의 속력을 PWM 출력
    digitalWrite(MOTOR_A_b, LOW);     //모터A- LOW
  }
  if(m_b_dir == 1)
  {
    digitalWrite(MOTOR_B_a, LOW);     //모터B+ LOW
    analogWrite(MOTOR_B_b, m_b_spd);  //모터B-의 속력을 PWM 출력
  }
  else
  {
    analogWrite(MOTOR_B_a, m_b_spd);  //모터B+의 속력을 PWM 출력
    digitalWrite(MOTOR_B_b, LOW);     //모터B- LOW
  }
}

void led_red() {
  digitalWrite(LED_R_PIN, HIGH);
  digitalWrite(LED_G_PIN, LOW);
  digitalWrite(LED_B_PIN, LOW);
}

void led_green() {
  digitalWrite(LED_R_PIN, LOW);
  digitalWrite(LED_G_PIN, HIGH);
  digitalWrite(LED_B_PIN, LOW);
}

void led_blue() {
  digitalWrite(LED_R_PIN, LOW);
  digitalWrite(LED_G_PIN, LOW);
  digitalWrite(LED_B_PIN, HIGH);
}

void led_white() {
  digitalWrite(LED_R_PIN, HIGH);
  digitalWrite(LED_G_PIN, HIGH);
  digitalWrite(LED_B_PIN, HIGH);
}