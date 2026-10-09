## @file
#
#  Radxa CM5 (RK3588S2) on the Radxa CM5 IO Board.
#
#  Based on the ROCK 5 Model A platform (RK3588S).
#
#  Copyright (c) 2014-2018, Linaro Limited. All rights reserved.
#  Copyright (c) 2023, Molly Sophia <mollysophia379@gmail.com>
#  Copyright (c) 2023-2024, Mario Bălănică <mariobalanica02@gmail.com>
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = CM5
  PLATFORM_VENDOR                = Radxa
  PLATFORM_GUID                  = 6ff0e82f-bc5e-4bd1-9c64-889dc24ef6ef
  PLATFORM_VERSION               = 0.2
  DSC_SPECIFICATION              = 0x00010019
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  VENDOR_DIRECTORY               = Platform/$(PLATFORM_VENDOR)
  PLATFORM_DIRECTORY             = $(VENDOR_DIRECTORY)/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Silicon/Rockchip/RK3588/RK3588.fdf
  RK_PLATFORM_FVMAIN_MODULES     = $(PLATFORM_DIRECTORY)/$(PLATFORM_NAME).Modules.fdf.inc

  #
  # Enable RTC (Hymacro HYM8563 on i2c6, per rk3588s-radxa-cm5-io dts)
  #
  DEFINE RK_RTC8563_ENABLE = TRUE

  #
  # RK3588S-based platform (CM5 uses the RK3588S2 variant)
  #
!include Silicon/Rockchip/RK3588/RK3588SPlatform.dsc.inc

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################

[LibraryClasses.common]
  RockchipPlatformLib|$(PLATFORM_DIRECTORY)/Library/RockchipPlatformLib/RockchipPlatformLib.inf

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform.
#
################################################################################

[PcdsFixedAtBuild.common]
  # SMBIOS platform config
  gRockchipTokenSpaceGuid.PcdPlatformName|"Radxa CM5 IO"
  gRockchipTokenSpaceGuid.PcdPlatformVendorName|"Radxa"
  gRockchipTokenSpaceGuid.PcdFamilyName|"CM5"
  gRockchipTokenSpaceGuid.PcdProductUrl|"https://docs.radxa.com/en/cm5/cm5_overview/CM5_Datasheet"
  gRockchipTokenSpaceGuid.PcdDeviceTreeName|"rk3588s-radxa-cm5-io"

  # I2C
  #   0x42/0x43 : RK8602/RK8603 GPU DVFS regulators on i2c0
  #   0x51      : RTC (HYM8563) on i2c6
  #   0x11      : Audio codec (ES8311) on i2c8
  gRockchipTokenSpaceGuid.PcdI2cSlaveAddresses|{ 0x42, 0x43, 0x51, 0x11 }
  gRockchipTokenSpaceGuid.PcdI2cSlaveBuses|{ 0x0, 0x0, 0x6, 0x8 }
  gRockchipTokenSpaceGuid.PcdI2cSlaveBusesRuntimeSupport|{ FALSE, FALSE, TRUE, FALSE }
  gRockchipTokenSpaceGuid.PcdRk860xRegulatorAddresses|{ 0x42, 0x43 }
  gRockchipTokenSpaceGuid.PcdRk860xRegulatorBuses|{ 0x0, 0x0 }
  gRockchipTokenSpaceGuid.PcdRk860xRegulatorTags|{ $(SCMI_CLK_CPUB01), $(SCMI_CLK_CPUB23) }
  gRockchipTokenSpaceGuid.PcdRtc8563Bus|0x6

  #
  # PCIe/SATA/USB Combo PIPE PHY support flags and default values
  #
  # ComboPHY0 -> M.2 Key-M slot (PCIe x2, pcie20l2)
  # ComboPHY2 -> Type-C port (USB3 + DP0)
  #
  gRK3588TokenSpaceGuid.PcdComboPhy0Switchable|TRUE
  gRK3588TokenSpaceGuid.PcdComboPhy2Switchable|TRUE
  gRK3588TokenSpaceGuid.PcdComboPhy0ModeDefault|$(COMBO_PHY_MODE_PCIE)
  gRK3588TokenSpaceGuid.PcdComboPhy2ModeDefault|$(COMBO_PHY_MODE_USB3)

  #
  # USB/DP Combo PHY support flags and default values
  #
  gRK3588TokenSpaceGuid.PcdUsbDpPhy0Supported|TRUE
  gRK3588TokenSpaceGuid.PcdDp0LaneMux|{ 0x2, 0x3 }

  #
  # GMAC (1Gbps Ethernet on the CM5 IO board, rtk8211f PHY at address 0,
  # tx-delay 0x41 / rx-delay 0x3a per rk3588s-radxa-cm5-io.dts)
  #
  gRK3588TokenSpaceGuid.PcdGmac1Supported|TRUE
  gRK3588TokenSpaceGuid.PcdGmac1TxDelay|0x41
  gRK3588TokenSpaceGuid.PcdGmac1RxDelay|0x3a

  #
  # I2S (headphone jack path; audio codec is ES8311 on i2c8)
  #
  gRK3588TokenSpaceGuid.PcdI2S0Supported|TRUE

  #
  # On-board fan output (PWM11_M2 on the 40-pin header)
  #
  gRK3588TokenSpaceGuid.PcdHasOnBoardFanOutput|TRUE

  #
  # Display support flags and default values
  # (HDMI full-size connector + DP alt-mode over the Type-C port)
  #
  gRK3588TokenSpaceGuid.PcdDisplayConnectors|{CODE({
    VOP_OUTPUT_IF_HDMI0,
    VOP_OUTPUT_IF_DP0
  })}

################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform.
#
################################################################################
[Components.common]
  # ACPI Support
  $(PLATFORM_DIRECTORY)/AcpiTables/AcpiTables.inf

  # Device Tree Support
  $(PLATFORM_DIRECTORY)/DeviceTree/Vendor.inf
  $(PLATFORM_DIRECTORY)/DeviceTree/Mainline.inf

  # Splash screen logo
  $(VENDOR_DIRECTORY)/Drivers/LogoDxe/LogoDxe.inf
