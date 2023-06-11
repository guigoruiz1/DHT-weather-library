#ifndef Weather_h
#define Weather_h

#include <DHT.h>
#include <DHT_U.h>

enum PerceptionState {
  Perception_Dry = 0,
  Perception_VeryComfy = 1,
  Perception_Comfy = 2,
  Perception_Ok = 3,
  Perception_UnComfy = 4,
  Perception_QuiteUnComfy = 5,
  Perception_VeryUnComfy = 6,
  Perception_SevereUncomfy = 7
};

enum ComfortState {
  Comfort_OK = 0,
  Comfort_TooHot = 1,
  Comfort_TooCold = 2,
  Comfort_TooDry = 4,
  Comfort_TooHumid = 8,
  Comfort_HotAndHumid = 9,
  Comfort_HotAndDry = 5,
  Comfort_ColdAndHumid = 10,
  Comfort_ColdAndDry = 6
};

struct Comfort {
  ComfortState state;
  float ratio;
};

class Weather {
public:
  Weather(uint8_t pin, uint8_t type);

  float toFahrenheit(float fromCelcius);
  float toCelsius(float fromFahrenheit);
  bool isTooHot(float temperature, float humidity);
  bool isTooHumid(float temperature, float humidity);
  bool isTooCold(float temperature, float humidity);
  bool isTooDry(float temperature, float humidity);
  float computeHeatIndex(float temperature, float humidity);
  float computeDewPoint(float temperature, float percentHumidity, bool isFahrenheit = false);
  float computeAbsoluteHumidity(float temperature, float percentHumidity, bool isFahrenheit = false);
  byte computePerception(float temperature, float percentHumidity, bool isFahrenheit = false);
  Comfort computeComfort(float temperature, float percentHumidity, bool isFahrenheit = false);
  void begin();
  DHT_Unified::Temperature temperature();
  DHT_Unified::Humidity humidity();

  struct ComfortProfile {
    float m_tooHot_m;
    float m_tooHot_b;
    float m_tooHumid_m;
    float m_tooHumid_b;
    float m_tooCold_m;
    float m_tooCold_b;
    float m_tooDry_m;
    float m_tooDry_b;

    bool isTooHot(float temperature, float humidity);
    bool isTooHumid(float temperature, float humidity);
    bool isTooCold(float temperature, float humidity);
    bool isTooDry(float temperature, float humidity);
    float distanceTooHot(float temperature, float humidity);
    float distanceTooHumid(float temperature, float humidity);
    float distanceTooCold(float temperature, float humidity);
    float distanceTooDry(float temperature, float humidity);
  };

  ComfortProfile comfortProfile;

private:
  DHT _dht;
  DHT_Unified _dht_u;
};

#endif
