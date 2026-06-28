#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
#include <math.h>

struct Vec3
{
    float x, y, z;

    // Length
    float magnitude() const
    {
        return sqrt(x * x + y * y + z * z);
    }

    // Unit vector
    Vec3 normalized() const
    {
        float mag = magnitude();
        if (mag < 0.00001)
            return {0.0f, 0.0f, 0.0f};

        return *this / mag;
    }
    float operator*(const Vec3& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 operator*(float s) const
    {
        return {x * s, y * s, z * s};
    }

    Vec3 operator+(const Vec3& other) const
    {
        return {x + other.x, y + other.y, z + other.z};
    }

    Vec3 operator-(const Vec3& other) const
    {
        return {x - other.x, y - other.y, z - other.z};
    }

    Vec3 operator/(float s) const
    {
        return {x / s, y / s, z / s};
    }
};


// Clamp value to [-1, 1] before acos()
float clamp(float x)
{
  return constrain(x, -1.f, 1.f);
}
void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
  }
}

void loop()
{
  sensors_event_t a, ge, temp;
  mpu.getEvent(&a, &ge, &temp);
  /* Print out the values */
  Serial.print("AccelX:");
  Serial.print(a.acceleration.x);
  Serial.print(",\t");
  Serial.print("AccelY:");
  Serial.print(a.acceleration.y);
  Serial.print(",\t");
  Serial.print("AccelZ:");
  Serial.print(a.acceleration.z);
  Serial.print(",\t");

  // Calibration vectors
  Vec3 g0 = {10, 0, -1};        // Reading at 0°
  Vec3 g90 = {-0.3, -9.6, 0.7}; // Reading at 90°

  Vec3 u = g0.normalized();
  Vec3 v = g90.normalized();

  // Current accelerometer reading
  Vec3 g = {
      a.acceleration.x,
      a.acceleration.y,
      a.acceleration.z};

  Vec3 w = g.normalized();

  float total = acos(clamp(u * v));
  float current = acos(clamp(u * w));

  float angle = 90.0f * current / total;

  Serial.print("Angle: ");
  Serial.print(angle);
  Serial.print('\r');
  delay(100);
}