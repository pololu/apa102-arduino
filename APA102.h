// Copyright Pololu Corporation.  For more information, see http://www.pololu.com/

#pragma once

/*! \file APA102.h
 * This is the main header file for the APA102 library. */

#include <Arduino.h>

namespace Pololu
{
  #ifndef _POLOLU_RGB_COLOR
  #define _POLOLU_RGB_COLOR
  /*! A struct that can be used to represent colors.  Each field is a number
   * between 0 and 255 that represents the brightness of a component of the
   * color. */
  typedef struct rgb_color
  {
    uint8_t red, green, blue;
  } rgb_color;
  #endif

  /*! An abstract base class for APA102.  This class is useful if you want
   *  to have a pointer that can point to different APA102 objects.
   *
   * The only virtual function provided by this class is write() because making
   * the low-level functions be virtual causes a noticeable slowdown.
   */
  class APA102Base
  {
  public:
    /*! Writes the specified colors to the LED strip.
     *
     * @param colors A pointer to an array of colors.
     * @param count The number of colors to write.
     * @param brightness A 5-bit brightness value (between 0 and 31) that will
     *   be written to each LED.
     */
    virtual void write(rgb_color * colors, uint16_t count, uint8_t brightness = 31) = 0;
  };

  /*! A template class that represents an APA102 or SK9822 LED strip controlled
   * by a particular clock and data pin.
   *
   * @param dataPin The Arduino pin number or name for the pin that will be
   *   used to control the APA102 data input.
   *
   * @param clockPin The Arduino pin number or name for the pin that will be
   *   used to control the APA102 clock input.
   */
  template<uint8_t dataPin, uint8_t clockPin> class APA102 : public APA102Base
  {
  public:

    virtual void write(rgb_color * colors, uint16_t count, uint8_t brightness = 31)
    {
      startFrame();
      for(uint16_t i = 0; i < count; i++)
      {
        sendColor(colors[i], brightness);
      }
      endFrame(count);
    }

    /*! Initializes the I/O lines and sends a "Start Frame" signal to the LED
     *  strip.
     *
     * This is part of the low-level interface provided by this class, which
     * allows you to send LED colors as you are computing them instead of
     * storing them in an array.  To use the low-level interface, first call
     * startFrame(), then call sendColor() some number of times, then call
     * endFrame(). */
    void startFrame()
    {
      init();
      transfer(0);
      transfer(0);
      transfer(0);
      transfer(0);
    }

    /*! Sends an "End Frame" signal to the LED strip.  This is the last step in
     * updating the LED strip if you are using the low-level interface described
     * in the startFrame() documentation.
     *
     * After this function returns, the clock and data lines will both be
     * outputs that are driving low.  This makes it easier to use one clock pin
     * to control multiple LED strips. */
    void endFrame(uint16_t count)
    {
      // We need to send some more bytes to ensure that all the LEDs in the
      // chain see their new color and start displaying it.
      //
      // The data stream seen by the last LED in the chain will be delayed by
      // (count - 1) clock edges, because each LED before it inverts the clock
      // line and delays the data by one clock edge.  Therefore, to make sure
      // the last LED actually receives the data we wrote, the number of extra
      // edges we send at the end of the frame must be at least (count - 1).
      // For the APA102C, that is sufficient.
      //
      // The SK9822 only updates after it sees 32 zero bits followed by one more
      // rising edge.  To avoid having the update time depend on the color of
      // the last LED, we send a dummy 0xFF byte.  (Unfortunately, this means
      // that partial updates of the beginning of an LED strip are not possible;
      // the LED after the last one you are trying to update will be black.)
      // After that, to ensure that the last LED in chain sees 32 zero bits and
      // a rising edge, we need to send at least 65 + (count - 1) edges.  It is
      // sufficent and simpler to just send (5 + count/16) bytes of zeros.
      //
      // We are ignoring the specification for the end frame in the APA102/SK988
      // datasheets because it does not actually ensure that all the LEDs will
      // start displaying their new colors right away.

      transfer(0xFF);
      for (uint16_t i = 0; i < 5 + count / 16; i++)
      {
        transfer(0);
      }
    }

    /*! Sends a single 24-bit color and an optional 5-bit brightness value.
     * This is part of the low-level interface described in the startFrame()
     * documentation. */
    void sendColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness = 31)
    {
      transfer(0b11100000 | brightness);
      transfer(blue);
      transfer(green);
      transfer(red);
    }

    /*! Sends a single 24-bit color and an optional 5-bit brightness value.
     * This is part of the low-level interface described in the startFrame()
     * documentation. */
    void sendColor(rgb_color color, uint8_t brightness = 31)
    {
      sendColor(color.red, color.green, color.blue, brightness);
    }

  protected:
    void init()
    {
      #ifdef APA102_USE_FAST_GPIO
      FastGPIO::Pin<dataPin>::setOutputLow();
      FastGPIO::Pin<clockPin>::setOutputLow();
      #else
      digitalWrite(dataPin, LOW);
      pinMode(dataPin, OUTPUT);
      digitalWrite(clockPin, LOW);
      pinMode(clockPin, OUTPUT);
      #endif
    }

    void transfer(uint8_t b)
    {
      #ifdef APA102_USE_FAST_GPIO
      FastGPIO::Pin<dataPin>::setOutputValue(b >> 7 & 1);
      FastGPIO::Pin<clockPin>::setOutputValueHigh();
      FastGPIO::Pin<clockPin>::setOutputValueLow();
      FastGPIO::Pin<dataPin>::setOutputValue(b >> 6 & 1);
      FastGPIO::Pin<clockPin>::setOutputValueHigh();
      FastGPIO::Pin<clockPin>::setOutputValueLow();
      FastGPIO::Pin<dataPin>::setOutputValue(b >> 5 & 1);
      FastGPIO::Pin<clockPin>::setOutputValueHigh();
      FastGPIO::Pin<clockPin>::setOutputValueLow();
      FastGPIO::Pin<dataPin>::setOutputValue(b >> 4 & 1);
      FastGPIO::Pin<clockPin>::setOutputValueHigh();
      FastGPIO::Pin<clockPin>::setOutputValueLow();
      FastGPIO::Pin<dataPin>::setOutputValue(b >> 3 & 1);
      FastGPIO::Pin<clockPin>::setOutputValueHigh();
      FastGPIO::Pin<clockPin>::setOutputValueLow();
      FastGPIO::Pin<dataPin>::setOutputValue(b >> 2 & 1);
      FastGPIO::Pin<clockPin>::setOutputValueHigh();
      FastGPIO::Pin<clockPin>::setOutputValueLow();
      FastGPIO::Pin<dataPin>::setOutputValue(b >> 1 & 1);
      FastGPIO::Pin<clockPin>::setOutputValueHigh();
      FastGPIO::Pin<clockPin>::setOutputValueLow();
      FastGPIO::Pin<dataPin>::setOutputValue(b >> 0 & 1);
      FastGPIO::Pin<clockPin>::setOutputValueHigh();
      FastGPIO::Pin<clockPin>::setOutputValueLow();
      #else
      digitalWrite(dataPin, b >> 7 & 1);
      digitalWrite(clockPin, HIGH);
      digitalWrite(clockPin, LOW);
      digitalWrite(dataPin, b >> 6 & 1);
      digitalWrite(clockPin, HIGH);
      digitalWrite(clockPin, LOW);
      digitalWrite(dataPin, b >> 5 & 1);
      digitalWrite(clockPin, HIGH);
      digitalWrite(clockPin, LOW);
      digitalWrite(dataPin, b >> 4 & 1);
      digitalWrite(clockPin, HIGH);
      digitalWrite(clockPin, LOW);
      digitalWrite(dataPin, b >> 3 & 1);
      digitalWrite(clockPin, HIGH);
      digitalWrite(clockPin, LOW);
      digitalWrite(dataPin, b >> 2 & 1);
      digitalWrite(clockPin, HIGH);
      digitalWrite(clockPin, LOW);
      digitalWrite(dataPin, b >> 1 & 1);
      digitalWrite(clockPin, HIGH);
      digitalWrite(clockPin, LOW);
      digitalWrite(dataPin, b >> 0 & 1);
      digitalWrite(clockPin, HIGH);
      digitalWrite(clockPin, LOW);
      #endif
    }

  };
}

using namespace Pololu;
