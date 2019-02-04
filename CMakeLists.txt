cmake_minimum_required(VERSION 3.5)
project(teleop_twist_joy)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

include_directories(include)

add_library(${PROJECT_NAME} src/${PROJECT_NAME})

ament_target_dependencies(${PROJECT_NAME}
  "geometry_msgs"
  "rclcpp"
  "sensor_msgs"
)

add_executable(${PROJECT_NAME}_node src/teleop_node.cpp)
target_link_libraries(${PROJECT_NAME}_node ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}_node
  PROPERTIES OUTPUT_NAME teleop_node PREFIX "")

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

install(TARGETS ${PROJECT_NAME}_node
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include)

#install(DIRECTORY launch config
#  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

if(BUILD_TESTING)
  # find_package(roslint REQUIRED)
  # roslint_cpp()
  # roslint_python()
  # roslint_add_test()

  # find_package(roslaunch REQUIRED)
  # roslaunch_add_file_check(launch/teleop.launch)

  # find_package(rostest REQUIRED)
  # include_directories(include ${catkin_INCLUDE_DIRS})

  # # Check axes and scaling.
  # add_rostest(test/differential_joy.test)
  # add_rostest(test/holonomic_joy.test)
  # add_rostest(test/six_dof_joy.test)

  # # Check enable and turbo button logic.
  # add_rostest(test/no_enable_joy.test)
  # add_rostest(test/turbo_enable_joy.test)
  # add_rostest(test/only_turbo_joy.test)
  # add_rostest(test/turbo_angular_enable_joy.test)
  # add_rostest(test/turbo_angular_enable_joy_with_rosparam_map.test)
endif()

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_package()
