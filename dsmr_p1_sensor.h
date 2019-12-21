#include "esphome.h"
#include "dsmr.h"

// * Parsed Data
using MyData = ParsedData<
  /* String */ identification,
  /* String */ p1_version,
  /* String */ timestamp,
  /* String */ equipment_id,
  /* FixedValue */ energy_delivered_tariff1,
  /* FixedValue */ energy_delivered_tariff2,
  /* FixedValue */ energy_returned_tariff1,
  /* FixedValue */ energy_returned_tariff2,
  /* String */ electricity_tariff,
  /* FixedValue */ power_delivered,
  /* FixedValue */ power_returned,
  /* FixedValue */ electricity_threshold,
  /* uint8_t */ electricity_switch_position,
  /* uint32_t */ electricity_failures,
  /* uint32_t */ electricity_long_failures,
  /* String */ electricity_failure_log,
  /* uint32_t */ electricity_sags_l1,
  /* uint32_t */ electricity_sags_l2,
  /* uint32_t */ electricity_sags_l3,
  /* uint32_t */ electricity_swells_l1,
  /* uint32_t */ electricity_swells_l2,
  /* uint32_t */ electricity_swells_l3,
  /* String */ message_short,
  /* String */ message_long,
  /* FixedValue */ voltage_l1,
  /* FixedValue */ voltage_l2,
  /* FixedValue */ voltage_l3,
  /* FixedValue */ current_l1,
  /* FixedValue */ current_l2,
  /* FixedValue */ current_l3,
  /* FixedValue */ power_delivered_l1,
  /* FixedValue */ power_delivered_l2,
  /* FixedValue */ power_delivered_l3,
  /* FixedValue */ power_returned_l1,
  /* FixedValue */ power_returned_l2,
  /* FixedValue */ power_returned_l3,
  /* uint16_t */ gas_device_type,
  /* String */ gas_equipment_id,
  /* uint8_t */ gas_valve_position,
  /* TimestampedFixedValue */ gas_delivered,
  /* uint16_t */ thermal_device_type,
  /* String */ thermal_equipment_id,
  /* uint8_t */ thermal_valve_position,
  /* TimestampedFixedValue */ thermal_delivered,
  /* uint16_t */ water_device_type,
  /* String */ water_equipment_id,
  /* uint8_t */ water_valve_position,
  /* TimestampedFixedValue */ water_delivered,
  /* uint16_t */ slave_device_type,
  /* String */ slave_equipment_id,
  /* uint8_t */ slave_valve_position,
  /* TimestampedFixedValue */ slave_delivered
>;

uint8_t req_pin = 5;
unsigned long last;
bool readmessage = false;

P1Reader reader(&Serial,2);

struct Printer {
  template<typename Item>
  void apply(Item &i) {
    if (i.present()) {
      ESP_LOGD("DmsrCustom","%s",Item::name);
      Serial.print(Item::name);
      Serial.print(F(": "));
      Serial.print(i.val());
//      ESP_LOGD("DmsrCustom","%u",i.val());
      Serial.print(Item::unit());
      ESP_LOGD("DmsrCustom","%s",Item::unit());
      Serial.println();
    }
  }
};

class DsmrP1CustomSensor : public PollingComponent, public UARTDevice {
 public:
  DsmrP1CustomSensor(UARTComponent *parent) : UARTDevice(parent) {}

  Sensor *consumption_low_tarif_sensor = new Sensor();
  Sensor *consumption_high_tarif_sensor = new Sensor();
  Sensor *actual_consumption_sensor = new Sensor();
  Sensor *instant_power_current_sensor = new Sensor();
  Sensor *instant_power_usage_sensor = new Sensor();
  Sensor *gas_meter_m3_sensor = new Sensor();
  Sensor *actual_tarif_sensor = new Sensor();
  Sensor *short_power_outages_sensor = new Sensor();
  Sensor *long_power_outages_sensor = new Sensor();
  Sensor *short_power_drops_sensor = new Sensor();
  Sensor *short_power_peaks_sensor = new Sensor();


  void setup() override {
        MyData data;
        pinMode(D5, OUTPUT);
        digitalWrite(D5,LOW);
        reader.enable(true);
        last = millis();
	readmessage = false;
  }

  void update() override {
    // Allow the reader to check the serial buffer regularly
  reader.loop();
  digitalWrite(D5,HIGH);
  // Every minute, fire off a one-off reading
  unsigned long now = millis();
  if (now - last > 15000) {
    reader.enable(true);
    last = now;
    readmessage = false;
  }

  if (available()) {
    MyData data;
    String err;
    //ESP_LOGD("DmsrCustom","READING....");
    if (reader.parse(&data, &err)) {
      // Parse succesful, print result
      //data.applyEach(Printer());
      if (!readmessage){
      ESP_LOGD("DmsrCustom","READING READY");
      consumption_low_tarif_sensor->publish_state(data.energy_delivered_tariff1);
      consumption_high_tarif_sensor->publish_state(data.energy_delivered_tariff2);
      actual_consumption_sensor->publish_state(data.power_delivered);
      instant_power_current_sensor->publish_state(data.current_l1);
      instant_power_usage_sensor->publish_state(data.power_delivered_l1);
      gas_meter_m3_sensor->publish_state(data.gas_delivered);
//      actual_tarif_sensor->publish_state(data.electricity_tariff);
      short_power_outages_sensor->publish_state(data.electricity_failures);
      long_power_outages_sensor->publish_state(data.electricity_long_failures);
      short_power_drops_sensor->publish_state(data.electricity_sags_l1);
      short_power_peaks_sensor->publish_state(data.electricity_swells_l1);
      reader.enable(false);
      readmessage = true;
      };
    } else {
      // Parser error, print error
      Serial.println(err);
    }
  }
}
};
