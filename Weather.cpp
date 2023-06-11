#include "Weather.h"

Weather::Weather(uint8_t pin, uint8_t type)
  : _dht(pin, type), _dht_u(pin, type) {}

float Weather::toFahrenheit(float fromCelcius) {
  return _dht.convertCtoF(fromCelcius);
}

float Weather::toCelsius(float fromFahrenheit) {
  return _dht.convertFtoC(fromFahrenheit);
}

bool Weather::isTooHot(float temperature, float humidity) {
  return comfortProfile.isTooHot(temperature, humidity);
}

bool Weather::isTooHumid(float temperature, float humidity) {
  return comfortProfile.isTooHumid(temperature, humidity);
}

bool Weather::isTooCold(float temperature, float humidity) {
  return comfortProfile.isTooCold(temperature, humidity);
}

bool Weather::isTooDry(float temperature, float humidity) {
  return comfortProfile.isTooDry(temperature, humidity);
}

float Weather::computeHeatIndex(float temperature, float humidity) {
  return _dht.computeHeatIndex(temperature, humidity);
}

float Weather::computeDewPoint(float temperature, float percentHumidity, bool isFahrenheit) {
  // reference: http://wahiduddin.net/calc/density_algorithms.htm
  if (isFahrenheit) {
    temperature = toCelsius(temperature);
  }
  double calc_temp = 373.15 / (273.15 + (double)temperature);
  double calc_sum = -7.90298 * (calc_temp - 1);
  calc_sum += 5.02808 * log10(calc_temp);
  calc_sum += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / calc_temp))) - 1);
  calc_sum += 8.1328e-3 * (pow(10, (-3.49149 * (calc_temp - 1))) - 1);
  calc_sum += log10(1013.246);
  double calc_value = pow(10, calc_sum - 3) * (double)percentHumidity;
  double calc_dew_temp = log(calc_value / 0.61078);  // temperature var
  calc_dew_temp = (241.88 * calc_dew_temp) / (17.558 - calc_dew_temp);
  return isFahrenheit ? toCelsius(calc_dew_temp) : calc_dew_temp;
}

float Weather::computeAbsoluteHumidity(float temperature, float percentHumidity, bool isFahrenheit) {
  // Calculate the absolute humidity in g/m³
  // https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/
  if (isFahrenheit) {
    temperature = toCelsius(temperature);
  }

  float absHumidity;
  float absTemperature;
  absTemperature = temperature + 273.15;

  absHumidity = 6.112;
  absHumidity *= exp((17.67 * temperature) / (243.5 + temperature));
  absHumidity *= percentHumidity;
  absHumidity *= 2.1674;
  absHumidity /= absTemperature;

  return absHumidity;
}

byte Weather::computePerception(float temperature, float percentHumidity, bool isFahrenheit) {
// Computing human perception from dew point
  // reference: https://en.wikipedia.org/wiki/Dew_point ==> Relationship to human comfort
  // reference: Horstmeyer, Steve (2006-08-15). "Relative Humidity....Relative to What? The Dew Point Temperature...a better approach". Steve Horstmeyer, Meteorologist, WKRC TV, Cincinnati, Ohio, USA. Retrieved 2009-08-20.
  // Using table
  // Return value Dew point    Human perception[6]
  //    7         Over 26 °C   Severely high, even deadly for asthma related illnesses
  //    6         24–26 °C     Extremely uncomfortable, oppressive
  //    5         21–24 °C     Very humid, quite uncomfortable
  //    4         18–21 °C     Somewhat uncomfortable for most people at upper edge
  //    3         16–18 °C     OK for most, but all perceive the humidity at upper edge
  //    2         13–16 °C     Comfortable
  //    1         10–12 °C     Very comfortable
  //    0         Under 10 °C  A bit dry for some

  if (isFahrenheit) {
    temperature = toCelsius(temperature);
  }
  float dewPoint = computeDewPoint(temperature, percentHumidity);

  if (dewPoint < 10.0f) {
    return Perception_Dry;
  } else if (dewPoint < 13.0f) {
    return Perception_VeryComfy;
  } else if (dewPoint < 16.0f) {
    return Perception_Comfy;
  } else if (dewPoint < 18.0f) {
    return Perception_Ok;
  } else if (dewPoint < 21.0f) {
    return Perception_UnComfy;
  } else if (dewPoint < 24.0f) {
    return Perception_QuiteUnComfy;
  } else if (dewPoint < 26.0f) {
    return Perception_VeryUnComfy;
  }
  // else dew >= 26.0
  return Perception_SevereUncomfy;
}

Comfort Weather::computeComfort(float temperature, float percentHumidity, bool isFahrenheit) {
  float ratio = 100;  //100%
  float distance = 0;
  float kTempFactor = 3;     //take into account the slope of the lines
  float kHumidFactor = 0.1;  //take into account the slope of the lines
  uint8_t tempComfort = 0;

  if (isFahrenheit) {
    temperature = toCelsius(temperature);
  }

  distance = comfortProfile.distanceTooHot(temperature, percentHumidity);
  if (distance > 0) {
    //update the comfort descriptor
    tempComfort += (uint8_t)Comfort_TooHot;
    //decrease the comfot ratio taking the distance into account
    ratio -= distance * kTempFactor;
  }

  distance = comfortProfile.distanceTooHumid(temperature, percentHumidity);
  if (distance > 0) {
    //update the comfort descriptor
    tempComfort += (uint8_t)Comfort_TooHumid;
    //decrease the comfot ratio taking the distance into account
    ratio -= distance * kHumidFactor;
  }

  distance = comfortProfile.distanceTooCold(temperature, percentHumidity);
  if (distance > 0) {
    //update the comfort descriptor
    tempComfort += (uint8_t)Comfort_TooCold;
    //decrease the comfot ratio taking the distance into account
    ratio -= distance * kTempFactor;
  }

  distance = comfortProfile.distanceTooDry(temperature, percentHumidity);
  if (distance > 0) {
    //update the comfort descriptor
    tempComfort += (uint8_t)Comfort_TooDry;
    //decrease the comfot ratio taking the distance into account
    ratio -= distance * kHumidFactor;
  }

  if (ratio < 0)
    ratio = 0;

  Comfort comfort = {
    .state = (ComfortState)tempComfort,
    .ratio = ratio
  };

  return comfort;
}

void Weather::begin() {
  _dht_u.begin();
}

DHT_Unified::Temperature Weather::temperature() {
  return _dht_u.temperature();
}

DHT_Unified::Humidity Weather::humidity() {
  return _dht_u.humidity();
}

bool Weather::ComfortProfile::isTooHot(float temperature, float humidity) {
  return (temperature > (humidity * m_tooHot_m + m_tooHot_b));
}

bool Weather::ComfortProfile::isTooHumid(float temperature, float humidity) {
  return (temperature > (humidity * m_tooHumid_m + m_tooHumid_b));
}

bool Weather::ComfortProfile::isTooCold(float temperature, float humidity) {
  return (temperature < (humidity * m_tooCold_m + m_tooCold_b));
}

bool Weather::ComfortProfile::isTooDry(float temperature, float humidity) {
  return (temperature < (humidity * m_tooDry_m + m_tooDry_b));
}

float Weather::ComfortProfile::distanceTooHot(float temperature, float humidity) {
  return temperature - (humidity * m_tooHot_m + m_tooHot_b);
}

float Weather::ComfortProfile::distanceTooHumid(float temperature, float humidity) {
  return temperature - (humidity * m_tooHumid_m + m_tooHumid_b);
}

float Weather::ComfortProfile::distanceTooCold(float temperature, float humidity) {
  return (humidity * m_tooCold_m + m_tooCold_b) - temperature;
}

float Weather::ComfortProfile::distanceTooDry(float temperature, float humidity) {
  return (humidity * m_tooDry_m + m_tooDry_b) - temperature;
}
