/*
 * Print.cpp - Base class that provides print() and println()
 * Copyright (c) 2008 David A. Mellis.  All right reserved.
 * Copyright (c) 2011 LeafLabs, LLC.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * Modified 23 November 2006 by David A. Mellis
 * Modified 12 April 2011 by Marti Bolivar <mbolivar@leaflabs.com>
 */

#include "Print.h"

#include "wirish_math.h"
#include "limits.h"

#ifndef LLONG_MAX
/*
 * Note:
 *
 * At time of writing (12 April 2011), the limits.h that came with the
 * newlib we distributed didn't include LLONG_MAX.  Because we're
 * staying away from using templates (see /notes/coding_standard.rst,
 * "Language Features and Compiler Extensions"), this value was
 * copy-pasted from a println() of the value
 *
 *     std::numeric_limits<long long>::max().
 */
#define LLONG_MAX 9223372036854775807LL
#endif

/*
 * Public methods
 */

size_t Print::write(const char *str) {
    if (str == NULL) return 0;
	return write((const uint8_t *)str, strlen(str));
}

size_t Print::write(const uint8_t *buffer, size_t size) {
	size_t n = 0;
    while (size--) {
        write(*buffer++);
        n++;
    }
	return n;
}

size_t Print::print(uint8 b, int base) {
    return print((uint64)b, base);
}

size_t Print::print(const String &s)
{
  return write(s.c_str(), s.length());
}

size_t Print::print(char c) {
    return write(c);
}

size_t Print::print(const char str[]) {
    return write(str);
}

size_t Print::print(int n, int base) {
    return print((long long)n, base);
}

size_t Print::print(unsigned int n, int base) {
    return print((unsigned long long)n, base);
}

size_t Print::print(long n, int base) {
    return print((long long)n, base);
}

size_t Print::print(unsigned long n, int base) {
    return print((unsigned long long)n, base);
}

size_t Print::print(long long n, int base) {
    if (n < 0) {
        print('-');
        n = -n;
    }
    return printNumber(n, base);
}

size_t Print::print(unsigned long long n, int base) {
	return printNumber(n, base);
}

size_t Print::print(double n, int digits) {
    return printFloat(n, digits);
}

size_t Print::print(const __FlashStringHelper *ifsh)
{
  return print(reinterpret_cast<const char *>(ifsh));
}

size_t Print::print(const Printable& x)
{
  return x.printTo(*this);
}

size_t Print::println(void) 
{
	return write("\r\n", 2);
}

size_t Print::println(const String &s)
{
  size_t n = print(s);
  n += println();
  return n;
}

size_t Print::println(char c) {
    size_t n = print(c);
    n += println();
	return n;
}

size_t Print::println(const char c[]) {
    size_t n = print(c);
    n += println();
	return n;
}

size_t Print::println(uint8 b, int base) {
    size_t n = print(b, base);
	n += println();
	return n;
}

size_t Print::println(int n, int base) {
    size_t s = print(n, base);
    s += println();
	return s;
}

size_t Print::println(unsigned int n, int base) {
    size_t s = print(n, base);
    s += println();
	return s;
}

size_t Print::println(long n, int base) {
    size_t s = print((long long)n, base);
    s += println();
	return s;
}

size_t Print::println(unsigned long n, int base) {
    size_t s = print((unsigned long long)n, base);
    s += println();
	return s;
}

size_t Print::println(long long n, int base) {
    size_t s = print(n, base);
    s += println();
	return s;
}

size_t Print::println(unsigned long long n, int base) {
    size_t s = print(n, base);
    s += println();
	return s;
}

size_t Print::println(double n, int digits) {
    size_t s = print(n, digits);
    s += println();
	return s;
}

size_t Print::println(const __FlashStringHelper *ifsh)
{
  size_t n = print(ifsh);
  n += println();
  return n;
}

size_t Print::println(const Printable& x)
{
  size_t n = print(x);
  n += println();
  return n;
}

size_t Print::printf(const char* format, ...)
{
	char loc_buf[64];
	char* temp = loc_buf;
	va_list arg;
	va_list copy;
	va_start(arg, format);
	va_copy(copy, arg);
	int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
	va_end(copy);
	if (len < 0) {
		va_end(arg);
		return 0;
	};
	if (len >= sizeof(loc_buf)) {
		temp = (char*)malloc(len + 1);
		if (temp == NULL) {
			va_end(arg);
			return 0;
		}
		len = vsnprintf(temp, len + 1, format, arg);
}
	va_end(arg);
	len = write((uint8_t*)temp, len);
	if (temp != loc_buf) {
		free(temp);
	}
	return len;
}

/*
 * Private methods
 */

size_t Print::printNumber(unsigned long long n, uint8 base) {
    unsigned char buf[CHAR_BIT * sizeof(long long)];
    unsigned long i = 0;
	size_t s=0;
    if (n == 0) {
        print('0');
        return 1;
    }

    while (n > 0) {
        buf[i++] = n % base;
        n /= base;
    }

    for (; i > 0; i--) {
        s += print((char)(buf[i - 1] < 10 ?
                     '0' + buf[i - 1] :
                     'A' + buf[i - 1] - 10));
    }
	return s;
}


/* According to snprintf(),
 *
 * nextafter((double)numeric_limits<long long>::max(), 0.0) ~= 9.22337e+18
 *
 * This slightly smaller value was picked semi-arbitrarily. */
#define LARGE_DOUBLE_TRESHOLD (9.1e18)

/* THIS FUNCTION SHOULDN'T BE USED IF YOU NEED ACCURATE RESULTS.
 *
 * This implementation is meant to be simple and not occupy too much
 * code size.  However, printing floating point values accurately is a
 * subtle task, best left to a well-tested library function.
 *
 * See Steele and White 2003 for more details:
 *
 * http://kurtstephens.com/files/p372-steele.pdf
 */
size_t Print::printFloat(double number, uint8 digits) {
size_t s=0;
    // Hackish fail-fast behavior for large-magnitude doubles
    if (abs(number) >= LARGE_DOUBLE_TRESHOLD) {
        if (number < 0.0) {
            s=print('-');
        }
        s+=print("<large double>");
        return s;
    }

    // Handle negative numbers
    if (number < 0.0) {
        s+=print('-');
        number = -number;
    }

    // Simplistic rounding strategy so that e.g. print(1.999, 2)
    // prints as "2.00"
    double rounding = 0.5;
    for (uint8 i = 0; i < digits; i++) {
        rounding /= 10.0;
    }
    number += rounding;

    // Extract the integer part of the number and print it
    long long int_part = (long long)number;
    double remainder = number - int_part;
    s+=print(int_part);

    // Print the decimal point, but only if there are digits beyond
    if (digits > 0) {
        s+=print(".");
    }

    // Extract digits from the remainder one at a time
    while (digits-- > 0) {
        remainder *= 10.0;
        int to_print = (int)remainder;
        s+=print(to_print);
        remainder -= to_print;
    }
	return s;
}

