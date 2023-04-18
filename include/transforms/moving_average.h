#ifndef _moving_average_H_
#define _moving_average_H_

#include <vector>

#include "sensesp/transforms/transform.h"

/**
 * @brief Outputs the moving average of the last sample_size inputs.
 *
 * Used to smooth the output of a value (signal) that has
 * frequent variations. For example, the output of a temperature sensor may vary
 * from 180 to 185 several times over a short period, but you just want to see
 * the average of that. MovingAverage outputs the average of the most recent
 * sample_size values. It also incorporates a "scale" factor, in case you want
 * to increase or decrease your final output by a fixed percentage.
 */

// y = k * 1/n * \sum_k=1^n(x_k)
template <typename T, typename U>
class MovingAverage : public sensesp::SymmetricTransform<T> {
 public:
  /**
   * @param sample_size The number of most recent values you want to average for
   * your output.
   *
   * @param multiplier Moving average will be multiplied by multiplier before it
   * is output - make it something other than 1. if you need to scale your
   * output up or down by a fixed percentage.
   *
   * @param config_path The path used to configure this transform in the Config
   * UI.
   * */
  MovingAverage(int sample_size, U multiplier = 1.0, String config_path = "");
  virtual void set_input(T input, uint8_t inputChannel = 0) override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  std::vector<T> buf_;
  int ptr_ = 0;
  int sample_size_;
  U multiplier_;
  bool initialized_;
};

template <typename T, typename U>
MovingAverage<T, U>::MovingAverage(int sample_size, U multiplier,
                                String config_path)
    : sensesp::SymmetricTransform<T>(config_path),
      sample_size_{sample_size},
      multiplier_{multiplier} {
  buf_.resize(sample_size_, 0);
  initialized_ = false;
  this->load_configuration();
}

template <typename T, typename U>
void MovingAverage<T, U>::set_input(T input, uint8_t inputChannel) {
  // So the first value to be included in the average doesn't default to 0.0
  if (!initialized_) {
    buf_.assign(sample_size_, input);
    this->output = input;
    initialized_ = true;
  } else {
    // Subtract 1/nth of the oldest value and add 1/nth of the newest value
    this->output += -multiplier_ * buf_[ptr_] / sample_size_;
    this->output += multiplier_ * input / sample_size_;

    // Save the most recent input, then advance to the next storage location.
    // When storage location n is reached, start over again at 0.
    buf_[ptr_] = input;
    ptr_ = (ptr_ + 1) % sample_size_;
  }
  this->notify();
}

template <typename T, typename U>
void MovingAverage<T, U>::get_configuration(JsonObject& root) {
  root["multiplier"] = multiplier_;
  root["sample_size"] = sample_size_;
}

template <typename T, typename U>
String MovingAverage<T, U>::get_config_schema() {
  const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "sample_size": { "title": "Number of samples in average", "type": "integer" },
        "multiplier": { "title": "Multiplier", "type": "number" }
    }
  })";
  return FPSTR(SCHEMA);
}

template <typename T, typename U>
bool MovingAverage<T, U>::set_configuration(const JsonObject& config) {
  String expected[] = {"multiplier", "sample_size"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  multiplier_ = config["multiplier"];
  int n_new = config["sample_size"];
  // need to reset the ring buffer if size changes
  if (sample_size_ != n_new) {
    buf_.assign(sample_size_, 0);
    ptr_ = 0;
    initialized_ = false;
    sample_size_ = n_new;
  }
  return true;
}

#endif
