/** @file
*
*  Radxa CM5 (RK3588S2) on the Radxa CM5 IO Board.
*
*  Ported from the ROCK 5 Model A platform. Pinmux/GPIO values were derived
*  from the Radxa CM5 device tree:
*    arch/arm64/boot/dts/rockchip/rk3588s-radxa-cm5-io.dts
*    (armbian/linux-rockchip, rk-6.1-rkr branch; also in mainline Linux >= 6.11)
*
*  Copyright (c) 2021, Rockchip Limited. All rights reserved.
*  Copyright (c) 2023-2024, Mario Bălănică <mariobalanica02@gmail.com>
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/GpioLib.h>
#include <Library/RK806.h>
#include <Library/Rk3588Pcie.h>
#include <Library/PWMLib.h>
#include <Soc.h>
#include <VarStoreData.h>

static struct regulator_init_data  rk806_init_data[] = {
  /* Master PMIC */
  RK8XX_VOLTAGE_INIT (MASTER_BUCK1,  750000),
  RK8XX_VOLTAGE_INIT (MASTER_BUCK3,  750000),
  RK8XX_VOLTAGE_INIT (MASTER_BUCK4,  750000),
  RK8XX_VOLTAGE_INIT (MASTER_BUCK5,  850000),
  // RK8XX_VOLTAGE_INIT(MASTER_BUCK6, 750000),
  RK8XX_VOLTAGE_INIT (MASTER_BUCK7,  2000000),
  RK8XX_VOLTAGE_INIT (MASTER_BUCK8,  3300000),
  RK8XX_VOLTAGE_INIT (MASTER_BUCK10, 1800000),

  RK8XX_VOLTAGE_INIT (MASTER_NLDO1,  750000),
  RK8XX_VOLTAGE_INIT (MASTER_NLDO2,  850000),
  RK8XX_VOLTAGE_INIT (MASTER_NLDO3,  750000),
  RK8XX_VOLTAGE_INIT (MASTER_NLDO4,  850000),
  RK8XX_VOLTAGE_INIT (MASTER_NLDO5,  750000),

  RK8XX_VOLTAGE_INIT (MASTER_PLDO1,  1800000),
  RK8XX_VOLTAGE_INIT (MASTER_PLDO2,  1800000),
  RK8XX_VOLTAGE_INIT (MASTER_PLDO3,  1200000),
  RK8XX_VOLTAGE_INIT (MASTER_PLDO4,  3300000),
  RK8XX_VOLTAGE_INIT (MASTER_PLDO5,  3300000),
  RK8XX_VOLTAGE_INIT (MASTER_PLDO6,  1800000),
  /* No dual PMICs on this platform */
};

VOID
EFIAPI
SdmmcIoMux (
  VOID
  )
{
  /* sdmmc0 iomux (microSD socket on the CM5 IO board) */
  BUS_IOC->GPIO4D_IOMUX_SEL_L  = (0xFFFFUL << 16) | (0x1111); // SDMMC_D0,SDMMC_D1,SDMMC_D2,SDMMC_D3
  BUS_IOC->GPIO4D_IOMUX_SEL_H  = (0x00FFUL << 16) | (0x0011); // SDMMC_CLK,SDMMC_CMD
  PMU1_IOC->GPIO0A_IOMUX_SEL_H = (0x000FUL << 16) | (0x0001); // SDMMC_DET
}

VOID
EFIAPI
SdhciEmmcIoMux (
  VOID
  )
{
  /* sdhci0 iomux (eMMC on the CM5 SoM) */
  /* Do not override, set by earlier boot stages. */
}

#define NS_CRU_BASE       0xFD7C0000
#define CRU_CLKSEL_CON59  0x03EC
#define CRU_CLKSEL_CON78  0x0438

VOID
EFIAPI
Rk806SpiIomux (
  VOID
  )
{
  /* io mux (PMIC SPI / vsel interface, same wiring as ROCK 5A) */
  PMU1_IOC->GPIO0A_IOMUX_SEL_H = (0x0FF0UL << 16) | 0x0110;
  PMU1_IOC->GPIO0B_IOMUX_SEL_L = (0xF0FFUL << 16) | 0x1011;
  MmioWrite32 (NS_CRU_BASE + CRU_CLKSEL_CON59, (0x00C0UL << 16) | 0x0080);
}

VOID
EFIAPI
Rk806Configure (
  VOID
  )
{
  UINTN  RegCfgIndex;

  RK806Init ();

  RK806PinSetFunction (MASTER, 1, 2); // rk806_dvs1_pwrdn

  for (RegCfgIndex = 0; RegCfgIndex < ARRAY_SIZE (rk806_init_data); RegCfgIndex++) {
    RK806RegulatorInit (rk806_init_data[RegCfgIndex]);
  }
}

VOID
EFIAPI
SetCPULittleVoltage (
  IN UINT32  Microvolts
  )
{
  struct regulator_init_data  Rk806CpuLittleSupply =
    RK8XX_VOLTAGE_INIT (MASTER_BUCK2, Microvolts);

  RK806RegulatorInit (Rk806CpuLittleSupply);
}

VOID
EFIAPI
NorFspiIomux (
  VOID
  )
{
  /* No SPI NOR flash on this platform */
}

VOID
EFIAPI
GmacIomux (
  IN UINT32  Id
  )
{
  switch (Id) {
    case 1:
      /* gmac1 iomux (rgmii, RTL8211F PHY on the CM5 IO board) */
      BUS_IOC->GPIO3B_IOMUX_SEL_H = (0x0FFFUL << 16) | 0x0111;
      BUS_IOC->GPIO3A_IOMUX_SEL_L = (0xFFFFUL << 16) | 0x1111;
      BUS_IOC->GPIO3B_IOMUX_SEL_L = (0xF0FFUL << 16) | 0x1011;
      BUS_IOC->GPIO3A_IOMUX_SEL_H = (0xF0FFUL << 16) | 0x1011;
      BUS_IOC->GPIO3C_IOMUX_SEL_L = (0xFF00UL << 16) | 0x1100;

      /* phy reset (gmac_int / PHY_RESET, GPIO3_B7, active low) */
      GpioPinSetDirection (3, GPIO_PIN_PB7, GPIO_PIN_OUTPUT);
      break;
    default:
      break;
  }
}

VOID
EFIAPI
GmacIoPhyReset (
  UINT32   Id,
  BOOLEAN  Enable
  )
{
  switch (Id) {
    case 1:
      /* phy reset (active low) */
      GpioPinWrite (3, GPIO_PIN_PB7, !Enable);
      break;
    default:
      break;
  }
}

VOID
EFIAPI
NorFspiEnableClock (
  UINT32  *CruBase
  )
{
  /* No SPI NOR flash on this platform */
}

VOID
EFIAPI
I2cIomux (
  UINT32  id
  )
{
  switch (id) {
    case 0:
      GpioPinSetFunction (0, GPIO_PIN_PD1, 3); // i2c0_scl_m2 (RK806 PMIC)
      GpioPinSetFunction (0, GPIO_PIN_PD2, 3); // i2c0_sda_m2 (RK806 PMIC)
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      break;
    case 6:
      GpioPinSetFunction (0, GPIO_PIN_PC7, 2); // i2c6_scl_m0 (RTC HYM8563)
      GpioPinSetFunction (0, GPIO_PIN_D0, 2);  // i2c6_sda_m0 (RTC HYM8563)
      break;
    case 7:
      break;
    case 8:
      GpioPinSetFunction (1, GPIO_PIN_D6, 2);  // i2c8_scl_m0 (ES8311 audio codec)
      GpioPinSetFunction (1, GPIO_PIN_D7, 2);  // i2c8_sda_m0 (ES8311 audio codec)
      break;
    default:
      break;
  }
}

VOID
EFIAPI
UsbPortPowerEnable (
  VOID
  )
{
  DEBUG ((DEBUG_INFO, "UsbPortPowerEnable called\n"));

  /* vcc5v0_host (USB3.0 HOST port over ComboPHY2 / Type-C) */
  GpioPinSetDirection (0, GPIO_PIN_PD5, GPIO_PIN_OUTPUT);
  GpioPinWrite (0, GPIO_PIN_PD5, TRUE);
}

VOID
EFIAPI
Usb2PhyResume (
  VOID
  )
{
  MmioWrite32 (0xfd5d0008, 0x20000000);
  MmioWrite32 (0xfd5d4008, 0x20000000);
  MmioWrite32 (0xfd5d8008, 0x20000000);
  MmioWrite32 (0xfd5dc008, 0x20000000);
  MmioWrite32 (0xfd7f0a10, 0x07000700);
  MmioWrite32 (0xfd7f0a10, 0x07000000);
}

VOID
EFIAPI
PcieIoInit (
  UINT32  Segment
  )
{
  if (Segment == PCIE_SEGMENT_PCIE20L2) {
    // M.2 Key-M slot: power enable + PERST#
    GpioPinSetDirection (3, GPIO_PIN_PA7, GPIO_PIN_OUTPUT); // vpcie3v3_en
    GpioPinSetDirection (3, GPIO_PIN_PD1, GPIO_PIN_OUTPUT); // pcie20l2_peresetn
  }
}

VOID
EFIAPI
PciePowerEn (
  UINT32   Segment,
  BOOLEAN  Enable
  )
{
  if (Segment == PCIE_SEGMENT_PCIE20L2) {
    /* output high to enable vpcie3v3 */
    GpioPinWrite (3, GPIO_PIN_PA7, Enable);
  }
}

VOID
EFIAPI
PciePeReset (
  UINT32   Segment,
  BOOLEAN  Enable
  )
{
  if (Segment == PCIE_SEGMENT_PCIE20L2) {
    GpioPinWrite (3, GPIO_PIN_PD1, !Enable);
  }
}

VOID
EFIAPI
HdmiTxIomux (
  IN UINT32  Id
  )
{
  switch (Id) {
    case 0:
      GpioPinSetFunction (4, GPIO_PIN_PC1, 5); // hdmim0_tx0_cec
      GpioPinSetPull (4, GPIO_PIN_PC1, GPIO_PIN_PULL_NONE);
      GpioPinSetFunction (4, GPIO_PIN_PB7, 5); // hdmim0_tx0_scl (DDC)
      GpioPinSetPull (4, GPIO_PIN_PB7, GPIO_PIN_PULL_NONE);
      GpioPinSetFunction (4, GPIO_PIN_PC0, 5); // hdmim0_tx0_sda (DDC)
      GpioPinSetPull (4, GPIO_PIN_PC0, GPIO_PIN_PULL_NONE);
      GpioPinSetFunction (3, GPIO_PIN_PD4, 3); // hdmim1_tx0_hpd
      GpioPinSetPull (3, GPIO_PIN_PD4, GPIO_PIN_PULL_NONE);
      break;
  }
}

PWM_DATA  pwm_data = {
  .ControllerID = PWM_CONTROLLER2,
  .ChannelID    = PWM_CHANNEL3,
  .PeriodNs     = 4000000,
  .DutyNs       = 4000000,
  .Polarity     = FALSE,
}; // PWM11 (controller2 channel3), pwm_8ch_m2 pin function

VOID
EFIAPI
PwmFanIoSetup (
  VOID
  )
{
  /*
   * Fan output routed to the 40-pin header (PWM11_M2).
   * NOTE: verify the exact mux index against the CM5 TRM/DTS during
   * bring-up; pwm_8ch_m2 uses a non-default function index.
   */
  GpioPinSetFunction (1, GPIO_PIN_D5, 10); // pwm11_ir_m2 -> fan pin
  RkPwmSetConfig (&pwm_data);
  RkPwmEnable (&pwm_data);
}

VOID
EFIAPI
PwmFanSetSpeed (
  IN UINT32  Percentage
  )
{
  pwm_data.DutyNs = pwm_data.PeriodNs * Percentage / 100;
  RkPwmSetConfig (&pwm_data);
}

VOID
EFIAPI
PlatformInitLeds (
  VOID
  )
{
  /* Status indicator (user-led, GPIO4_B4, active high per cm5-io dts) */
  GpioPinWrite (4, GPIO_PIN_PB4, FALSE);
  GpioPinSetDirection (4, GPIO_PIN_PB4, GPIO_PIN_OUTPUT);
}

VOID
EFIAPI
PlatformSetStatusLed (
  IN BOOLEAN  Enable
  )
{
  GpioPinWrite (4, GPIO_PIN_PB4, Enable);
}

CONST EFI_GUID *
EFIAPI
PlatformGetDtbFileGuid (
  IN UINT32  CompatMode
  )
{
  STATIC CONST EFI_GUID  VendorDtbFileGuid = {
    // DeviceTree/Vendor.inf
    0xf472556d, 0x5137, 0x4b99, { 0x82, 0x54, 0x8d, 0x42, 0x2f, 0x66, 0x9a, 0x19 }
  };
  STATIC CONST EFI_GUID  MainlineDtbFileGuid = {
    // DeviceTree/Mainline.inf
    0x37f5fdc7, 0xbfea, 0x4004, { 0x8f, 0x81, 0xbb, 0xa6, 0x4f, 0xfe, 0xea, 0xaf }
  };

  switch (CompatMode) {
    case FDT_COMPAT_MODE_VENDOR:
      return &VendorDtbFileGuid;
    case FDT_COMPAT_MODE_MAINLINE:
      return &MainlineDtbFileGuid;
  }

  return NULL;
}

VOID
EFIAPI
PlatformEarlyInit (
  VOID
  )
{
  // Configure various things specific to this platform
}
