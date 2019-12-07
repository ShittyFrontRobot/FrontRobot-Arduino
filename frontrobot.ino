#include <Arduino.h>


enum MotorState {
    Stop, Break, Speed
};

template<typename T>
class Controller {
public:
    virtual double operator()(T data) = 0;
};

class PID : Controller<double> {
private:
    double sum{}, last{};
public:
    double kp{}, ki{}, kd{}, integrate_area{}, dead_area{};

    double operator()(double data) override {
        auto value = abs(data);
        if (value < dead_area) {
            sum = .0;
            return .0;
        }
        if (value > integrate_area)
            sum = .0;
        else sum += data;
        auto result = data + kd * (data - last) + ki * sum;
        last = data;
        return kp * result;
    }
};

#define L298N_SLOT_A 0x0
#define L298N_SLOT_B 0x1

class L298N {
private:
    uint8_t in1, in2, in3, in4;
public:
    L298N(uint8_t _in1,
          uint8_t _in2,
          uint8_t _in3,
          uint8_t _in4)
            : in1(_in1),
              in2(_in2),
              in3(_in3),
              in4(_in4) {}

    void init() {
        pinMode(in1, OUTPUT);
        pinMode(in2, OUTPUT);
        pinMode(in3, OUTPUT);
        pinMode(in4, OUTPUT);
        motor_a_break();
        motor_b_break();
    }

    void motor_a_stop() {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
    }

    void motor_a_break() {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, HIGH);
    }

    void motor_a_pwm(int value) {
        if (value > 0) {
            analogWrite(in1, value);
            analogWrite(in2, 0);
        } else {
            analogWrite(in1, 0);
            analogWrite(in2, value);
        }
    }

    void motor_b_pwm(int value) {
        if (value > 0) {
            analogWrite(in3, value);
            analogWrite(in4, 0);
        } else {
            analogWrite(in3, 0);
            analogWrite(in4, value);
        }
    }

    void motor_b_stop() {
        digitalWrite(in3, LOW);
        digitalWrite(in4, LOW);
    }

    void motor_b_break() {
        digitalWrite(in3, HIGH);
        digitalWrite(in4, HIGH);
    }


};

class Motor {

private:
    double power{};
    uint8_t slot;
    L298N *driver;
    // TODO encoder
public:
    Motor(L298N _driver, uint8_t _slot) : driver(&_driver), slot(_slot) {

    }

    MotorState state = Break;
    int16_t target_speed{};

    void set_power(double _power) {
        if (_power > 1.0 || _power < -1.0)
            return;
        power = _power;
        if (slot == L298N_SLOT_A)
            driver->motor_a_pwm((int) (power * 255));
        else driver->motor_b_pwm((int) (power * 255));
    }

    double get_power() {
        return power;
    }

};

MotorState from_raw(uint8_t data) {
    MotorState state = Stop;
    switch (data) {
        case 0x0:
            state = Stop;
            break;
        case 0x1:
            state = Break;
            break;
        case 0x2:
            state = Speed;
            break;
        default:
            break;
    }
    return state;
}

#define SPEED_HEAD 0xA1
#define SPEED_SIZE 14
#define STATE_HEAD 0xA3
#define STATE_SIZE 8


void decode_from_nano(HardwareSerial serial) {
    // 拿帧头
    int head = serial.read();
    if (head == SPEED_HEAD) {
        uint8_t buf[SPEED_SIZE - 1];
        serial.readBytes(buf, SPEED_SIZE - 1);
        short speeds[6];
        int k = 0;
        // 合并两字节
        for (int i = 0; i < SPEED_SIZE - 2; i += 2)
            speeds[k++] = (uint16_t) buf[i] & ((uint16_t) buf[i + 1] << 8);

        // 校验
        auto check = speeds[0];
        for (int i = 1; i < 6; i++) {
            check ^= speeds[i];
        }

        if (check != buf[SPEED_SIZE - 2])return;
        // TODO assign
    } else if (head == STATE_HEAD) {
        uint8_t buf[STATE_SIZE - 1];
        serial.readBytes(buf, STATE_SIZE - 1);
        MotorState states[6];
        for (int i = 0; i < 6; i++)
            states[i] = from_raw(buf[i]);

        uint8_t check = states[0];

        for (int i = 1; i < 6; i++) {
            check ^= states[i];
        }

        if (check != buf[STATE_SIZE - 2])return;
        // TODO assign
    }
}

void setup() {

    Serial.begin(9600);
}

void loop() {
    uint16_t a = (uint16_t) 0xFF | ((uint16_t) 0xFF << 8);
    Serial.println((int16_t) a);
    delay(2000);
}
