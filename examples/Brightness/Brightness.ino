/* This example shows how to make an LED pattern with a large
 * dynamic range using the the extra 5-bit brightness register in
 * the APA102.
 *
 * It sets every LED on the strip to white, with the dimmest
 * possible white at the input end of the strip and the brightest
 * possible white at the other end, and a smooth logarithmic
 * gradient between them.
 *
 * The dimmest possible white is achieved by setting the red,
 * green, and blue color channels to 1, and setting the
 * brightness register to 1.  The brightest possibe white is
 * achieved by setting the color channels to 255 and setting the
 * brightness register to 31.
 */

/* By default, the APA102 uses pinMode and digitalWrite to write
 * to the LEDs, which works on all Arduino-compatible boards but
 * might be slow.  If you have a board supported by the FastGPIO
 * library and want faster LED updates, then install the
 * FastGPIO library and uncomment the next two lines: */
// #include <FastGPIO.h>
// #define APA102_USE_FAST_GPIO

#include <APA102.h>

// Define which pins to use.
const uint8_t dataPin = 11;
const uint8_t clockPin = 12;

// Create an object for writing to the LED strip.
APA102<dataPin, clockPin> ledStrip;

// Set the number of LEDs to control.
const uint16_t ledCount = 72;

// We define "value" in this sketch to be the product of the
// 8-bit color channel value and the 5-bit brightness register.
// The maximum possible value is 255 * 31.
const uint16_t maxValue = 255 * 31;

// The value we want to show on the first LED is 1, which
// corresponds to the dimmest possible white.
const uint16_t minValue = 1;

// Calculate what the ratio between the values of consecutive
// LEDs needs to be in order to reach the maxValue on the last
// LED of the strip.
const float multiplier = pow(maxValue / minValue, 1.0 / (ledCount - 1));

void setup()
{
}

void sendWhite(uint16_t value)
{
  uint8_t brightness = 1;
  while(brightness * 0xFF < value)
  {
    brightness++;
  }

  // Uncomment this line to simulate an LED strip that doesn't
  // have the extra 5-bit brightness register.  You will notice
  // that roughly the first third of the LED strip turns off
  // because the brightness2 equals zero.
  //brightness = 31;

  uint8_t brightness2 = (value + (brightness / 2)) / brightness;
  ledStrip.sendColor(brightness2, brightness2, brightness2, brightness);
}

void loop()
{
  ledStrip.startFrame();
  float value = minValue;
  for(uint16_t i = 0; i < ledCount; i++)
  {
    sendWhite(value);
    value = value * multiplier;
  }
  ledStrip.endFrame(ledCount);
}
