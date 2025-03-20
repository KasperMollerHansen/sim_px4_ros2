#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/passthrough.h>
#include <pcl_conversions/pcl_conversions.h>
#include <px4_msgs/msg/vehicle_odometry.hpp>
#include <cmath>

using namespace std;

class LidarFilterNode : public rclcpp::Node
{
public:
    LidarFilterNode() : Node("lidar_filter_node")
    {
        // Create a subscriber for the PointCloud2 topic
        cloud_subscriber_ = this->create_subscription<sensor_msgs::msg::PointCloud2>(
            "/isaac/lidar/raw/pointcloud", rclcpp::SensorDataQoS(),
            std::bind(&LidarFilterNode::pointCloudCallback, this, std::placeholders::_1));

        // Create a publisher for the filtered PointCloud2
        cloud_publisher_ = this->create_publisher<sensor_msgs::msg::PointCloud2>("/isaac/lidar/filtered/pointcloud", 10);

        // Create a subscriber for the vehicle odometry
        odometry_subscriber_ = this->create_subscription<px4_msgs::msg::VehicleOdometry>(
            "/fmu/out/vehicle_odometry",
            rclcpp::SystemDefaultsQoS(),
            std::bind(&LidarFilterNode::odometryCallback, this, std::placeholders::_1));   
    }

    private:
    void pointCloudCallback(const sensor_msgs::msg::PointCloud2::SharedPtr msg)
    {
        // // Check if the angular velocity is over the threshold
        // if (angular_velocity_magnitude_ > angular_velocity_threshold_)
        // {
        //     // If angular velocity is large, discard all points in the cloud
        //     pcl::PointCloud<pcl::PointXYZ>::Ptr filtered_cloud(new pcl::PointCloud<pcl::PointXYZ>);
        //     sensor_msgs::msg::PointCloud2 filtered_msg;
        //     pcl::toROSMsg(*filtered_cloud, filtered_msg);
        //     filtered_msg.header = msg->header;

        //     // Publish the empty cloud
        //     cloud_publisher_->publish(filtered_msg);
        //     return;
        // }

        // Publish the original PointCloud2 message if the velocity is within limits
        cloud_publisher_->publish(*msg);
    }

    void odometryCallback(const px4_msgs::msg::VehicleOdometry::SharedPtr msg)
    {
        // Update the current position from the odometry message
        current_position_.x = msg->position[1]; // Y
        current_position_.y = msg->position[0]; // X
        current_position_.z = -msg->position[2]; // -Z

        // Calculate the angular velocity magnitude
        float angular_velocity_magnitude = std::sqrt(std::pow(msg->angular_velocity[1], 2) + // Y
                                                    std::pow(msg->angular_velocity[0], 2) + // X
                                                    std::pow(msg->angular_velocity[2], 2)); // Z

        // Store the angular velocity magnitude
        angular_velocity_magnitude_ = angular_velocity_magnitude;
    }

    struct Position
    {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    } current_position_;

    float angular_velocity_magnitude_ = 0.0;
    const float angular_velocity_threshold_ = 0.5;

    rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr cloud_subscriber_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr cloud_publisher_;
    rclcpp::Subscription<px4_msgs::msg::VehicleOdometry>::SharedPtr odometry_subscriber_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<LidarFilterNode>());
    rclcpp::shutdown();
    return 0;
}
