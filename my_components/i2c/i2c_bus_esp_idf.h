#pragma once

#ifdef USE_ESP_IDF

#include "esphome/core/component.h"
#include "i2c_bus.h"
#include <driver/i2c_master.h>

namespace esphome {
namespace i2c {

enum RecoveryCode {
  RECOVERY_FAILED_SCL_LOW,
  RECOVERY_FAILED_SDA_LOW,
  RECOVERY_COMPLETED,
};

class IDFI2CBus : public InternalI2CBus, public Component {
 public:
  void setup() override;
  void dump_config() override;
  ErrorCode write_readv(uint8_t address, const uint8_t *write_buffer, size_t write_count, uint8_t *read_buffer,
                        size_t read_count) override;
  //##########################################################################
  //PREVIOUSLY float get_setup_priority() const override { return setup_priority::BUS; }
  float get_setup_priority() const override { return setup_priority::DATA; }


  void set_scan(bool scan) { this->scan_ = scan; }
  void set_sda_pin(uint8_t sda_pin) { this->sda_pin_ = sda_pin; }
  void set_sda_pullup_enabled(bool sda_pullup_enabled) { this->sda_pullup_enabled_ = sda_pullup_enabled; }
  void set_scl_pin(uint8_t scl_pin) { this->scl_pin_ = scl_pin; }
  void set_scl_pullup_enabled(bool scl_pullup_enabled) { this->scl_pullup_enabled_ = scl_pullup_enabled; }
  void set_frequency(uint32_t frequency) { this->frequency_ = frequency; }
  void set_timeout(uint32_t timeout) { this->timeout_ = timeout; }

  int get_port() const override { return this->port_; }

 private:
  void recover_();
  RecoveryCode recovery_result_{};

 protected:
  i2c_master_dev_handle_t dev_{};
  i2c_master_bus_handle_t bus_{};
  i2c_port_t port_{};
  uint8_t sda_pin_{};
  bool sda_pullup_enabled_{};
  uint8_t scl_pin_{};
  bool scl_pullup_enabled_{};
  uint32_t frequency_{};
  uint32_t timeout_ = 0;
  bool initialized_ = false;
};

}  // namespace i2c
}  // namespace esphome

#endif  // USE_ESP_IDF
