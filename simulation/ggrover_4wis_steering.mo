model ggrover_4wis_steering "ggRover 4-Wheel Independent Steering & Chassis Dynamics"
  // Vehicle Parameters
  parameter Real mass = 2.5 "Chassis Mass (kg)";
  parameter Real wheelbase = 0.220 "Wheelbase (m)";
  parameter Real track_width = 0.240 "Track Width (m)";

  // Steering & Motor Inputs
  input Real servo_angle_FL "Front-Left Steering Angle (rad)";
  input Real servo_angle_FR "Front-Right Steering Angle (rad)";
  input Real motor_speed_L "Left Wheel Speed (rad/s)";
  input Real motor_speed_R "Right Wheel Speed (rad/s)";

  // System States
  Real v_x "Forward Velocity (m/s)";
  Real yaw_rate "Chassis Yaw Rate (rad/s)";

equation
  // Differential dynamics
  v_x = (motor_speed_L + motor_speed_R) * 0.0375 / 2.0;
  yaw_rate = (motor_speed_R - motor_speed_L) * 0.0375 / track_width;
end ggrover_4wis_steering;
