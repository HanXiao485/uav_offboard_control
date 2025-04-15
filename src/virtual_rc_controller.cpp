#include <ros/ros.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>
#include <mavros_msgs/AttitudeTarget.h>
#include <geometry_msgs/Quaternion.h>
#include <tf2/LinearMath/Quaternion.h>

mavros_msgs::State current_state;
void state_cb(const mavros_msgs::State::ConstPtr& msg) {
    current_state = *msg;
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "offboard_controller");
    ros::NodeHandle nh;

    // 订阅MAVROS状态
    ros::Subscriber state_sub = nh.subscribe<mavros_msgs::State>
        ("mavros/state", 10, state_cb);
    
    // 发布姿态目标（包含推力）
    ros::Publisher attitude_pub = nh.advertise<mavros_msgs::AttitudeTarget>
        ("mavros/setpoint_raw/attitude", 10);
    
    // 服务客户端
    ros::ServiceClient arming_client = nh.serviceClient<mavros_msgs::CommandBool>
        ("mavros/cmd/arming");
    ros::ServiceClient set_mode_client = nh.serviceClient<mavros_msgs::SetMode>
        ("mavros/set_mode");

    // 控制频率20Hz
    ros::Rate rate(20.0);

    // 等待飞控连接
    while(ros::ok() && !current_state.connected) {
        ros::spinOnce();
        rate.sleep();
    }

    // 初始化姿态目标消息
    mavros_msgs::AttitudeTarget attitude_cmd;
    attitude_cmd.type_mask = 0;  // 全量控制
    attitude_cmd.thrust = 0.3;   // 初始推力50%

    // 创建四元数（示例：绕Y轴倾斜30度）
    tf2::Quaternion q;
    q.setRPY(0.0, 0.0, 0.0); // Roll=0°, Pitch=30°, Yaw=0°
    attitude_cmd.orientation.x = q.x();
    attitude_cmd.orientation.y = q.y();
    attitude_cmd.orientation.z = q.z();
    attitude_cmd.orientation.w = q.w();

    // 预发送设定点
    for(int i = 0; ros::ok() && i < 100; ++i){
        attitude_pub.publish(attitude_cmd);
        ros::spinOnce();
        rate.sleep();
    }
    ROS_INFO("3");

    // 设置Offboard模式
    mavros_msgs::SetMode offb_set_mode;
    offb_set_mode.request.custom_mode = "OFFBOARD";

    // 解锁指令
    mavros_msgs::CommandBool arm_cmd;
    arm_cmd.request.value = true;

    ros::Time last_request = ros::Time::now();

    while(ros::ok()) {
        // 模式切换
        if(current_state.mode != "OFFBOARD" && 
           (ros::Time::now() - last_request > ros::Duration(5.0))) {
            if(set_mode_client.call(offb_set_mode) &&
               offb_set_mode.response.mode_sent) {
                ROS_INFO("Offboard enabled");
            }
            last_request = ros::Time::now();
        } else {
            // 解锁检测
            if(!current_state.armed &&
               (ros::Time::now() - last_request > ros::Duration(5.0))) {
                if(arming_client.call(arm_cmd) &&
                   arm_cmd.response.success) {
                    ROS_INFO("Vehicle armed");
                }
                last_request = ros::Time::now();
            }
        }

        // 持续发布控制指令
        attitude_pub.publish(attitude_cmd);
        ros::spinOnce();
        rate.sleep();
    }
    return 0;
}