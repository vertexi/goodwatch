/*! \file hebrew.c
  \brief Hebrew calendar library for LCD watches.
  
  This is a library for implementing the modern Hebrew calendar on an
  LCD wristwatch, sourced from the RTC module of an MSP430.  I'm new
  to this calendar, so I'd suggest double-checking this code before
  relying on it.
  
  --Travis Goodspeed
  
  Ported to MSP430 from CLISP's spvw_calendar.c, which was ported from Emacs.
  Copyright (C) 1995, 1997 Free Software Foundation, Inc.
  Copyright (C) 2003 Bruno Haible
*/

#include <stdint.h>

#include "hebrew.h"

/* Test whether the given year is a Hebrew calendar leap year. */
static int hebrew_calendar_leap_year_p (int year) {
  return ((7 * year + 1) % 19) < 7;
}

/* Months up to mean conjunction of Tishri of the given year. */
static int hebrew_calendar_elapsed_months (int year) {
  return ((year - 1) / 19) * 235
         + ((year - 1) % 19) * 12
         + (((year - 1) % 19) * 7 + 1) / 19;
}

/* Days up to mean conjunction of Tishri of the given year. */
static int hebrew_calendar_elapsed_days (int year) {
  int months_elapsed = hebrew_calendar_elapsed_months (year);
  int parts_elapsed = (months_elapsed % 1080) * 793 + 204;
  int hours_elapsed =
    5 + months_elapsed * 12 + (months_elapsed / 1080) * 793
    + (parts_elapsed / 1080);
  int parts = (hours_elapsed % 24) * 1080 + (parts_elapsed % 1080);
  int day = 1 + months_elapsed * 29 + (hours_elapsed / 24);
  int day1 =
    (parts >= 19440
     || ((day % 7) == 2 && parts >= 9924
         && !hebrew_calendar_leap_year_p (year))
     || ((day % 7) == 1 && parts >= 16789
         && hebrew_calendar_leap_year_p (year - 1))
     ? day + 1
     : day);
  int day2 =
    ((day1 % 7) == 0 || (day1 % 7) == 3 || (day1 % 7) == 5
     ? day1 + 1
     : day1);
  return day2;
}

/* Return the number of days in the given year. */
/* Example:
     hebrew_calendar_days_in_year (5763) = 385
     hebrew_calendar_days_in_year (5764) = 355
   Note that the result                is in the range 351..357 or 380..386.
   Probably (but I cannot prove it) it is in the range 353..355 or 383..385.
*/
static int hebrew_calendar_days_in_year (int year) {
  return hebrew_calendar_elapsed_days (year + 1)
         - hebrew_calendar_elapsed_days (year);
}

/* Test whether in the given year, the Heshvan month is long. */
static int hebrew_calendar_long_heshvan_p (int year) {
  return (hebrew_calendar_days_in_year (year) % 10) == 5;
}

/* Test whether in the given year, the Kislev month is short. */
static int hebrew_calendar_short_kislev_p (int year) {
  return (hebrew_calendar_days_in_year (year) % 10) == 3;
}

/* Return the number of months of the given year. */
static int hebrew_calendar_months_in_year (int year) {
  return (hebrew_calendar_leap_year_p (year) ? 13 : 12);
}

/* Return the number of days in the given month of the given year. */
static int hebrew_calendar_last_day_of_month (int year, int month) {
  /* Note that month 7 is the first month, and month 6 is the last one. */
  switch (month) {
  case 7: /* Tishri */
    return 30;
  case 8: /* Heshvan */
    return (hebrew_calendar_long_heshvan_p (year) ? 30 : 29);
  case 9: /* Kislev */
    return (hebrew_calendar_short_kislev_p (year) ? 29 : 30);
  case 10: /* Teveth */
    return 29;
  case 11: /* Shevat */
    return 30;
  case 12: /* Adar, or - if leap year - Adar I */
    return (hebrew_calendar_leap_year_p (year) ? 30 : 29);
  case 13: /* - only if leap year - Adar II */
    return 29;
  case 1: /* Nisan */
    return 30;
  case 2: /* Iyar */
    return 29;
  case 3: /* Sivan */
    return 30;
  case 4: /* Tammuz */
    return 29;
  case 5: /* Av */
    return 30;
  case 6: /* Elul */
    return 29;
  default:
    //TODO: Some sort of error message here.
    //abort ();
    return 300; //Very wrong for now.
  }
}

/* Return the number of days since 1900-01-01 of a given Hebrew date. */
static int hebrew_calendar_to_universal (int year, int month, int day) {
  int days;
  int m;

  days = hebrew_calendar_elapsed_days (year) - 2067024;
  if (month < 7) {
    int max_month = hebrew_calendar_months_in_year (year);
    for (m = 7; m <= max_month; m++)
      days += hebrew_calendar_last_day_of_month (year, m);
    for (m = 1; m < month; m++)
      days += hebrew_calendar_last_day_of_month (year, m);
  } else {
    for (m = 7; m < month; m++)
      days += hebrew_calendar_last_day_of_month (year, m);
  }
  days += day - 1;
  return days;
}


/* Return the Hebrew date corresponding to a given universal date (= number
   of days since 1900-01-01). */
/* Example:
     hebrew_calendar_from_universal (37888) = { 5763, 6, 29 }
     hebrew_calendar_from_universal (37889) = { 5764, 7,  1 }
*/
void hebrew_calendar_from_universal (int udate, struct hebrew_date *result) {
  int year;
  int elapsed_days;
  int remaining_days;
  int max_month;
  int month;

  year = (int)((float)udate/(float)365.2422) + 5661;
  for (;; year--) {
    elapsed_days = hebrew_calendar_elapsed_days (year) - 2067024;
    if (udate >= elapsed_days)
      break;
  }

  remaining_days = udate - elapsed_days;
  max_month = hebrew_calendar_months_in_year (year);
  for (month = 7; month <= max_month; month++) {
    int mlength = hebrew_calendar_last_day_of_month (year, month);
    if (remaining_days < mlength)
      break;
    remaining_days -= mlength;
  }
  if (month > max_month) {
    for (month = 1; month < 7; month++) {
      int mlength = hebrew_calendar_last_day_of_month (year, month);
      if (remaining_days < mlength)
        break;
      remaining_days -= mlength;
    }

    //TODO: Re-enable this sanity check.
    //if (month == 7) abort ();
  }

  result->year = year;
  result->month = month;
  result->day = remaining_days + 1;
}

/* Return the number of Hanukka candles for a given universal date. */
static int hebrew_calendar_hanukka_candles (int udate) {
  /* The first day of Hanukka is on 25 Kislev. */
  struct hebrew_date date;
  int hanukka_first_day;

  hebrew_calendar_from_universal (udate, &date);
  hanukka_first_day = hebrew_calendar_to_universal (date.year, 9, 25);
  if (udate - hanukka_first_day >= 0 && udate - hanukka_first_day <= 7)
    return (udate - hanukka_first_day + 1);
  else
    return 0;
}

// To store number of days in all months from January to Dec. 
const int monthDays[12] = {31, 28, 31, 30, 31, 30, 
                           31, 31, 30, 31, 30, 31}; 
  
//! This function counts number of leap years before the given date.
static int countLeapYears(int y, int m, int d) { 
    int years = y; 
  
    // Check if the current year needs to be considered for the count
    // of leap years or not
    if (m <= 2) 
      years--; 
  
    // A year is a leap year if it is a multiple of 4, multiple of 400
    // and not a multiple of 100.
    return years / 4 - years / 100 + years / 400; 
} 
  
// This function returns number of days since 1900-01-01
uint32_t get_universal(int y, int m, int d)  {
  // Total number of days since 0.
  uint32_t n = ((uint32_t)y)*365 + d; 
  for (int i=0; i< m-1; i++) 
    n += monthDays[i]; 
  n += countLeapYears(y, m, d); 
  
  // Add 1900.
  return n-693961;
}

const char *hdaysofweek[7]={
  "  Rishon",
  "   Sheni",
  " Shlishi",
  "  Revi'i",
  "Chamishi",
  "  Shishi",
  " Shabbat"
};

/* Months are in the Civil order number, not the religious order.
 */
const char *hmonths[14]={
  "   Error",
  //Religious year begins here.
  "   Nisan", //7 Civil
  "    Iyar",
  "   Sivan",
  "  Tammuz",
  "      Av",
  "    Elul", //6 Civil
  " Tishrei", //1 Civil
  "Cheshvan",
  "  Kislev",
  "   Tevet",
  "  Shevat",
  "    Adar", //Adar I in leap hears.
  " Adar II"
};


//Define this for unit testing in Unix.
#ifdef STANDALONE

#include <stdio.h>
#include <assert.h>

//Very important, time.h can't be used in functions above.
#include <time.h>

//Gregorian calendar.
time_t gtime;
struct tm* gtm;
//Gregorian year, month, day, day of week
int y=0, m=0, d=0, dow=0;
//Days since 1900;
uint32_t udate=0;

//Hebrew calendar.
struct hebrew_date hdate;

//! Unix tool for testing.
int main(){
  //Asserts to test the sanity of our conversion functions.
  assert(hebrew_calendar_leap_year_p (5763));
  assert(!hebrew_calendar_leap_year_p (5764));
  assert(hebrew_calendar_elapsed_months (5763) == 71266);
  assert(hebrew_calendar_elapsed_months (5764) == 71279);
  assert(hebrew_calendar_elapsed_days (5763) == 2104528);
  assert(hebrew_calendar_elapsed_days (5764) == 2104913);
  assert(hebrew_calendar_days_in_year (5763) == 385);
  assert(hebrew_calendar_days_in_year (5764) == 355);
  assert(hebrew_calendar_to_universal (5763, 6, 29) == 37888);
  assert(hebrew_calendar_to_universal (5764, 7,  1) == 37889);
  //hebrew_calendar_from_universal (37888) = { 5763, 6, 29 }
  hebrew_calendar_from_universal(37888, &hdate);
  assert(hdate.year==5763 &&
	 hdate.month==6   &&
	 hdate.day==29);
  //hebrew_calendar_from_universal (37889) = { 5764, 7,  1 }
  hebrew_calendar_from_universal(37889, &hdate);
  assert(hdate.year==5764 &&
	 hdate.month==7   &&
	 hdate.day==1);
  
  //Get the current time.
  gtime=time(0);
  gtm=gmtime(&gtime);

  //Decode it to what would be in the MSP430 registers.
  y=gtm->tm_year+1900;
  m=gtm->tm_mon+1; //1 is january
  d=gtm->tm_mday;
  dow=gtm->tm_wday;
  printf("Gregorian: %04d.%02d.%02d, DOW=%d\n",
	 y,m,d,dow);

  udate=get_universal(y,m,d);
  //udate=get_universal(1900,1,1);
  printf("Days since 1900: %d\n",
	 udate);

  hebrew_calendar_from_universal(udate, &hdate);
  printf("Hebrew: %02d %s %04d, %s\n",
	 hdate.day, hmonths[hdate.month], hdate.year,
	 hdaysofweek[dow]);
  
  return 0;
}


#endif //STANDALONE
