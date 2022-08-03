#include "arm.h"

#define LEFT -100
#define RIGHT 70
// NewPing::NewPing(uint8_t, uint8_t, unsigned int) : m_left_sonar() {}

#define SWT_PIN PC15

#define MOTOR_INTERFACE 1
#define DIR_PIN PB9
#define STP_PIN PB8
#define SLP_PIN PB7

#define SERVO_PIN_GRAB PA10
#define SERVO_PIN_TILT_LEFT PA2
#define SERVO_PIN_TILT_RIGHT PA1

#define SERVO_CLOSE_ANGLE 80 // closed grabbed
#define SERVO_OPEN_ANGLE 0   // released open
#define LEFT_LOWER_ANGLE 85 // lowered
#define LEFT_UPPER_ANGLE 10  // raised
#define RIGHT_LOWER_ANGLE 35 // lowered
#define RIGHT_UPPER_ANGLE 125  // raised
#define SERVO_WAIT_TIME 1000


#define L_TRIG PA14
#define L_ECHO PA15
#define R_TRIG PA4
#define R_ECHO PA5

#define SONAR_MAX_RANGE 25
#define USTEP_RATIO 8
#define DIST_RATIO 3.704
#define MAX_SPD 40000
#define ACCEL 300

namespace Arm {
    volatile int min_dist = SONAR_MAX_RANGE + 1;
    volatile bool see_idol_right = false;
    volatile bool see_idol_left = false;

    int move_distance = 0;
    int idol_position = 0;

    AccelStepper stepper(MOTOR_INTERFACE, STP_PIN, DIR_PIN);

    void armSetup() {
        pinMode(SLP_PIN, OUTPUT);
        digitalWrite(SLP_PIN, HIGH);
        pinMode(SWT_PIN, INPUT_PULLUP);
        stepper.setMaxSpeed(MAX_SPD);
        stepper.setAcceleration(ACCEL);
    }

    /**
     * @brief move arm to distance in cm, moves stepper the appropriate amount of steps for this distance
     */
    void armGoTo() {
        if(stepper.distanceToGo() == 0) {
            stepper.moveTo((int)move_distance*DIST_RATIO*10);
        }
        stepper.run();
    }

    /**
     * @brief gets the stepper to perform a homing routine
     *
     *
     *
     * @code{}
     * @return true
     * @return false
     */
    bool armSetHome() {
        bool switch_initial = digitalRead(SWT_PIN);
        int direction;

        // initialize direction towards center
        if (switch_initial) {
            direction = LEFT;
        } else {
            direction = RIGHT;
        }

        do {
            stepper.move(direction);
            stepper.run();
            digitalWrite(PB2, HIGH);
        } while (switch_initial == digitalRead(SWT_PIN));

        if (digitalRead(SWT_PIN)) { // if we have pressed it
            armSetHome();
        }

        stepper.setCurrentPosition(0);
        return true;
    }

    /**
     * @brief moves stepper to 0 from its current position. Warning: This method needs to be placed in loop in order to run the stepper motor properly.
     *
     * @return
     */
    void armGoHome() {
        stepper.moveTo(0);
        stepper.run();
    }

    int armCurrPos() {
        return stepper.currentPosition();
    }

    int armGetDistanceToGo() {
        return stepper.distanceToGo();
    }


    int armGetDistance(uint8_t trig_pin, uint8_t echo_pin) {
        int duration;
        digitalWrite(trig_pin, LOW);  // Added this line
        delayMicroseconds(2); // Added this line
        digitalWrite(trig_pin, HIGH);
        delayMicroseconds(10); // Added this line
        digitalWrite(trig_pin, LOW);
        duration = pulseIn(echo_pin, HIGH, 10000UL);
        return((duration/2) / 29.1);
    }

    /**
     * @brief Use sonar to sense for an idol.
     * This needs to be called inside a loop. Otherwise sonar
     * pings will not function properly.
     * \n
     * Ideally, this method is called with some delay
     * such that we are not pinging the sonar every loop.
     *
     * @return int, distance in cm needed to move in order to
     * reach idol.
     */
    int armSenseForIdol() {
        int left_distance;
        int right_distance;
        left_distance = armGetDistance(L_TRIG, L_ECHO);
        right_distance = armGetDistance(R_TRIG, R_ECHO);

        //establishes max range for sonar
        if(left_distance > SONAR_MAX_RANGE) {
            left_distance = 0;
        }
        if(right_distance > SONAR_MAX_RANGE) {
            right_distance = 0;
        }


        if(right_distance < min_dist && right_distance > 0) {
            min_dist = right_distance;
            see_idol_right = true;
        } else if(left_distance < min_dist && left_distance > 0) {
            min_dist = left_distance;
            see_idol_left = true;
        }

        if(min_dist <= SONAR_MAX_RANGE && see_idol_right) {
            return -min_dist;
        } else if(min_dist <= SONAR_MAX_RANGE && see_idol_left) {
            return min_dist;
        } else {
            return 0;
        }
    }

    /**
     * @brief setup to initialize sonar on arm
     *
     */
    void armSonarSetup() {
        pinMode(L_TRIG, OUTPUT);
        pinMode(L_ECHO, INPUT);
        pinMode(R_TRIG, OUTPUT);
        pinMode(R_ECHO, INPUT);
    }

    void armSleep() {
        digitalWrite(SLP_PIN, LOW);
    }

    void armDropOff() {

    }
}