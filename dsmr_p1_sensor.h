

#include "esphome.h"
#include "dsmr.h"

// * Baud rate for both hardware and software serial
#define BAUD_RATE 115200

// * Max telegram length
#define P1_MAXLINELENGTH 100

// * P1 Meter RX pin
#define P1_SERIAL_RX D3

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

//  P1Reader reader(&Serial, 0);
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
   // * Set to store received telegram
  char telegram[P1_MAXLINELENGTH];
  
  // * Set to store the data values read
  long CONSUMPTION_LOW_TARIF;
  long CONSUMPTION_HIGH_TARIF;
  long ACTUAL_CONSUMPTION;
  long INSTANT_POWER_CURRENT;
  long INSTANT_POWER_USAGE;
  long GAS_METER_M3;
  
  // Set to store data counters read
  long ACTUAL_TARIF;
  long SHORT_POWER_OUTAGES;
  long LONG_POWER_OUTAGES;
  long SHORT_POWER_DROPS;
  long SHORT_POWER_PEAKS;
  
  // * Set during CRC checking
  unsigned int currentCRC = 0; 
  bool startFound = false;
  uint8_t req_pin = 2;

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

//  DsmrP1CustomSensor() : PollingComponent(15000) { }


  void setup() override {
//	Serial.begin(BAUD_RATE);
	
    // * Start software serial for p1 meter
//    p1_serial.begin(BAUD_RATE);
	
	ESP_LOGD("DmsrCustom","Init baud rate pin d5");
	MyData data;
	pinMode(D5, OUTPUT);
	digitalWrite(D5,LOW);
        reader.enable(true);
	last = millis();
  }

  void update() override {
    // Allow the reader to check the serial buffer regularly
  reader.loop();
  //read();  
  //ESP_LOGD("DmsrCustom","Updating..");	
  digitalWrite(D5,HIGH);
//  delay(500);
  // Every minute, fire off a one-off reading
  unsigned long now = millis();
  if (now - last > 5000) {
    reader.enable(true);
    last = now;
  }

  if (available()) {
    MyData data;
    String err;
    //ESP_LOGD("DmsrCustom","READING....");
    if (reader.parse(&data, &err)) {
      // Parse succesful, print result
      data.applyEach(Printer());
      ESP_LOGD("DmsrCustom","READING READY");
      consumption_low_tarif_sensor->publish_state(data.energy_delivered_tariff1);
      consumption_high_tarif_sensor->publish_state(data.energy_delivered_tariff2);
  




  } else {
      // Parser error, print error
      Serial.println(err);
    }
  }


















  if (false)
//  if (available())
    {
      ESP_LOGD("DmsrCustom","Data ready..start if loop");	
      memset(telegram, 0, sizeof(telegram));
      while (available())
      {
        ESP.wdtDisable();
        int len = readBytesUntil('\n', telegram, P1_MAXLINELENGTH);
//        ESP_LOGD("DmsrCustom","Length of telegram %d",len);
        ESP.wdtEnable(1);
//        ESP_LOGD("DmsrCustom","*%s",telegram);
        telegram[len] = '\n';
        telegram[len + 1] = 0;
        yield();
	ESP_LOGD("DmsrCustom","*%s",telegram);
        bool result = decode_telegram(len + 1);
        if (result)
        {
	  ESP_LOGD("DmsrCustom","CRC Ok_PIN LOW");  
          consumption_low_tarif_sensor->publish_state(CONSUMPTION_LOW_TARIF);
          consumption_high_tarif_sensor->publish_state(CONSUMPTION_HIGH_TARIF);
          actual_consumption_sensor->publish_state(ACTUAL_CONSUMPTION);
          instant_power_current_sensor->publish_state(INSTANT_POWER_CURRENT);
          instant_power_usage_sensor->publish_state(INSTANT_POWER_USAGE);
          gas_meter_m3_sensor->publish_state(GAS_METER_M3);
	  actual_tarif_sensor->publish_state(ACTUAL_TARIF);
          short_power_outages_sensor->publish_state(SHORT_POWER_OUTAGES);
          long_power_outages_sensor->publish_state(LONG_POWER_OUTAGES);
          short_power_drops_sensor->publish_state(SHORT_POWER_DROPS);
          short_power_peaks_sensor->publish_state(SHORT_POWER_PEAKS);
	digitalWrite(D5,LOW);
        }
	else
	{
//	  ESP_LOGD("DmsrCustom","CRC Not Ok");	
	}
      }
    }
    else
    {
    //  ESP_LOGD("DmsrCustom","No data ready..");
    }
  }

 private:
  unsigned int CRC16(unsigned int crc, unsigned char *buf, int len)
  {
  	for (int pos = 0; pos < len; pos++)
    {
  		crc ^= (unsigned int)buf[pos];  // * XOR byte into least sig. byte of crc
                        // * Loop over each bit
      for (int i = 8; i != 0; i--)
      {
        // * If the LSB is set
        if ((crc & 0x0001) != 0)
        {
          // * Shift right and XOR 0xA001
          crc >>= 1;
  	  crc ^= 0xA001;
  			}
        // * Else LSB is not set
        else
          // * Just shift right
          crc >>= 1;
  		}
  	}
  	return crc;
  }
  
  bool isNumber(char *res, int len)
  {
    for (int i = 0; i < len; i++)
    {
      if (((res[i] < '0') || (res[i] > '9')) && (res[i] != '.' && res[i] != 0))
        return false;
    }
    return true;
  }
  
  int FindCharInArrayRev(char array[], char c, int len)
  {
    for (int i = len - 1; i >= 0; i--)
    {
      if (array[i] == c)
        return i;
    }
    return -1;
  }
  
  long getValue(char *buffer, int maxlen, char startchar, char endchar)
  {
    int s = FindCharInArrayRev(buffer, startchar, maxlen - 2);
    int l = FindCharInArrayRev(buffer, endchar, maxlen - 2) - s - 1;
  
    char res[16];
    memset(res, 0, sizeof(res));
  
    if (strncpy(res, buffer + s + 1, l))
    {
      if (endchar == '*')
      {
        if (isNumber(res, l))
          // * Lazy convert float to long
          return (1000 * atof(res));
      }
      else if (endchar == ')')
      {
        if (isNumber(res, l))
          return atof(res);
      }
    }
    return 0;
  }
  
  bool decode_telegram(int len)
  {
    int startChar = FindCharInArrayRev(telegram, '/', len);
    //ESP_LOGD("DmsrCustom","Start Char %d",startChar);
    int endChar = FindCharInArrayRev(telegram, '!', len);
    ESP_LOGD("DmsrCustom","End Char %d",endChar);
    bool validCRCFound = false;

    for (int cnt = 0; cnt < len; cnt++)
      //Serial.print(telegram[cnt]);
      //ESP_LOGD("DmsrCustom","for loop %d", cnt);
    if (startChar >= 0)
    {
      //ESP_LOGD("DmsrCustom","Start gevonden"); 
      // * Start found. Reset CRC calculation
      currentCRC = CRC16(0x0000,(unsigned char *) telegram+startChar, len-startChar);
      startFound = true;
      //ESP_LOGD("DmsrCustom","START %d",currentCRC);
    }
    else if (endChar >= 0)
    {
      // * Add to crc calc
            ESP_LOGD("DmsrCustom","Eind gevonden");
      currentCRC = CRC16(currentCRC,(unsigned char*)telegram+endChar, 1);
      //ESP_LOGD("DmsrCustom","EIND %d",currentCRC);
      char messageCRC[5];
      strncpy(messageCRC, telegram + endChar + 1, 4);
      messageCRC[4] = 0;   // * Thanks to HarmOtten (issue 5)
//      ESP_LOGD("DmsrCustom","%s",messageCRC);
      validCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC);
//      if (startFound)
  //    {
//	startFound = false;
//	validCRCFound = true;
  //    }
      if (validCRCFound)
      {
        Serial.println(F("CRC Valid!"));
 //       ESP_LOGD("DmsrCustom","CRC VALID!");
      }
      else
      {
	Serial.println(F("CRC Invalid!"));
 //     	ESP_LOGD("DmsrCustom","CRC INVALID!");
      }
      currentCRC = 0;
    }
    else
    {
      currentCRC = CRC16(currentCRC, (unsigned char*) telegram, len);
      //ESP_LOGD("DmsrCustom","MID %d",currentCRC);
    }
    // 1-0:1.8.1(000992.992*kWh)
    // 1-0:1.8.1 = Elektra verbruik laag tarief (DSMR v4.0)
    if (strncmp(telegram, "1-0:1.8.1", strlen("1-0:1.8.1")) == 0)
    {
      CONSUMPTION_LOW_TARIF = getValue(telegram, len, '(', '*');
      ESP_LOGD("DmsrCustom","Consumption low tarif found!");
    }
    
    // 1-0:1.8.2(000560.157*kWh)
    // 1-0:1.8.2 = Elektra verbruik hoog tarief (DSMR v4.0)
    if (strncmp(telegram, "1-0:1.8.2", strlen("1-0:1.8.2")) == 0)
    {
      CONSUMPTION_HIGH_TARIF = getValue(telegram, len, '(', '*');
    }
  
    // 1-0:1.7.0(00.424*kW) Actueel verbruik
    // 1-0:2.7.0(00.000*kW) Actuele teruglevering
    // 1-0:1.7.x = Electricity consumption actual usage (DSMR v4.0)
    if (strncmp(telegram, "1-0:1.7.0", strlen("1-0:1.7.0")) == 0)
    {
      ACTUAL_CONSUMPTION = getValue(telegram, len, '(', '*');
    }
  
    // 1-0:21.7.0(00.378*kW)
    // 1-0:21.7.0 = Instantaan vermogen Elektriciteit levering
    if (strncmp(telegram, "1-0:21.7.0", strlen("1-0:21.7.0")) == 0)
    {
      INSTANT_POWER_USAGE = getValue(telegram, len, '(', '*');
    }
  
    // 1-0:31.7.0(002*A)
    // 1-0:31.7.0 = Instantane stroom Elektriciteit
    if (strncmp(telegram, "1-0:31.7.0", strlen("1-0:31.7.0")) == 0)
    {
      INSTANT_POWER_CURRENT = getValue(telegram, len, '(', '*');
    }
  
    // 0-1:24.2.1(150531200000S)(00811.923*m3)
    // 0-1:24.2.1 = Gas (DSMR v4.0) on Kaifa MA105 meter
    if (strncmp(telegram, "0-1:24.2.1", strlen("0-1:24.2.1")) == 0)
    {
      GAS_METER_M3 = getValue(telegram, len, '(', '*');
    }
  
    // 0-0:96.14.0(0001)
    // 0-0:96.14.0 = Actual Tarif
    if (strncmp(telegram, "0-0:96.14.0", strlen("0-0:96.14.0")) == 0)
    {
      ACTUAL_TARIF = getValue(telegram, len, '(', ')');
    }
  
    // 0-0:96.7.21(00003)
    // 0-0:96.7.21 = Aantal onderbrekingen Elektriciteit
    if (strncmp(telegram, "0-0:96.7.21", strlen("0-0:96.7.21")) == 0)
    {
      SHORT_POWER_OUTAGES = getValue(telegram, len, '(', ')');
    }
  
    // 0-0:96.7.9(00001)
    // 0-0:96.7.9 = Aantal lange onderbrekingen Elektriciteit
    if (strncmp(telegram, "0-0:96.7.9", strlen("0-0:96.7.9")) == 0)
    {
      LONG_POWER_OUTAGES = getValue(telegram, len, '(', ')');
    }
  
    // 1-0:32.32.0(00000)
    // 1-0:32.32.0 = Aantal korte spanningsdalingen Elektriciteit in fase 1
    if (strncmp(telegram, "1-0:32.32.0", strlen("1-0:32.32.0")) == 0)
    {
      SHORT_POWER_DROPS = getValue(telegram, len, '(', ')');
    }
  
    // 1-0:32.36.0(00000)
    // 1-0:32.36.0 = Aantal korte spanningsstijgingen Elektriciteit in fase 1
    if (strncmp(telegram, "1-0:32.36.0", strlen("1-0:32.36.0")) == 0)
    {
      SHORT_POWER_PEAKS = getValue(telegram, len, '(', ')');
    }
  
    return validCRCFound;
  } 
  
};

