// #include <ros/ros.h>
// #include <mavros_msgs/State.h>
// #include <mavros_msgs/SetMode.h>
// #include <mavros_msgs/CommandBool.h>
// #include <mavros_msgs/RCIn.h>
// #include <geometry_msgs/PoseStamped.h>
// #include "uav_keyboard_control/keyboard_handler.h"

// mavros_msgs::State current_state;
// void state_cb(const mavros_msgs::State::ConstPtr& msg) {
//     current_state = *msg;
// }

// int main(int argc, char **argv) {
//     ros::init(argc, argv, "keyboard_control");
//     ros::NodeHandle nh;

//     // MAVROS接口初始化
//     ros::Subscriber state_sub = nh.subscribe<mavros_msgs::State>
//         ("mavros/state", 10, state_cb);
//     ros::Publisher local_pos_pub = nh.advertise<geometry_msgs::PoseStamped>
//         ("mavros/setpoint_position/local", 10);
//     ros::Publisher RC_control = nh.advertise<mavros_msgs::RCIn>
//         ("mavros/rc/in", 10);
//     ros::ServiceClient arming_client = nh.serviceClient<mavros_msgs::CommandBool>
//         ("mavros/cmd/arming");
//     ros::ServiceClient set_mode_client = nh.serviceClient<mavros_msgs::SetMode>
//         ("mavros/set_mode");

//     // 键盘初始化
//     KeyboardHandler kb;
//     char input_key;
//     geometry_msgs::PoseStamped pose;
//     pose.pose.position.z = 2.0; // 默认起飞高度

//     // 主循环
//     while(ros::ok()) {
//         input_key = kb.getKey();
        
//         // 模式切换处理
//         if(input_key == 'o') { // OFFBOARD模式
//             mavros_msgs::SetMode offb_set_mode;
//             offb_set_mode.request.custom_mode = "OFFBOARD";
//             set_mode_client.call(offb_set_mode);
//         }
        
//         // 解锁处理
//         if(input_key == 'a') { // 解锁
//             mavros_msgs::CommandBool arm_cmd;
//             arm_cmd.request.value = true;
//             arming_client.call(arm_cmd);
//         }

//         // 方向控制（示例）
//         switch(input_key) {
//             case 'w': pose.pose.position.x += 1.0; break;
//             case 's': pose.pose.position.x -= 1.0; break;
//             case 'a': pose.pose.position.y += 1.0; break;
//             case 'd': pose.pose.position.y -= 1.0; break;
//             case 'q': pose.pose.position.z += 0.5; break;
//             case 'e': pose.pose.position.z -= 0.5; break;
//         }

//         local_pos_pub.publish(pose);
//         ros::spinOnce();
//         usleep(100000); // 100ms周期
//     }
//     return 0;
// }