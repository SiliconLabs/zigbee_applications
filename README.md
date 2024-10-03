<table border="0">
  <tr>
    <td align="left" valign="middle">
    <h1>EFR32 Zigbee Application Examples</h1>
  </td>
  <td align="left" valign="middle">
    <a href="https://www.silabs.com/wireless/zigbee">
      <img src="http://pages.silabs.com/rs/634-SLU-379/images/WGX-transparent.png"  title="Silicon Labs Gecko and Wireless Gecko MCUs" alt="EFM32 32-bit Microcontrollers" width="250"/>
    </a>
  </td>
  </tr>
</table>

# Silicon Labs Zigbee Application Examples #

This repo demonstrates some of the applications that can be built on top of the Silicon Labs Zigbee stack.

This repository provides only SLCP projects (as External Repositories) that are listed in the [Examples](#examples) section. Please refer [Working with Projects](#working-with-projects) section to know how to add this repository to **External Repo** in Simplicity Studio.

## Examples ##

| No | Example name | Link to example |
|:--:|:-------------|:---------------:|
| 1  | Zigbee - Battery Monitor | [Click Here](./zigbee_battery_monitor) |
| 2  | Zigbee - Manufacturing Library Extension | [Click Here](./zigbee_mfglib_extension) |
| 3  | Zigbee - RTC Time | [Click Here](./zigbee_rtc_time_sync) |
| 4  | Zigbee - SoC Sleepy Switch | [Click Here](./zigbee_sed_z3switch) |
| 5  | Zigbee - Smart Lighting using Motion Sensor PIR | [Click Here](./zigbee_smart_lighting) |
| 6  | Zigbee - Sleepy End-Device and Gateway | [Click Here](./zigbee_sed_rht_sensor) |
| 7  | Zigbee - Human Detection - MLX90640 | [Click Here](./zigbee_human_detection) |
| 8  | Zigbee - Philips Hue Smart Light with Home Assistant OS | [Click here](./zigbee_philips_hue_with_home_assistant_os) |

## Working with Projects ##

1. To add an external repository, perform the following steps.

    - From Simplicity Studio 5, go to **Preferences > Simplicity Studio > External Repos**. Here you can add the repo `https://github.com/SiliconLabs/bluetooth_applications.git`

    - Cloning and then selecting the branch, tag, or commit via UI. The default branch is Master. This repo cloned to `<path_to_the_SimplicityStudio_v5>\developer\repos\`

2. From Launcher, select your device from the "Debug Adapters" on the left before creating a project. Then click the **EXAMPLE PROJECTS & DEMOS** tab -> check **bluetooth_applications** under **Provider** to show a list of Bluetooth example projects compatible with the selected device. Click CREATE on a project to generate a new application from the selected template.

## Porting to Another Board ##

To change the target board, navigate to Project -> Properties -> C/C++ Build -> Board/Part/SDK. Start typing in the Boards search box and locate the desired development board, then click Apply to change the project settings. Ensure that the board specifics include paths, found in Project -> Properties -> C/C++ General -> Paths and Symbols, correctly match the target board.

## Documentation ##

Official documentation can be found on our [Developer Documentation](https://docs.silabs.com/zigbee/latest/) page.

## Reporting Bugs/Issues and Posting Questions and Comments ##

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of this repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of this repo.

## Disclaimer ##

The Gecko SDK suite supports development with Silicon Labs IoT SoC and module devices. Unless otherwise specified in the specific directory, all examples are considered to be EXPERIMENTAL QUALITY which implies that the code provided in the repository has not been formally tested and is provided as-is.  It is not suitable for production environments.  In addition, this code will not be maintained and there may be no bug maintenance planned for these resources. Silicon Labs may update projects from time to time.
