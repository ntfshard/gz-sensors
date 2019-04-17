/*
 * Copyright (C) 2019 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <ignition/msgs/magnetometer.pb.h>
#include <ignition/transport/Node.hh>

#include "ignition/sensors/SensorFactory.hh"
#include "ignition/sensors/MagnetometerSensor.hh"

using namespace ignition;
using namespace sensors;

/// \brief Private data for MagnetometerSensor
class ignition::sensors::MagnetometerSensorPrivate
{
  /// \brief node to create publisher
  public: transport::Node node;

  /// \brief publisher to publish magnetometer messages.
  public: transport::Node::Publisher pub;

  /// \brief true if Load() has been called and was successful
  public: bool initialized = false;

  /// \brief The latest field reading from the sensor, based on the world
  /// field and the sensor's current pose.
  public: ignition::math::Vector3d localField;

  /// \brief Store world magnetic field vector. We assume it is uniform
  /// everywhere in the world, and that it doesn't change during the simulation.
  public: ignition::math::Vector3d worldField;

  /// \brief World pose of the magnetometer
  public: ignition::math::Pose3d worldPose;
};

//////////////////////////////////////////////////
MagnetometerSensor::MagnetometerSensor()
  : dataPtr(new MagnetometerSensorPrivate())
{
}

//////////////////////////////////////////////////
MagnetometerSensor::~MagnetometerSensor()
{
}

//////////////////////////////////////////////////
bool MagnetometerSensor::Init()
{
  return this->Sensor::Init();
}

//////////////////////////////////////////////////
bool MagnetometerSensor::Load(const sdf::Sensor &_sdf)
{
  if (!Sensor::Load(_sdf))
    return false;

  std::string topic = this->Topic();
  std::cout << "TOPIC[" << topic << "]\n";
  if (topic.empty())
    topic = "/magnetometer";

  this->dataPtr->pub =
      this->dataPtr->node.Advertise<ignition::msgs::Magnetometer>(topic);

  if (!this->dataPtr->pub)
    return false;

  this->dataPtr->initialized = true;
  return true;
}

//////////////////////////////////////////////////
bool MagnetometerSensor::Load(sdf::ElementPtr _sdf)
{
  if (!Sensor::Load(_sdf))
    return false;

  std::string topic = this->Topic();
  if (topic.empty())
    topic = "/magnetometer";

  this->dataPtr->pub =
      this->dataPtr->node.Advertise<ignition::msgs::Magnetometer>(topic);

  if (!this->dataPtr->pub)
    return false;

  this->dataPtr->initialized = true;
  return true;
}

//////////////////////////////////////////////////
bool MagnetometerSensor::Update(const ignition::common::Time &_now)
{
  if (!this->dataPtr->initialized)
  {
    ignerr << "Not initialized, update ignored.\n";
    return false;
  }

  // compute magnetic field in body frame
  this->dataPtr->localField =
      this->dataPtr->worldPose.Rot().Inverse().RotateVector(
      this->dataPtr->worldField);

  msgs::Magnetometer msg;
  msg.mutable_header()->mutable_stamp()->set_sec(_now.sec);
  msg.mutable_header()->mutable_stamp()->set_nsec(_now.nsec);
  msgs::Set(msg.mutable_field_tesla(), this->dataPtr->localField);

  // publish
  this->dataPtr->pub.Publish(msg);

  return true;
}

//////////////////////////////////////////////////
void MagnetometerSensor::SetWorldPose(const math::Pose3d _pose)
{
  this->dataPtr->worldPose = _pose;
}

//////////////////////////////////////////////////
math::Pose3d MagnetometerSensor::WorldPose() const
{
  return this->dataPtr->worldPose;
}

//////////////////////////////////////////////////
void MagnetometerSensor::SetWorldMagneticField(const math::Vector3d &_field)
{
  this->dataPtr->worldField = _field;
}

//////////////////////////////////////////////////
math::Vector3d MagnetometerSensor::WorldMagneticField() const
{
  return this->dataPtr->worldField;
}

//////////////////////////////////////////////////
math::Vector3d MagnetometerSensor::MagneticField() const
{
  return this->dataPtr->localField;
}

IGN_SENSORS_REGISTER_SENSOR(MagnetometerSensor)
