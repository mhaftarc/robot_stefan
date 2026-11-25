# robot_stefan - firmware architecture

Short overview of how the firmware is split, who does what and how the parts talk to each other.
Read this before you start coding. If you want to change something here (types, protocol, folders), tell the group first.

Tools: ESP-IDF v6.1.1 (VS Code + ESP-IDF extension), plain C, FreeRTOS. Board: ESP32 DevKit (WROOM-32).

## Big picture

Every 10 ms (100 Hz) the robot does the same loop:

```
sensors  -->  sensor_data_t  -->  autonomy  -->  motor_cmd_t  -->  motors
                                     ^
                                     |  autonomy_params_t, robot_mode_t
                                     |
laptop app  -- Bluetooth SPP -->  bt_link  -->  control (state machine)
```

- Sensor drivers fill `sensor_data_t`.
- Autonomy (line tracking / following / avoiding) reads it and returns `motor_cmd_t`.
- Motor driver turns `motor_cmd_t` into PWM.
- The laptop app sends commands (mode, manual drive, params) over Bluetooth. `control` decides what the robot does.

Autonomy does not know anything about pins or PWM. Drivers don't know anything about PID. The only thing everyone shares are the types in `common`.

## Folders and owners

```
robot_stefan/
  CMakeLists.txt
  sdkconfig                 shared config (commit it!)
  main/
    main.c                  app_main: init + create tasks              Michal H.
  components/
    common/                 shared types (robot_types.h) + pins (board_pins.h)  Michal H. (change only after agreeing)
    autonomy/               line tracking, following, avoiding         Robert
    motor/                  TB6612FNG, PWM, speed limit                Michal H.
    line_sensor/            5x HW-006 line sensors (digital)           Michal H.
    battery/                battery voltage (ADC, GPIO 34)             Michal H.
    ultrasonic/             HC-SR04                                    Michal H.
    bt_link/                Bluetooth SPP + protocol parsing           Michal H. (protocol with Michal B.)
    control/                state machine, fail-safe, heartbeat        Michal H.
```

Laptop app (Python) lives in its own folder or repo - Michal B. It only needs the protocol below, not the C headers.

Every component looks the same:
- `include/name.h` - public functions, what others can call
- `name.c` - implementation
- `CMakeLists.txt` - source files + `REQUIRES` (what it depends on)

## Hardware notes (from Kuba, 22.11)

- all pins are in `components/common/include/board_pins.h`, that file is the only place to change them
- line sensors are 5 separate HW-006 modules, digital only (no analog value, fixed threshold), about 1.1 cm apart, numbered 1-5 from the left. The module gives 0 on black, the driver flips it so in code 1 = line
- battery is measured on GPIO 34 through a 4x 1k divider (V_bat / 4)
- ESP32 can have only one power source: with USB plugged in, VIN must be disconnected
- HC-SR04 and the motors work only with the battery connected (5 V comes from the step-down)

## Shared types (components/common/include/robot_types.h)

| Type | What it is | Filled by | Used by |
|---|---|---|---|
| `sensor_data_t` | `line[5]` (1 = sees black line, 0 = light floor, index 0 = leftmost sensor), `distance_cm` (-1 = no echo), `battery_v`, `timestamp_ms` | sensor drivers | autonomy, telemetry |
| `motor_cmd_t` | `left`, `right`, float -1.0 .. 1.0, plus = forward | autonomy or manual drive | motor driver |
| `robot_mode_t` | `MODE_IDLE`, `MODE_MANUAL`, `MODE_LINE`, `MODE_FOLLOW`, `MODE_AVOID`, `MODE_FAILSAFE` | control | autonomy, telemetry |
| `autonomy_params_t` | `line_kp`, `line_ki`, `line_kd`, `line_base_speed`, `follow_target_cm`, `follow_kp`, `avoid_dist_cm`, `avoid_speed` | bt_link (from the app) | autonomy |

Motor speed limit (about 70% PWM, motors are 6 V and battery is up to 8.4 V) is done inside the motor driver. Autonomy always works in -1..1.

## Autonomy interface (components/autonomy/include/autonomy.h)

```c
void autonomy_reset(void);

motor_cmd_t autonomy_step(
    robot_mode_t mode,
    const sensor_data_t *s,
    const autonomy_params_t *p,
    float dt_s
);
```

Rules for autonomy code:
- no ESP32 or FreeRTOS calls (so it can be tested on a PC with gcc)
- no malloc
- never blocks, it is called every 10 ms
- `autonomy_reset()` is called when the mode changes (clear integral etc.)

## FreeRTOS tasks (plan)

| Task | Core | Rate | What it does |
|---|---|---|---|
| `control_task` | 1 | 100 Hz | read line sensors, take latest distance, check heartbeat, run state machine, call `autonomy_step()` or use manual command, send command to motors |
| `ultrasonic_task` | 1 | 20 Hz | trigger HC-SR04, measure echo, median of 3, publish latest distance |
| `bt_task` | 0 | on data | receive frames, check CRC, pass commands to control |
| `telemetry_task` | 0 | 10 Hz | send mode, battery, distance, line error to the laptop |

How they talk:
- commands from Bluetooth -> `control_task`: FreeRTOS queue (`xQueueSend` / `xQueueReceive`)
- latest distance from `ultrasonic_task`: queue of length 1 with `xQueueOverwrite` / `xQueuePeek` (always the newest value)
- `autonomy_params_t`: one shared struct protected by a mutex (bt_task writes, control_task reads)

Fail-safe: the app sends HEARTBEAT 10 times per second. If `control_task` sees nothing for 300 ms -> `MODE_FAILSAFE`, motors stop. The robot leaves FAILSAFE only after an explicit SET_MODE from the app. We don't rely on the Bluetooth disconnect callback, it can come too late.

## Bluetooth protocol (draft, finish with Michal B.)

Link: Bluetooth Classic SPP. On the laptop the robot shows up as a COM port, Python uses pyserial.

Frame:

```
| 0xAA | TYPE | LEN | PAYLOAD (LEN bytes) | CRC8 |
```

- CRC8 is computed over TYPE, LEN and PAYLOAD
- numbers are little-endian (ESP32 and PC are both little-endian, in Python use `struct.pack('<f', x)`)

Laptop -> robot:

| TYPE | Name | Payload |
|---|---|---|
| 0x01 | SET_MODE | u8 mode (same numbers as `robot_mode_t`) |
| 0x02 | DRIVE | int8 left, int8 right (-100..100), only in MANUAL |
| 0x03 | STOP | - |
| 0x04 | HEARTBEAT | - |
| 0x05 | PARAM | u8 param_id, float value |
| 0x06 | PING | u32 laptop timestamp |

Robot -> laptop:

| TYPE | Name | Payload |
|---|---|---|
| 0x81 | TELEMETRY | u8 mode, u8 line_bits (bit 0 = sensor 1 ... bit 4 = sensor 5), float distance_cm, float battery_v |
| 0x85 | ACK | u8 type of the acked frame |
| 0x86 | ECHO | u32 timestamp copied from PING (latency test, SC4) |

PARAM ids (names match `autonomy_params_t`):

| id | name | unit |
|---|---|---|
| 1 | line_kp | - |
| 2 | line_ki | - |
| 3 | line_kd | - |
| 4 | line_base_speed | 0..1 |
| 5 | follow_target_cm | cm |
| 6 | follow_kp | - |
| 7 | avoid_dist_cm | cm |
| 8 | avoid_speed | 0..1 |


## Git rules

- `main` must always build
- everyone works on their own branch (`autonomy`, `bt-link`, `motor` ...) and merges with a pull request
- don't commit `build/` and `.vscode/`
- if you change `robot_types.h` or the protocol, write it in the group chat first, it breaks other people's code
