ros2/teleop_twist_joy
================

# Overview
The purpose of this package is to provide a generic facility for tele-operating Twist-based ROS2 robots with a standard joystick. 
It converts joy messages to velocity commands.

This node provides no rate limiting or autorepeat functionality. It is expected that you take advantage of the features built into [joy](https://index.ros.org/p/joy/github-ros-drivers-joystick_drivers/#foxy) for this.

## Executables
The package comes with the `teleop_node` that republishes `sensor_msgs/msg/Joy` messages as scaled `geometry_msgs/msg/Twist` messages.

## Subscribed Topics
- `joy (sensor_msgs/msg/Joy)`
  - Joystick messages to be translated to velocity commands.

## Published Topics
- `cmd_vel (geometry_msgs/msg/Twist)`
  - Command velocity messages arising from Joystick commands.

## Parameters
- `axis_angular (int, default: 0)`
  - Joystick axis to use for angular movement control.
  
- `axis_linear (int, default: 1)`
  - Joystick axis to use for linear movement control.
  
- `enable_button (int, default: 0)`
  - Joystick button to enable regular-speed movement.
  
- `enable_turbo_button (int, default: -1)`
  - Joystick button to enable high-speed movement (disabled when -1).
  
- `scale_angular (double, default: 1.0)`
  - Scale to apply to joystick angular axis.
  
- `scale_angluar_turbo (double, default: 1.0)`
  - Scale to apply to joystick angular axis for high-speed movement.
    
- `scale_linear (double, default: 0.5)`
  - Scale to apply to joystick linear axis for regular-speed movement.
  
- `scale_linear_turbo (double, default: 1.0)`
  - Scale to apply to joystick linear axis for high-speed movement.

# Usage

## Install
For most users building from source will not be required, execute `apt-get install ros-<rosdistro>-teleop-twist-joy` to install.

## Run
A launch file has been provided which has three arguments which can be changed in the terminal or via your own launch file.
To configure the node to match your joystick a config file can be used. 
There are several common ones provided in this package (atk3, ps3-holonomic, ps3, xbox, xd3), located here: https://github.com/ros2/teleop_twist_joy/tree/eloquent/config.

PS3 is default, to run for another config (e.g. xbox) use this:
````
ros2 launch teleop_twist_joy teleop-launch.py joy_config:='xbox'
````

__Note:__ this launch file also launches the `joy` node so do not run it separately.


## Arguments
- `joy_config (string, default: 'ps3')`
  - Config file to use
- `joy_dev (string, default: 'dev/input/js0')`
  - Joystick device to use
- `config_filepath (string, default: '/opt/ros/<rosdistro>/share/teleop_twist_joy/config/' + LaunchConfig('joy_config') + '.config.yaml')`
  - Path to config files
