#include "Nintendo.h"

// Define a Gamecube Controller and a Console
CGamecubeController GamecubeController1(7);
CGamecubeController GamecubeController2(8);
CGamecubeConsole GamecubeConsole1(13);

// Pin definitions
#define pinLed LED_BUILTIN

void setup()
{
  // Set up debug led
  pinMode(pinLed, OUTPUT);

  // Start debug serial
  Serial.begin(115200);
  Serial.println(F("Start"));
}


void loop()
{
  for (uint8_t option = 0; option < 2; option++)
  {
    // Only read a single controller between each gamecube poll.
    // The gamecube polls every 1ms - 15ms.
    // In Mario football it polls 2 times fast (1ms between) and then again after 15ms.
    // It may also help to write 2 times to the gamecube instead of only a single time.
    switch (option) {
      case 0:
        {
          if (!GamecubeController1.read()) {
            Serial.println(F("Error reading Gamecube controller 1."));
            digitalWrite(pinLed, HIGH);
            delay(1000);
            return;
          }
        }
        break;
      case 1:
        {
          if (!GamecubeController2.read()) {
            Serial.println(F("Error reading Gamecube controller 2."));
            digitalWrite(pinLed, HIGH);
            delay(1000);
            return;
          }
        }
        break;
    }

    // Get the data of each controller
    auto r1 = GamecubeController1.getReport();
    auto r2 = GamecubeController2.getReport();

    // Merge both controller data into one
    Gamecube_Data_t d = defaultGamecubeData;

    if (r1.start) {
      r1.start = 0;
      r1.right = 255;
      r1.r = 1;
    }
    
    if (r1.z) {
      r1.z = 0;
      r1.left = 255;
      r1.l = 1;
    }

    int diagonals [2] = {-1, -1};
    if (r1.a) {
      r1.a = 0;
      diagonals[0] = 255;
      diagonals[1] = 255;
    } else if (r1.b) {
      r1.b = 0;
      diagonals[0] = 0;
      diagonals[1] = 255;
    } else if (r1.x) {
      r1.x = 0;
      diagonals[0] = 255;
      diagonals[1] = 0;
    } else if (r1.y) {
      r1.y = 0;
      diagonals[0] = 0;
      diagonals[1] = 0;
    }

    d.report.buttons0 |= r1.buttons0 | r2.buttons0;
    d.report.buttons1 |= r1.buttons1 | r2.buttons1;

    /**** REMAP DANCE PAD MOVEMENT ****/
    if (r1.dup) {
      d.report.dup = 0;
      d.report.raw8[2] = 128;
      d.report.raw8[3] = 255;
    } else if (r1.ddown) {
      d.report.ddown = 0;
      d.report.raw8[2] = 128;
      d.report.raw8[3] = 0;
    } else if (r1.dleft) {
      d.report.dleft = 0;
      d.report.raw8[2] = 0;
      d.report.raw8[3] = 128;
    } else if (r1.dright) {
      d.report.dright = 0;
      d.report.raw8[2] = 255;
      d.report.raw8[3] = 128;
    } else if (diagonals[0] != -1 && diagonals[1] != -1) {
      d.report.raw8[2] = diagonals[0];
      d.report.raw8[3] = diagonals[1];
    }

    // Use the maximum of each left/right trigger
    if (r1.left > r2.left) {
      d.report.left = r1.left;
    }
    else {
      d.report.left = r2.left;
    }
    if (r1.right > r2.right) {
      d.report.right = r1.right;
    }
    else {
      d.report.right = r2.right;
    }

    // Mirror the controller data to the console
    if (!GamecubeConsole1.write(d))
    {
      Serial.println(F("Error writing Gamecube controller."));
      digitalWrite(pinLed, HIGH);
      delay(1000);
    }

    /**** DEBUG ****/
//    auto status = GamecubeController1.getStatus();
//    print_gc_report(d.report, status);

    // Enable rumble
    if (d.status.rumble) {
      GamecubeController1.setRumble(true);
      GamecubeController2.setRumble(true);
    }
    else {
      GamecubeController1.setRumble(false);
      GamecubeController2.setRumble(false);
    }
  }

  digitalWrite(pinLed, LOW);
}

void print_gc_report(Gamecube_Report_t &gc_report, Gamecube_Status_t &gc_status)
{
  // Print device information
  Serial.print(F("Device: "));
  switch (gc_status.device) {
    case NINTENDO_DEVICE_GC_NONE:
      Serial.println(F("No Gamecube Controller found!"));
      break;
    case NINTENDO_DEVICE_GC_WIRED:
      Serial.println(F("Original Nintendo Gamecube Controller"));
      break;

    default:
      Serial.print(F("Unknown "));
      Serial.println(gc_status.device, HEX);
      break;
  }

  // Print rumble state
  Serial.print(F("Rumble "));
  if (gc_status.rumble)
    Serial.println(F("on"));
  else
    Serial.println(F("off"));

  // Prints the raw data from the controller
  Serial.println();
  Serial.println(F("Printing Gamecube controller report:"));
  Serial.print(F("Start:\t"));
  Serial.println(gc_report.start);

  Serial.print(F("Y:\t"));
  Serial.println(gc_report.y);

  Serial.print(F("X:\t"));
  Serial.println(gc_report.x);

  Serial.print(F("B:\t"));
  Serial.println(gc_report.b);

  Serial.print(F("A:\t"));
  Serial.println(gc_report.a);

  Serial.print(F("L:\t"));
  Serial.println(gc_report.l);
  Serial.print(F("R:\t"));
  Serial.println(gc_report.r);
  Serial.print(F("Z:\t"));
  Serial.println(gc_report.z);

  Serial.print(F("Dup:\t"));
  Serial.println(gc_report.dup);
  Serial.print(F("Ddown:\t"));
  Serial.println(gc_report.ddown);
  Serial.print(F("Dright:\t"));
  Serial.println(gc_report.dright);
  Serial.print(F("Dleft:\t"));
  Serial.println(gc_report.dleft);

  Serial.print(F("xAxis:\t"));
  Serial.println(gc_report.xAxis, DEC);
  Serial.print(F("yAxis:\t"));
  Serial.println(gc_report.yAxis, DEC);

  Serial.print(F("cxAxis:\t"));
  Serial.println(gc_report.cxAxis, DEC);
  Serial.print(F("cyAxis:\t"));
  Serial.println(gc_report.cyAxis, DEC);

  Serial.print(F("L:\t"));
  Serial.println(gc_report.left, DEC);
  Serial.print(F("R:\t"));
  Serial.println(gc_report.right, DEC);
  Serial.println();
}
