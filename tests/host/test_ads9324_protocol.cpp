/**
 * @file test_ads9324_protocol.cpp
 * @brief Host unit tests for ADS9324 24-bit SPI protocol, PGA packing, and conversion math.
 *
 * No silicon required. A register-file mock implements SpiInterface + HostInterface.
 */
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#include "ads9324.hpp"

namespace {

int g_fails = 0;

#define CHECK(cond)                                                                                \
  do {                                                                                             \
    if (!(cond)) {                                                                                 \
      std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                         \
      ++g_fails;                                                                                   \
    }                                                                                              \
  } while (0)

class MockBus : public ads9324::SpiInterface<MockBus>, public ads9324::HostInterface<MockBus> {
public:
  uint16_t mem[3][256]{};
  uint8_t bank_index = 0;
  uint8_t pending_read_addr = 0;
  bool pending_read = false;
  uint32_t last_tx = 0;
  std::vector<uint32_t> writes;
  int convst_pulses = 0;
  bool convst = false;
  uint8_t conversion_bytes[32]{};

  MockBus() {
    mem[0][ads9324::reg::DEVICE_ID] = ads9324::kDeviceIdAds9324;
  }

  void transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) {
    if (len == 3 && tx != nullptr) {
      const uint32_t word = (static_cast<uint32_t>(tx[0]) << 16) | (static_cast<uint32_t>(tx[1]) << 8) |
                            static_cast<uint32_t>(tx[2]);
      last_tx = word;
      const uint8_t addr = tx[0];
      const uint16_t data = static_cast<uint16_t>((static_cast<uint16_t>(tx[1]) << 8) | tx[2]);

      uint32_t reply = 0;
      if (pending_read && addr == 0 && data == 0) {
        reply = static_cast<uint32_t>(mem[bank_index][pending_read_addr]) << 8;
        pending_read = false;
      } else if (addr == ads9324::reg::BANK_SEL) {
        writes.push_back(word);
        if (data == ads9324::reg::BANK_COMMON) {
          bank_index = 0;
        } else if (data == ads9324::reg::BANK_AIN1_8) {
          bank_index = 1;
        } else if (data == ads9324::reg::BANK_AIN9_16) {
          bank_index = 2;
        }
        mem[0][ads9324::reg::BANK_SEL] = data;
      } else if (addr == ads9324::reg::GEN_CFG1 && (data & ads9324::reg::GEN_CFG1_REG_RD_EN) != 0) {
        writes.push_back(word);
        pending_read_addr = static_cast<uint8_t>(data >> 8);
        pending_read = true;
        mem[bank_index][ads9324::reg::GEN_CFG1] = data;
      } else {
        writes.push_back(word);
        mem[bank_index][addr] = data;
      }

      if (rx != nullptr) {
        rx[0] = static_cast<uint8_t>((reply >> 16) & 0xFF);
        rx[1] = static_cast<uint8_t>((reply >> 8) & 0xFF);
        rx[2] = static_cast<uint8_t>(reply & 0xFF);
      }
      return;
    }

    // Conversion burst
    if (rx != nullptr) {
      const std::size_t n = (len < sizeof(conversion_bytes)) ? len : sizeof(conversion_bytes);
      std::memcpy(rx, conversion_bytes, n);
      if (len > n) {
        std::memset(rx + n, 0, len - n);
      }
    }
  }

  void ConvstWrite(bool high) {
    if (convst && !high) {
      ++convst_pulses;
    }
    convst = high;
  }
  bool DrdyRead() { return true; }
  bool HasDrdy() const { return false; }
  void DelayUs(uint32_t) {}
  void DelayMs(uint32_t) {}
};

}  // namespace

int main() {
  using namespace ads9324;

  CHECK(FullScaleVolts(InputRange::PlusMinus10V) == 10.0f);
  CHECK(TwosToVoltage(0, 5.0f) == 0.0f);
  CHECK(TwosToVoltage(32767, 5.0f) > 4.9f);
  CHECK(WireWordToTwos(0x8000, DataFormat::OffsetBinary) == 0);
  CHECK(reg::GenCfg3Value(DoutLaneMode::Lane1, DoutLength::Bits16, true,
                          DataFormat::TwosComplement) == 0x0032);
  CHECK(reg::EncodePgaNibble({InputType::SingleEnded, InputRange::PlusMinus10V,
                              PgaBandwidth::Low, false}) ==
        static_cast<uint16_t>((5u << 4) | 4u));
  CHECK(reg::BankForChannel(0) == RegisterBank::Ain1_8);
  CHECK(reg::BankForChannel(8) == RegisterBank::Ain9_16);
  CHECK(reg::UnpackRegData(0x00000400u) == 0x0004);

  MockBus bus;
  // Mid-scale two's complement 0x0000 on every channel
  ADS9324<MockBus, MockBus> adc(bus, bus);
  CHECK(adc.EnsureInitialized());
  CHECK(adc.IsInitialized());
  CHECK(adc.ReadDeviceId() == kDeviceIdAds9324);
  CHECK(bus.mem[0][reg::GEN_CFG3] == 0x0032);

  ChannelConfig cfg;
  cfg.type = InputType::SingleEnded;
  cfg.range = InputRange::PlusMinus10V;
  cfg.bandwidth = PgaBandwidth::Wide;
  CHECK(adc.ConfigureChannel(0, cfg));
  CHECK(adc.GetChannelConfig(0).range == InputRange::PlusMinus10V);
  // AIN1 occupies low byte of PGA_CONFIG_AIN1_2
  CHECK((bus.mem[1][reg::PGA_CONFIG_AIN12] & 0x00FF) == reg::EncodePgaNibble(cfg));
  CHECK((bus.mem[1][reg::PGA_BW_SEL] & 0x3u) == 1u);

  CHECK(adc.SetChannelOffset(0, -1));  // 10-bit 0x3FF
  CHECK((bus.mem[1][reg::OFS_AIN1] & 0x3FF) == 0x3FFu);
  CHECK(adc.SetChannelGain(9, 0x0CCD));
  CHECK((bus.mem[2][reg::GAN_AIN1 + 1] & 0x3FFF) == 0x0CCDu);

  CHECK(adc.SetPowerDown(true));
  CHECK(bus.mem[0][reg::PDN_CTL] == 1);
  CHECK(adc.SetPowerDown(false));

  bus.conversion_bytes[0] = 0x00;
  bus.conversion_bytes[1] = 0x00;  // CH0 = 0
  bus.conversion_bytes[2] = 0x40;
  bus.conversion_bytes[3] = 0x00;  // CH1 = 0x4000
  auto snap = adc.ReadSnapshot();
  CHECK(snap.ok());
  CHECK(bus.convst_pulses >= 1);
  CHECK(snap.hasChannel(0));
  CHECK(snap.count[0] == 0);
  CHECK(snap.count[1] == static_cast<int16_t>(0x4000));

  auto r = adc.ReadChannel(0);
  CHECK(r.ok());
  CHECK(r.channel == 0);

  auto bad = adc.ReadChannel(16);
  CHECK(bad.error == Error::InvalidChannel);

  if (g_fails != 0) {
    std::fprintf(stderr, "%d check(s) failed\n", g_fails);
    return 1;
  }
  std::puts("hf-ads9324 host protocol tests: PASS");
  return 0;
}
