# microros-servo

High-speed dual stepper motor control for ESP32-S3 using micro-ROS and AccelStepper. The firmware listens for ROS 2 commands over serial and moves two stepper motors between preset positions with smooth acceleration.

## What’s inside

- **Board:** `esp32-s3-devkitm-1` (Arduino framework)
- **micro-ROS transport:** Serial (USB)
- **Stepper library:** AccelStepper
- **ROS 2 topic:** `/burger/motor_cmd` (`std_msgs/Int32`)

## Motor command behavior

The node reacts to integer commands on `/burger/motor_cmd`:

| Command | Action | motor1 target | motor2 target |
| --- | --- | --- | --- |
| `1` | Move to preset positions | `-4000` | `4000` |
| `2` | Return to home | `0` | `0` |

## Pinout

| Motor | PUL | DIR | ENA |
| --- | --- | --- | --- |
| Motor 1 | 18 | 17 | 16 |
| Motor 2 | 12 | 11 | 10 |

> Note: `ENA` is driven LOW to enable most stepper drivers. Confirm your driver’s enable polarity.

## Prerequisites

- PlatformIO (VS Code extension or CLI)
- ESP32-S3 DevKitM-1 connected over USB
- ROS 2 + micro-ROS agent on your host machine

## Build and upload

From the project root, you can use PlatformIO to build and upload:

```bash
pio run
pio run -t upload
```

Monitor serial output (optional):

```bash
pio device monitor -b 115200
```

## micro-ROS agent (host)

Run the agent on your host to bridge ROS 2 and the serial transport:

```bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0 -b 115200
```

Adjust the serial device to match your system (for example `/dev/ttyACM0`).

## ROS 2 example publish

Send a command to move motors:

```bash
ros2 topic pub /burger/motor_cmd std_msgs/msg/Int32 "{data: 1}" --once
```

Return to home:

```bash
ros2 topic pub /burger/motor_cmd std_msgs/msg/Int32 "{data: 2}" --once
```

## Configuration notes

Key runtime parameters live in `src/main.cpp`:

- `STEPS_PER_REV`, `MAX_SPEED`, `ACCELERATION`, `INITIAL_SPEED`
- Subscription topic: `/burger/motor_cmd`
- Node name: `motor_node`

## Project structure

```
.
├─ platformio.ini
├─ src/
│  └─ main.cpp
├─ include/
├─ lib/
└─ test/
```

## Troubleshooting

- If the board doesn’t respond, verify the serial transport macro is set by `micro_ros_platformio`.
- If the motors don’t move, confirm your step/dir pins and driver enable polarity.
- If motion is jerky, tune `MAX_SPEED` and `ACCELERATION` for your mechanics.

## License

Add your license details here.
