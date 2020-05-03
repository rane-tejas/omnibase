#include <omnibase_control/controller.h>
#include <ros/ros.h>


controller::controller(ros::NodeHandle& nh, ros::NodeHandle& nh_private):nh_(nh), nh_private_(nh_private)
{
	init_vars();
	get_params();

	vel_sub = nh_.subscribe("cmd_vel", 5, &controller::cmd_vel_cb, this);

	front_right_cmd_pub = nh_.advertise<std_msgs::Float64>("/omnibase/front_right_joint_velocity_controller/command",10);
	front_left_cmd_pub = nh_.advertise<std_msgs::Float64>("/omnibase/front_left_joint_velocity_controller/command",10);
	back_right_cmd_pub = nh_.advertise<std_msgs::Float64>("/omnibase/back_right_joint_velocity_controller/command",10);
	back_left_cmd_pub = nh_.advertise<std_msgs::Float64>("/omnibase/back_left_joint_velocity_controller/command",10);

	stop_bot();

	ROS_INFO("-----------------------------------------");
	ROS_INFO("         Controller Initialized");
	ROS_INFO("-----------------------------------------");
}


void controller::init_vars()
{
	omega_1.data = 0;
	omega_2.data = 0;
	omega_3.data = 0;
	omega_4.data = 0;

	x_dot = 0;
	y_dot = 0;
}


void controller::cmd_vel_cb(const geometry_msgs::Twist &vel)
{	
	if ((vel.linear.x < max_lin_vel) && (vel.linear.x > -max_lin_vel))
		x_dot = vel.linear.x;
	else if (vel.linear.x > max_lin_vel)
		x_dot = max_lin_vel;
	else
		x_dot = -max_lin_vel;

	if ((vel.linear.y < max_lin_vel) && (vel.linear.y > -max_lin_vel))
		y_dot = vel.linear.y;
	else if (vel.linear.y > max_lin_vel)
		y_dot = max_lin_vel;
	else
		y_dot = -max_lin_vel;


	// Converting the velocity to wheel velocity
	if (vel.angular.z == 0){
		omega_1.data = vel_mul * (y_dot + x_dot);
		omega_2.data = vel_mul * (-x_dot + y_dot);
		omega_3.data = vel_mul * (x_dot - y_dot);
		omega_4.data = vel_mul * (-x_dot - y_dot);
	}
	else
	{
		omega_1.data = vel_mul * vel.angular.z;
		omega_2.data = vel_mul * vel.angular.z;
		omega_3.data = vel_mul * vel.angular.z;
		omega_4.data = vel_mul * vel.angular.z;
	}

	vel_publish();
}

void controller::vel_publish()
{	
	front_right_cmd_pub.publish(omega_1);
	front_left_cmd_pub.publish(omega_2);
	back_right_cmd_pub.publish(omega_3);
	back_left_cmd_pub.publish(omega_4);
}

void controller::stop_bot()
{	
	for (int i = 0; i < 100; i++){
		front_right_cmd_pub.publish(omega_1);
		front_left_cmd_pub.publish(omega_2);
		back_right_cmd_pub.publish(omega_3);
		back_left_cmd_pub.publish(omega_4);
	}
}

void controller::get_params()
{
	if (nh_.hasParam("max_linear_velocity"))
		nh_.getParam("max_linear_velocity", max_lin_vel);
	else
		max_lin_vel = 2;

	if (nh_.hasParam("max_angular_velocity"))
		nh_.getParam("max_angular_velocity", max_angular_vel);
	else
		max_angular_vel = 2;

	if (nh_.hasParam("vel_mul_constant")) 
		nh_.getParam("vel_mul_constant", vel_mul);
	else
		vel_mul = 2;
}