/**
Software License Agreement (BSD)

\authors   Mike Purvis <mpurvis@clearpathrobotics.com>
\copyright Copyright (c) 2014, Clearpath Robotics, Inc., All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that
the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the
   following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
   following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of Clearpath Robotics nor the names of its contributors may be used to endorse or promote
   products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WAR-
RANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, IN-
DIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "teleop_twist_joy/teleop_twist_joy.h"

#include <geometry_msgs/msg/twist.hpp>
#include <rcutils/logging_macros.h>
#include <sensor_msgs/msg/joy.hpp>

#include <functional>
#include <map>
#include <string>

#define ROS_INFO_NAMED RCUTILS_LOG_INFO_NAMED
#define ROS_INFO_COND_NAMED RCUTILS_LOG_INFO_EXPRESSION_NAMED

namespace teleop_twist_joy
{

/**
 * Internal members of class. This is the pimpl idiom, and allows more flexibility in adding
 * parameters later without breaking ABI compatibility, for robots which link TeleopTwistJoy
 * directly into base nodes.
 */
struct TeleopTwistJoy::Impl
{
  void joyCallback(const sensor_msgs::msg::Joy::SharedPtr joy);

  rclcpp::Node::SharedPtr node;

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub;
  rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub;

  int enable_button;
  int enable_turbo_button;

  std::map<std::string, int> axis_linear_map;
  std::map<std::string, double> scale_linear_map;
  std::map<std::string, double> scale_linear_turbo_map;

  std::map<std::string, int> axis_angular_map;
  std::map<std::string, double> scale_angular_map;
  std::map<std::string, double> scale_angular_turbo_map;

  bool sent_disable_msg;
};

/**
 * Constructs TeleopTwistJoy.
 * \param nh NodeHandle to use for setting up the publisher and subscriber.
 * \param nh_param NodeHandle to use for searching for configuration parameters.
 */
TeleopTwistJoy::TeleopTwistJoy(rclcpp::Node::SharedPtr & node)
{
  pimpl_ = new Impl();

  pimpl_->node = node;

  pimpl_->cmd_vel_pub = node->create_publisher<geometry_msgs::msg::Twist>("cmd_vel",
    rmw_qos_profile_sensor_data);
  pimpl_->joy_sub = node->create_subscription<sensor_msgs::msg::Joy>("joy",
    std::bind(&TeleopTwistJoy::Impl::joyCallback, this->pimpl_, std::placeholders::_1),
    rmw_qos_profile_sensor_data);

  pimpl_->enable_button = 5;
  node->get_parameter("enable_button", pimpl_->enable_button);

  pimpl_->enable_turbo_button = -1;
  node->get_parameter("enable_turbo_button", pimpl_->enable_turbo_button);

  // TODO(clalancette): node->get_parameter(s) doesn't seem to
  // support getting a map of values yet.  Revisit this once it does.
  // if (nh_param->getParam("axis_linear", pimpl_->axis_linear_map))
  // {
  //   nh_param->getParam("axis_linear", pimpl_->axis_linear_map);
  //   nh_param->getParam("scale_linear", pimpl_->scale_linear_map);
  //   nh_param->getParam("scale_linear_turbo", pimpl_->scale_linear_turbo_map);
  // }
  // else
  {
    pimpl_->axis_linear_map["x"] = 5;
    node->get_parameter("axis_linear", pimpl_->axis_linear_map["x"]);
    pimpl_->scale_linear_map["x"] = 0.5;
    node->get_parameter("scale_linear", pimpl_->scale_linear_map["x"]);
    pimpl_->scale_linear_turbo_map["x"] = 1.0;
    node->get_parameter("scale_linear_turbo", pimpl_->scale_linear_turbo_map["x"]);
  }

  // TODO(clalancette): node->get_parameter(s) doesn't seem to
  // support getting a map of values yet.  Revisit this once it does.
  // if (nh_param->getParam("axis_angular", pimpl_->axis_angular_map))
  // {
  //   nh_param->getParam("axis_angular", pimpl_->axis_angular_map);
  //   nh_param->getParam("scale_angular", pimpl_->scale_angular_map);
  //   nh_param->getParam("scale_angular_turbo", pimpl_->scale_angular_turbo_map);
  // }
  // else
  {
    pimpl_->axis_angular_map["yaw"] = 2;
    node->get_parameter("axis_angular", pimpl_->axis_angular_map["yaw"]);
    pimpl_->scale_angular_map["yaw"] = 0.5;
    node->get_parameter("scale_angular", pimpl_->scale_angular_map["yaw"]);
    pimpl_->scale_angular_turbo_map["yaw"] = pimpl_->scale_angular_map["yaw"];
    node->get_parameter("scale_angular_turbo", pimpl_->scale_angular_turbo_map["yaw"]);
  }

  ROS_INFO_NAMED("TeleopTwistJoy", "Teleop enable button %i.", pimpl_->enable_button);
  ROS_INFO_COND_NAMED(pimpl_->enable_turbo_button >= 0, "TeleopTwistJoy",
      "Turbo on button %i.", pimpl_->enable_turbo_button);

  for (std::map<std::string, int>::iterator it = pimpl_->axis_linear_map.begin();
      it != pimpl_->axis_linear_map.end(); ++it)
  {
    ROS_INFO_NAMED("TeleopTwistJoy", "Linear axis %s on %i at scale %f.",
    it->first.c_str(), it->second, pimpl_->scale_linear_map[it->first]);
    ROS_INFO_COND_NAMED(pimpl_->enable_turbo_button >= 0, "TeleopTwistJoy",
        "Turbo for linear axis %s is scale %f.", it->first.c_str(), pimpl_->scale_linear_turbo_map[it->first]);
  }

  for (std::map<std::string, int>::iterator it = pimpl_->axis_angular_map.begin();
      it != pimpl_->axis_angular_map.end(); ++it)
  {
    ROS_INFO_NAMED("TeleopTwistJoy", "Angular axis %s on %i at scale %f.",
    it->first.c_str(), it->second, pimpl_->scale_angular_map[it->first]);
    ROS_INFO_COND_NAMED(pimpl_->enable_turbo_button >= 0, "TeleopTwistJoy",
        "Turbo for angular axis %s is scale %f.", it->first.c_str(), pimpl_->scale_angular_turbo_map[it->first]);
  }

  pimpl_->sent_disable_msg = false;
}

void TeleopTwistJoy::Impl::joyCallback(const sensor_msgs::msg::Joy::SharedPtr joy_msg)
{
  auto cmd_vel_msg = std::make_shared<geometry_msgs::msg::Twist>();

  cmd_vel_msg->linear.x = 0.0;
  cmd_vel_msg->linear.y = 0.0;
  cmd_vel_msg->linear.z = 0.0;

  cmd_vel_msg->angular.x = 0.0;
  cmd_vel_msg->angular.y = 0.0;
  cmd_vel_msg->angular.z = 0.0;

  if (enable_turbo_button >= 0 && joy_msg->buttons[enable_turbo_button])
  {
    if (axis_linear_map.find("x") != axis_linear_map.end())
    {
      cmd_vel_msg->linear.x = joy_msg->axes[axis_linear_map["x"]] * scale_linear_turbo_map["x"];
    }
    if (axis_linear_map.find("y") != axis_linear_map.end())
    {
      cmd_vel_msg->linear.y = joy_msg->axes[axis_linear_map["y"]] * scale_linear_turbo_map["y"];
    }
    if  (axis_linear_map.find("z") != axis_linear_map.end())
    {
      cmd_vel_msg->linear.z = joy_msg->axes[axis_linear_map["z"]] * scale_linear_turbo_map["z"];
    }
    if  (axis_angular_map.find("yaw") != axis_angular_map.end())
    {
      cmd_vel_msg->angular.z = joy_msg->axes[axis_angular_map["yaw"]] * scale_angular_turbo_map["yaw"];
    }
    if  (axis_angular_map.find("pitch") != axis_angular_map.end())
    {
      cmd_vel_msg->angular.y = joy_msg->axes[axis_angular_map["pitch"]] * scale_angular_turbo_map["pitch"];
    }
    if  (axis_angular_map.find("roll") != axis_angular_map.end())
    {
      cmd_vel_msg->angular.x = joy_msg->axes[axis_angular_map["roll"]] * scale_angular_turbo_map["roll"];
    }

    cmd_vel_pub->publish(cmd_vel_msg);
    sent_disable_msg = false;
  }
  else if (joy_msg->buttons[enable_button])
  {
    if  (axis_linear_map.find("x") != axis_linear_map.end())
    {
      cmd_vel_msg->linear.x = joy_msg->axes[axis_linear_map["x"]] * scale_linear_map["x"];
    }
    if  (axis_linear_map.find("y") != axis_linear_map.end())
    {
      cmd_vel_msg->linear.y = joy_msg->axes[axis_linear_map["y"]] * scale_linear_map["y"];
    }
    if  (axis_linear_map.find("z") != axis_linear_map.end())
    {
      cmd_vel_msg->linear.z = joy_msg->axes[axis_linear_map["z"]] * scale_linear_map["z"];
    }
    if  (axis_angular_map.find("yaw") != axis_angular_map.end())
    {
      cmd_vel_msg->angular.z = joy_msg->axes[axis_angular_map["yaw"]] * scale_angular_map["yaw"];
    }
    if  (axis_angular_map.find("pitch") != axis_angular_map.end())
    {
      cmd_vel_msg->angular.y = joy_msg->axes[axis_angular_map["pitch"]] * scale_angular_map["pitch"];
    }
    if  (axis_angular_map.find("roll") != axis_angular_map.end())
    {
      cmd_vel_msg->angular.x = joy_msg->axes[axis_angular_map["roll"]] * scale_angular_map["roll"];
    }

    cmd_vel_pub->publish(cmd_vel_msg);
    sent_disable_msg = false;
  }
  else
  {
    // When enable button is released, immediately send a single no-motion command
    // in order to stop the robot.
    if (!sent_disable_msg)
    {
      cmd_vel_pub->publish(cmd_vel_msg);
      sent_disable_msg = true;
    }
  }
}

}  // namespace teleop_twist_joy
