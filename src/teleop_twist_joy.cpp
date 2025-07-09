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

#include <cinttypes>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>

#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <rcutils/logging_macros.h>
#include <sensor_msgs/msg/joy.hpp>
#include <std_msgs/msg/bool.hpp>

#include "teleop_twist_joy/teleop_twist_joy.hpp"
#define ROS_INFO_NAMED RCUTILS_LOG_INFO_NAMED
#define ROS_INFO_COND_NAMED RCUTILS_LOG_INFO_EXPRESSION_NAMED

//-------
#include <iostream>
using namespace std;
#include <cmath>
#define _USE_MATH_DEFINES
#include <array>

// takes input [0, 1] for the left and right joystick, output Ux and Theta.dot Z, witch go into a twist file for the motor drivers

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
    void sendCmdVelMsg(const sensor_msgs::msg::Joy::SharedPtr, const std::string &which_map);
    double getVal(const sensor_msgs::msg::Joy::SharedPtr joy_msg, const std::map<std::string, int64_t> &axis_map,
                  const std::map<std::string, double> &scale_map, const std::string &fieldname, const std::map<std::string, double> &turbo_scale_map);

    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr lock_autonomy_pub;

    bool require_enable_button;
    bool require_autonomy_button;
    int64_t enable_axis;
    int64_t turbo_axis;
    int64_t track_control_button;
    int64_t autonomy_button;
    float deadzone;
    bool track_mode = false;
    bool track_button_latch = false;
    float base_width;

    std::map<std::string, int64_t> axis_linear_map;
    std::map<std::string, std::map<std::string, double>> scale_linear_map;

    std::map<std::string, int64_t> axis_angular_map;
    std::map<std::string, std::map<std::string, double>> scale_angular_map;
    array<float, 2> motionconverter(float gauche, float droit);
  };

  /**
   * Constructs TeleopTwistJoy.
   */
  TeleopTwistJoy::TeleopTwistJoy(const rclcpp::NodeOptions &options) : Node("teleop_twist_joy_node", options)
  {
    pimpl_ = new Impl;

    pimpl_->cmd_vel_pub = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
    pimpl_->lock_autonomy_pub = this->create_publisher<std_msgs::msg::Bool>("lock_autonomy", 10);
    pimpl_->joy_sub = this->create_subscription<sensor_msgs::msg::Joy>("joy", rclcpp::QoS(10),
                                                                       std::bind(&TeleopTwistJoy::Impl::joyCallback, this->pimpl_, std::placeholders::_1));

    pimpl_->require_enable_button = this->declare_parameter("require_enable_button", true);
    pimpl_->require_autonomy_button = this->declare_parameter("require_autonomy_button", true);
    pimpl_->enable_axis = this->declare_parameter("enable_axis", 5);
    pimpl_->turbo_axis = this->declare_parameter("turbo_axis", -1);
    pimpl_->track_control_button = this->declare_parameter<int>("track_control_button", 4);
    pimpl_->autonomy_button = this->declare_parameter<int>("autonomy_button", 0);

    pimpl_->base_width = this->declare_parameter<float>("base_width", 0.5);
    pimpl_->deadzone = this->declare_parameter<float>("deadzone", 0.2);

    std::map<std::string, int64_t> default_linear_map{
      {"x", 4L},
      {"y", -1L},
      {"z", -1L},
    };
    this->declare_parameters("axis_linear", default_linear_map);
    this->get_parameters("axis_linear", pimpl_->axis_linear_map);

    std::map<std::string, int64_t> default_angular_map{
      {"yaw", 3L},
      {"pitch", -1L},
      {"roll", -1L},
    };
    this->declare_parameters("axis_angular", default_angular_map);
    this->get_parameters("axis_angular", pimpl_->axis_angular_map);

    std::map<std::string, double> default_scale_linear_normal_map{
      {"x", 0.5},
      {"y", 0.0},
      {"z", 0.0},
    };
    this->declare_parameters("scale_linear", default_scale_linear_normal_map);
    this->get_parameters("scale_linear", pimpl_->scale_linear_map["normal"]);

    std::map<std::string, double> default_scale_linear_turbo_map{
      {"x", 1.0},
      {"y", 0.0},
      {"z", 0.0},
    };
    this->declare_parameters("scale_linear_turbo", default_scale_linear_turbo_map);
    this->get_parameters("scale_linear_turbo", pimpl_->scale_linear_map["turbo"]);

    std::map<std::string, double> default_scale_angular_normal_map{
      {"yaw", 0.5},
      {"pitch", 0.0},
      {"roll", 0.0},
    };
    this->declare_parameters("scale_angular", default_scale_angular_normal_map);
    this->get_parameters("scale_angular", pimpl_->scale_angular_map["normal"]);

    std::map<std::string, double> default_scale_angular_turbo_map{
      {"yaw", 1.0},
      {"pitch", 0.0},
      {"roll", 0.0},
    };
    this->declare_parameters("scale_angular_turbo", default_scale_angular_turbo_map);
    this->get_parameters("scale_angular_turbo", pimpl_->scale_angular_map["turbo"]);

    ROS_INFO_COND_NAMED(pimpl_->enable_axis, "TeleopTwistJoy",
                        "Teleop enable axis %" PRId64 ".", pimpl_->enable_axis);
    ROS_INFO_COND_NAMED(pimpl_->turbo_axis >= 0, "TeleopTwistJoy",
                        "Turbo on axis %" PRId64 ".", pimpl_->turbo_axis);
    ROS_INFO_COND_NAMED(pimpl_->track_control_button >= 0, "TeleopTwistJoy",
                        "Track control button %" PRId64 ".", pimpl_->track_control_button);
    ROS_INFO_COND_NAMED(pimpl_->require_autonomy_button, "TeleopTwistJoy",
                        "Autonomy enable button %" PRId64 ".", pimpl_->autonomy_button);

    for (std::map<std::string, int64_t>::iterator it = pimpl_->axis_linear_map.begin();
         it != pimpl_->axis_linear_map.end(); ++it)
    {
      ROS_INFO_COND_NAMED(it->second != -1L, "TeleopTwistJoy", "Linear axis %s on %" PRId64 " at scale %f.",
                          it->first.c_str(), it->second, pimpl_->scale_linear_map["normal"][it->first]);
      ROS_INFO_COND_NAMED(pimpl_->turbo_axis >= 0 && it->second != -1, "TeleopTwistJoy",
                          "Turbo for linear axis %s is scale %f.", it->first.c_str(), pimpl_->scale_linear_map["turbo"][it->first]);
    }

    for (std::map<std::string, int64_t>::iterator it = pimpl_->axis_angular_map.begin();
         it != pimpl_->axis_angular_map.end(); ++it)
    {
      ROS_INFO_COND_NAMED(it->second != -1L, "TeleopTwistJoy", "Angular axis %s on %" PRId64 " at scale %f.",
                          it->first.c_str(), it->second, pimpl_->scale_angular_map["normal"][it->first]);
      ROS_INFO_COND_NAMED(pimpl_->turbo_axis >= 0 && it->second != -1, "TeleopTwistJoy",
                          "Turbo for angular axis %s is scale %f.", it->first.c_str(), pimpl_->scale_angular_map["turbo"][it->first]);
    }

    auto param_callback = [this](std::vector<rclcpp::Parameter> parameters)
    {
      static std::set<std::string> intparams = {"axis_linear.x", "axis_linear.y", "axis_linear.z",
                                                "axis_angular.yaw", "axis_angular.pitch", "axis_angular.roll",
                                                "enable_axis", "turbo_axis", "track_control_button", "autonomy_button"};
      static std::set<std::string> doubleparams = {"scale_linear.x", "scale_linear.y", "scale_linear.z",
                                                   "scale_linear_turbo.x", "scale_linear_turbo.y", "scale_linear_turbo.z",
                                                   "scale_angular.yaw", "scale_angular.pitch", "scale_angular.roll",
                                                   "scale_angular_turbo.yaw", "scale_angular_turbo.pitch", "scale_angular_turbo.roll",
                                                   "base_width", "deadzone"};
      static std::set<std::string> boolparams = {"require_enable_button", "require_autonomy_button"};
      auto result = rcl_interfaces::msg::SetParametersResult();
      result.successful = true;

      // Loop to check if changed parameters are of expected data type
      for (const auto &parameter : parameters)
      {
        if (intparams.count(parameter.get_name()) == 1)
        {
          if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_INTEGER)
          {
            result.reason = "Only integer values can be set for '" + parameter.get_name() + "'.";
            RCLCPP_WARN(this->get_logger(), result.reason.c_str());
            result.successful = false;
            return result;
          }
        }
        else if (doubleparams.count(parameter.get_name()) == 1)
        {
          if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_DOUBLE)
          {
            result.reason = "Only double values can be set for '" + parameter.get_name() + "'.";
            RCLCPP_WARN(this->get_logger(), result.reason.c_str());
            result.successful = false;
            return result;
          }
        }
        else if (boolparams.count(parameter.get_name()) == 1)
        {
          if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_BOOL)
          {
            result.reason = "Only boolean values can be set for '" + parameter.get_name() + "'.";
            RCLCPP_WARN(this->get_logger(), result.reason.c_str());
            result.successful = false;
            return result;
          }
        }
      }

      // Loop to assign changed parameters to the member variables
      for (const auto &parameter : parameters)
      {
        if (parameter.get_name() == "require_enable_button")
        {
          this->pimpl_->require_enable_button = parameter.get_value<rclcpp::PARAMETER_BOOL>();
        }
        if (parameter.get_name() == "require_autonomy_button")
        {
          this->pimpl_->require_autonomy_button = parameter.get_value<rclcpp::PARAMETER_BOOL>();
        }
        if (parameter.get_name() == "enable_axis")
        {
          this->pimpl_->enable_axis = parameter.get_value<rclcpp::PARAMETER_INTEGER>();
        }
        else if (parameter.get_name() == "turbo_axis")
        {
          this->pimpl_->turbo_axis = parameter.get_value<rclcpp::PARAMETER_INTEGER>();
        }
        else if (parameter.get_name() == "track_control_button")
        {
          this->pimpl_->track_control_button = parameter.get_value<rclcpp::PARAMETER_INTEGER>();
        }
        else if (parameter.get_name() == "autonomy_button")
        {
          this->pimpl_->autonomy_button = parameter.get_value<rclcpp::PARAMETER_INTEGER>();
        }
        else if (parameter.get_name() == "base_width")
        {
          this->pimpl_->base_width = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "deadzone")
        {
          this->pimpl_->deadzone = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "axis_linear.x")
        {
          this->pimpl_->axis_linear_map["x"] = parameter.get_value<rclcpp::PARAMETER_INTEGER>();
        }
        else if (parameter.get_name() == "axis_linear.y")
        {
          this->pimpl_->axis_linear_map["y"] = parameter.get_value<rclcpp::PARAMETER_INTEGER>();
        }
        else if (parameter.get_name() == "axis_linear.z")
        {
          this->pimpl_->axis_linear_map["z"] = parameter.get_value<rclcpp::PARAMETER_INTEGER>();
        }
        else if (parameter.get_name() == "axis_angular.yaw")
        {
          this->pimpl_->axis_angular_map["yaw"] = parameter.get_value<rclcpp::PARAMETER_INTEGER>();
        }
        else if (parameter.get_name() == "axis_angular.pitch")
        {
          this->pimpl_->axis_angular_map["pitch"] = parameter.get_value<rclcpp::PARAMETER_INTEGER>();
        }
        else if (parameter.get_name() == "axis_angular.roll")
        {
          this->pimpl_->axis_angular_map["roll"] = parameter.get_value<rclcpp::PARAMETER_INTEGER>();
        }
        else if (parameter.get_name() == "scale_linear_turbo.x")
        {
          this->pimpl_->scale_linear_map["turbo"]["x"] = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "scale_linear_turbo.y")
        {
          this->pimpl_->scale_linear_map["turbo"]["y"] = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "scale_linear_turbo.z")
        {
          this->pimpl_->scale_linear_map["turbo"]["z"] = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "scale_linear.x")
        {
          this->pimpl_->scale_linear_map["normal"]["x"] = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "scale_linear.y")
        {
          this->pimpl_->scale_linear_map["normal"]["y"] = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "scale_linear.z")
        {
          this->pimpl_->scale_linear_map["normal"]["z"] = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "scale_angular_turbo.yaw")
        {
          this->pimpl_->scale_angular_map["turbo"]["yaw"] = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "scale_angular_turbo.pitch")
        {
          this->pimpl_->scale_angular_map["turbo"]["pitch"] = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "scale_angular_turbo.roll")
        {
          this->pimpl_->scale_angular_map["turbo"]["roll"] = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "scale_angular.yaw")
        {
          this->pimpl_->scale_angular_map["normal"]["yaw"] = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "scale_angular.pitch")
        {
          this->pimpl_->scale_angular_map["normal"]["pitch"] = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
        else if (parameter.get_name() == "scale_angular.roll")
        {
          this->pimpl_->scale_angular_map["normal"]["roll"] = parameter.get_value<rclcpp::PARAMETER_DOUBLE>();
        }
      }
      return result;
    };

    callback_handle = this->add_on_set_parameters_callback(param_callback);
  }

  array<float, 2> TeleopTwistJoy::Impl::motionconverter(float gauche, float droit)
  {
    array<float, 2> X;                        // vecteur X
    X[0] = (gauche + droit);                  // calcul matriciel, [U] = r*[J]*[y] , donne Ux
    X[1] = 1 / base_width * (droit - gauche); // calcul matriciel, [U] = r*[J]*[y] , donne thetadot Z
    return X;
  }

  TeleopTwistJoy::~TeleopTwistJoy()
  {
    delete pimpl_;
  }

    
  double TeleopTwistJoy::Impl::getVal(const sensor_msgs::msg::Joy::SharedPtr joy_msg, const std::map<std::string, int64_t> &axis_map,
                                      const std::map<std::string, double> &scale_map, const std::string &fieldname,
                                      const std::map<std::string, double> &turbo_scale_map)
  {
    if (axis_map.find(fieldname) == axis_map.end() ||
        axis_map.at(fieldname) == -1L ||
        scale_map.find(fieldname) == scale_map.end() ||
        static_cast<int>(joy_msg->axes.size()) <= axis_map.at(fieldname))
    {
      return 0.0;
    }

    // Turbo is applied as a ramp based on the axis value
    double turbo_ramp = turbo_scale_map.at(fieldname) - scale_map.at(fieldname);
    double turbo_gain = (joy_msg->axes[turbo_axis] - 1) * -0.5 * turbo_ramp;

    return joy_msg->axes[axis_map.at(fieldname)] * (scale_map.at(fieldname) + turbo_gain);
  }

  void TeleopTwistJoy::Impl::sendCmdVelMsg(const sensor_msgs::msg::Joy::SharedPtr joy_msg,
                                           const std::string &which_map)  
  {
    // Initializes with zeros by default.
    auto cmd_vel_msg = std::make_unique<geometry_msgs::msg::Twist>();
    if (!track_mode)
    {
      cmd_vel_msg->linear.x = getVal(joy_msg, axis_linear_map, scale_linear_map[which_map], "x", scale_linear_map["turbo"]);
      cmd_vel_msg->linear.y = getVal(joy_msg, axis_linear_map, scale_linear_map[which_map], "y", scale_linear_map["turbo"]);
      cmd_vel_msg->linear.z = getVal(joy_msg, axis_linear_map, scale_linear_map[which_map], "z", scale_linear_map["turbo"]);
      cmd_vel_msg->angular.z = getVal(joy_msg, axis_angular_map, scale_angular_map[which_map], "yaw", scale_angular_map["turbo"]);
      cmd_vel_msg->angular.y = getVal(joy_msg, axis_angular_map, scale_angular_map[which_map], "pitch", scale_angular_map["turbo"]);
      cmd_vel_msg->angular.x = getVal(joy_msg, axis_angular_map, scale_angular_map[which_map], "roll", scale_angular_map["turbo"]);
    }

    else if (scale_linear_map[which_map].find("x") != scale_linear_map[which_map].end() &&
             scale_angular_map[which_map].find("yaw") != scale_angular_map[which_map].end())
    {
      float left_stick_value = joy_msg->axes[1] * scale_linear_map[which_map].at("x") / 2;
      float right_stick_value = joy_msg->axes[4] * scale_linear_map[which_map].at("x") / 2;
      float vel_x = motionconverter(left_stick_value, right_stick_value)[0];
      float angular_vel_z = motionconverter(left_stick_value, right_stick_value)[1];

      // if BOTH joy sticks are near default(0), send no message
      if ((left_stick_value < deadzone && left_stick_value > -deadzone) &&
          (right_stick_value < deadzone && right_stick_value > -deadzone))
      {
        cmd_vel_msg->linear.x = 0;
        cmd_vel_msg->angular.z = 0;
      }
      else
      {
        cmd_vel_msg->linear.x = vel_x;
        cmd_vel_msg->angular.z = angular_vel_z;
      }
    }

    cmd_vel_pub->publish(std::move(cmd_vel_msg));
  }

  void TeleopTwistJoy::Impl::joyCallback(const sensor_msgs::msg::Joy::SharedPtr joy_msg)
  {

    if (enable_axis >= 0 && enable_axis < static_cast<int>(joy_msg->axes.size()) &&
        joy_msg->axes[enable_axis] < 0)
    {
      sendCmdVelMsg(joy_msg, "normal");
    }

    if (require_autonomy_button)
    {
      // Publish autonomous allowed
      std_msgs::msg::Bool lock_autonomy_msg;
      lock_autonomy_msg.data = !joy_msg->buttons[autonomy_button];
      lock_autonomy_pub->publish(lock_autonomy_msg);
    }


    if (joy_msg->buttons[track_control_button])
    {
        if (!track_button_latch)
        {
            track_mode = !track_mode;
            track_button_latch = true;
        }
    }
    else
    {
        track_button_latch = false;
    }
  }

} // namespace teleop_twist_joy

RCLCPP_COMPONENTS_REGISTER_NODE(teleop_twist_joy::TeleopTwistJoy)
