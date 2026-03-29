#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <AccelStepper.h>
#include <std_msgs/msg/int32.h>

#if !defined(MICRO_ROS_TRANSPORT_ARDUINO_SERIAL)
#error This example is only available for Arduino framework with serial transport.
#endif

// Motor 1 pins
#define PUL_PIN1 18
#define DIR_PIN1 17
#define ENA_PIN1 16

// Motor 2 pins
#define PUL_PIN2 12
#define DIR_PIN2 11
#define ENA_PIN2 10

// Motor parameters optimized for maximum speed with smooth acceleration
const int STEPS_PER_REV = 400;
const int MAX_SPEED = 200000;     // Very high maximum speed in steps per second
const int ACCELERATION = 30000;   // More moderate acceleration to prevent jerk
const int INITIAL_SPEED = 500;    // Starting speed to avoid initial jerk

// Initialize stepper motors with driver pins
AccelStepper motor1(AccelStepper::DRIVER, PUL_PIN1, DIR_PIN1);
AccelStepper motor2(AccelStepper::DRIVER, PUL_PIN2, DIR_PIN2);

// ROS2 objects
rcl_subscription_t motor_subscriber;
std_msgs__msg__Int32 motor_recv_msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

// Debug serial output (comment out if not needed)
#define DEBUG_SERIAL Serial

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();} }
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){} }

void error_loop() {
  while (1) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
  }
}

// Motor command subscription callback
void motor_subscription_callback(const void *msgin) {
  const std_msgs__msg__Int32 *msg = (const std_msgs__msg__Int32 *)msgin;
  
  if (msg->data == 1) {
    #ifdef DEBUG_SERIAL
    DEBUG_SERIAL.println("Command 1: Moving to positions 8000, -8000");
    #endif
    motor1.moveTo(-4000);
    motor2.moveTo(4000);
  } else if (msg->data == 2) {
    #ifdef DEBUG_SERIAL
    DEBUG_SERIAL.println("Command 2: Moving to home positions");
    #endif
    motor1.moveTo(0);
    motor2.moveTo(0);
  }
}

void configureMotor(AccelStepper &motor) {
  // Set maximum speed - extremely high for maximum performance
  motor.setMaxSpeed(MAX_SPEED);
  
  // Set acceleration - moderate to ensure smooth starts without jerking
  motor.setAcceleration(ACCELERATION);
  
  // Set initial speed to avoid initial jerk when starting from standstill
  // This creates a smoother S-curve-like acceleration profile
  motor.setSpeed(INITIAL_SPEED);
  
  // Enable minimal pulse width for high-speed operation
  // (some drivers need longer pulses, adjust if needed)
  motor.setMinPulseWidth(1);
}

void setup() {
  // Initialize debug serial port
  #ifdef DEBUG_SERIAL
  DEBUG_SERIAL.begin(115200);
  DEBUG_SERIAL.println("Starting motor controller with optimized speed profile...");
  #endif
  
  // Set up built-in LED for error signaling
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Configure motor pins
  pinMode(ENA_PIN1, OUTPUT);
  pinMode(ENA_PIN2, OUTPUT);
  digitalWrite(ENA_PIN1, LOW); // Enable motor (LOW enables on most drivers)
  digitalWrite(ENA_PIN2, LOW);
  
  // Configure motors with optimized parameters
  configureMotor(motor1);
  configureMotor(motor2);
  
  // Initialize micro-ROS
  set_microros_serial_transports(Serial);
  delay(500); // Reduced delay to initialize faster
  
  // Initialize ROS components
  allocator = rcl_get_default_allocator();
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
  RCCHECK(rclc_node_init_default(&node, "motor_node", "", &support));
  
  // Create subscription
  RCCHECK(rclc_subscription_init_default(
    &motor_subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "/burger/motor_cmd"));
  
  // Create executor and add subscription
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_subscription(
    &executor,
    &motor_subscriber,
    &motor_recv_msg,
    &motor_subscription_callback,
    ON_NEW_DATA));
    
  #ifdef DEBUG_SERIAL
  DEBUG_SERIAL.println("Motor controller initialized with high-speed settings");
  #endif
}

void loop() {
  // Time critical section - motors need to be run as frequently as possible
  // for smooth, high-speed operation
  motor1.run();
  motor2.run();
  
  // Process any ROS messages with minimal overhead
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(1)));
  
  // No delays at all to ensure maximum motor step frequency
  // The ESP32 is fast enough to handle this loop at high speed
}