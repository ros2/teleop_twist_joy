^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package teleop_twist_joy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

2.4.7 (2024-09-09)
------------------
* Update the launch file to work with modern joy. (`#52 <https://github.com/ros2/teleop_twist_joy/issues/52>`_) (`#53 <https://github.com/ros2/teleop_twist_joy/issues/53>`_)
  It should use "device_id" (not "dev") as the parameter,
  and the parameter should be a number, not a path (this is
  effectively the SDL device number, which is cross-platform).
  (cherry picked from commit e4856e4afeda704547bdb43edc29d008b07f15d9)
  # Conflicts:
  #	README.md
  Co-authored-by: Chris Lalancette <clalancette@gmail.com>
* Contributors: mergify[bot]

2.4.6 (2024-06-17)
------------------
* add inverted reverse param (`#35 <https://github.com/ros2/teleop_twist_joy/issues/35>`_) (`#43 <https://github.com/ros2/teleop_twist_joy/issues/43>`_)
  * add inverted-reverse param
  (cherry picked from commit 2a5f3e4f776869ae1e981f3ca1877cdf10318f37)
  # Conflicts:
  #	src/teleop_twist_joy.cpp
  Co-authored-by: Máté <56221639+turtlewizard73@users.noreply.github.com>
  Co-authored-by: Alejandro Hernández Cordero <ahcorde@gmail.com>
* Add an option to publish TwistStamped (backport `#42 <https://github.com/ros2/teleop_twist_joy/issues/42>`_) (`#45 <https://github.com/ros2/teleop_twist_joy/issues/45>`_)
  * Add an option to publish TwistStamped (`#42 <https://github.com/ros2/teleop_twist_joy/issues/42>`_)
  (cherry picked from commit 76cd6508a8c4e35d9fe3a6a8968abbe7159ffc08)
  # Conflicts:
  #	README.md
  #	src/teleop_twist_joy.cpp
  * resolved merge conflicts (`#47 <https://github.com/ros2/teleop_twist_joy/issues/47>`_)
  Co-authored-by: Yannic Bachmann <yannic.bachmann@maha.de>
  ---------
  Co-authored-by: Tamaki Nishino <otamachan@gmail.com>
  Co-authored-by: Yannic Bachmann <76436302+YBachmann@users.noreply.github.com>
  Co-authored-by: Yannic Bachmann <yannic.bachmann@maha.de>
  Co-authored-by: Alejandro Hernández Cordero <ahcorde@gmail.com>
* Added Humble CI (`#50 <https://github.com/ros2/teleop_twist_joy/issues/50>`_)
* Contributors: Alejandro Hernández Cordero, mergify[bot]

2.4.5 (2023-06-08)
------------------
* Cleanup CMakeLists.txt. (`#36 <https://github.com/ros2/teleop_twist_joy/issues/36>`_)
* Contributors: Chris Lalancette

2.4.4 (2023-06-07)
------------------
* Install includes to include/${PROJECT_NAME} (`#30 <https://github.com/ros2/teleop_twist_joy/issues/30>`_)
* joy_vel argument (`#29 <https://github.com/ros2/teleop_twist_joy/issues/29>`_)
* Contributors: Raffaello Bonghi, Shane Loretz

2.4.3 (2021-08-02)
------------------
* Fix the launch file to use 'executable'. (`#28 <https://github.com/ros2/teleop_twist_joy/issues/28>`_)
* fix launch notation (`#26 <https://github.com/ros2/teleop_twist_joy/issues/26>`_)
* Contributors: Chris Lalancette, Shigeki Kobayashi

2.4.2 (2021-03-18)
------------------
* Update README to reflect changes to config parameters. (fixes `#23 <https://github.com/ros2/teleop_twist_joy/issues/23>`_) (`#24 <https://github.com/ros2/teleop_twist_joy/issues/24>`_)
* Contributors: Josh Newans

2.4.1 (2020-12-01)
------------------
* Add parameter to enable/disable requiring the enable button to be held for motion (`#21 <https://github.com/ros2/teleop_twist_joy/issues/21>`__)
* Contributors: Chris Lalancette, kgibsonjca

2.4.0 (2020-11-09)
------------------
* Switch to modern ReadyToTest for the tests.
* Switch from node_executable -> executable for Foxy.
* Update README for Ros2 (`#17 <https://github.com/ros2/teleop_twist_joy/issues/17>`_) (`#18 <https://github.com/ros2/teleop_twist_joy/issues/18>`_)
* Contributors: Chris Lalancette, nfry321

2.3.0 (2020-08-05)
------------------
* Make Parameters dynamic (`#16 <https://github.com/ros2/teleop_twist_joy/issues/16>`_)
* Contributors: aravindsrj

2.2.2 (2019-10-23)
------------------
* Export interfaces for Shared Lib on Windows.
* Make teleop_twist_joy composable.
* Reenable cppcheck.
* Rename teleop_twist_joy.h to teleop_twist_joy.hpp
* Get some basic tests running (`#10 <https://github.com/ros2/teleop_twist_joy/issues/10>`_)
* Port config and launch to ROS 2 (`#11 <https://github.com/ros2/teleop_twist_joy/issues/11>`_)
* Contributors: Chris Lalancette, Scott K Logan, seanyen

2.2.0 (2019-05-31)
------------------
* Fix parameters so things actually work in Dashing. (`#9 <https://github.com/ros2/teleop_twist_joy/issues/9>`_)
* Contributors: Chris Lalancette

2.1.1 (2019-02-08)
------------------
* Add in the ability to control via parameters (`#8 <https://github.com/ros2/teleop_twist_joy/issues/8>`_)
* Contributors: Chris Lalancette

2.1.0 (2018-06-26)
------------------
* ParameterService auto started (`#7 <https://github.com/ros2/teleop_twist_joy/issues/7>`_)
* Contributors: Shane Loretz

2.0.0 (2017-12-08)
------------------
* Initial port to ROS2
* Contributors: Chris Lalancette, Mikael Arguedas, Deanna Hood

0.1.2 (2016-08-31)
------------------
* Fixed incorrect key. (`#21 <https://github.com/ros-teleop/teleop_twist_joy/issues/21>`__)
* Allow custom config file from location outside of this package
* Setting scale_angular_turbo if axis_angular is set so that turning works when turbo is pressed.
* Added turbo scale for angular velocities and accompanying test.
* Add LICENSE.txt.
* Contributors: Daniel Aden, Isaac I.Y. Saito, Mike Purvis, Tony Baltovski

0.1.1 (2015-06-27)
------------------
* Add rostests.
* Added maps to allow multi-dof velocity publishing.
* Added Xbox 360 controller example.
* Contributors: Mike Purvis, Tony Baltovski

0.1.0 (2014-07-25)
------------------
* Added configurations for Logitech Attack3 and Extreme 3D Pro joysticks.
* Initial version, with example config for PS3 joystick.
* Contributors: Mike Purvis, Tony Baltovski
