//
// Created by berberman on 12/21/2019.
//

#ifndef FRONTROBOT_MOTOR_HPP
#define FRONTROBOT_MOTOR_HPP


template<typename T>
class controller_t {
public:
    virtual double operator()(T data) = 0;
};

class pid_t : controller_t<double> {
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

/**
 * 电机内核
 * 提供电机输出
 * @tparam pin1 H 桥引脚
 * @tparam pin2 H 桥引脚
 */
template<uint8_t pin1, uint8_t pin2>
struct motor_core_t {

    motor_core_t() = delete;

    ~motor_core_t() = delete;

    void state_stop() {
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, LOW);
    }

    void state_break() {
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, HIGH);
    }

    void pwm(int value) {
        if (value >= 0) {
            analogWrite(pin1, value);
            analogWrite(pin2, 0);
        } else {
            analogWrite(pin1, 0);
            analogWrite(pin2, -value);
        }
    }
};

using motor_state_t = uint8_t;

#define MOTOR_STATE_STOP 0x0
#define MOTOR_STATE_BREAK 0x1
#define MOTOR_STATE_SPEED 0x2

/**
 * 电机
 * 封装电机输出
 * @tparam core 电机内核
 */
template<class core>
class motor_t {
private:
    bool close_loop = false;
    int ticks = 0, last_ticks = 0;
    float current_speed = .0, target_speed = .0;
public:
    pid_t pid;

    void set_power(int power) {
        core::pwm(power / 255);
    }

    void set_state(uint8_t state) {
        close_loop = false;
        switch (state) {
            case MOTOR_STATE_STOP:
                core::state_stop();
                break;
            case MOTOR_STATE_BREAK:
                core::state_break();
                break;
            case MOTOR_STATE_SPEED:
                close_loop = true;
                break;
            default:
                break;
        }
    }

    void set_speed(float speed) {
        target_speed = speed;
    }

    void run() {
        current_speed = ticks - last_ticks;
        if (!close_loop)return;
        set_power(pid(target_speed - current_speed));
    }
};

#endif //FRONTROBOT_MOTOR_HPP
