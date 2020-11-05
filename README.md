ros2/teleop_twist_joy [![Build Status](https://travis-ci.org/ros-teleop/teleop_twist_joy.svg?branch=indigo-devel)](https://travis-ci.org/ros-teleop/teleop_twist_joy)
================

# Overview
The purpose of this package is to provide a generic facility for tele-operating Twist-based ROS robots with a standard joystick. Examples of such platforms include [TurtleBot](http://wiki.ros.org/Robots/TurtleBot), [Husky](http://wiki.ros.org/Robots/Husky), and [Kingfisher](http://wiki.ros.org/Robots/Kingfisher).

This node provides no rate limiting or autorepeat functionality. It is expected that you take advantage of the features built into [joy](https://index.ros.org/p/joy/github-ros-drivers-joystick_drivers/#foxy) for this.

## Install
For most users building from source will not be required, execute `apt-get install ros-foxy-teleop-twist-joy` to install.

## Executables
The package comes with a node, `teleop_node`, which may be used out of the box, this republishes Joy messages as scaled geometry_msgs/Twist messages.


## Subscribed Topics
joy (sensor_msgs/Joy)
Joystick messages to be translated to velocity commands.

## Published Topics
cmd_vel (geometry_msgs/Twist)
Command velocity messages arising from Joystick commands.

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

- `use_sim_time` _TODO:add details_

