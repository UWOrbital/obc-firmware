/*
@brief Fault monitor test for rm46. Test verifies that the rm46 can run continuously for at least 60 seconds without
resetting. Reads timestamp of log at the beginning of test, waits for 90 seconds, then checks logs to ensure system
remained operational during the time period. Test passes if RM46 produces logs covering the full time window and fails
if there is a crash or reset during the period.

@note Please test with code where there is a segfault, and where there is stable code. Ensure test fails for segfault
and passes for main.
*/
// local library imports
/* TODO: include a log reader or direct communicate with rm46 ? */

// external library imports
#include <gtest/gtest.h>

// standard library imports
#include <chrono>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <string>

#define MIN_LOGS_AQUIRED 2
#define LOG_FILE "foo.txt" /* some log file*/
#define DELAY_PERIOD_SECONDS 90

/**
 * TODO: Implement this function
 * @brief Parses rm46 log line for timestamp using regex.
 *
 * @param line - line to parse.
 * @return Returns the timestamp in seconds.
 */
unsigned int parseTimeStamp(std::string line) {
  // some regex stuff
  unsigned int timeStampSeconds = 69;
  return timeStampSeconds;
}

TEST(/*FaultMonitorTask, rm46_logs*/) {
  std::ifstream read;
  read.open(LOG_FILE);
  ASSERT_TRUE(read.is_open()); /* log must open */

  std::string line;
  std::getline(read, line);
  ASSERT_FALSE(line.empty()); /* log must be non-empty */

  // TO DO: Implement a parse time function from logs
  unsigned int initialTime = parseTimeStamp(line); /* some time */

  read.close();

  std::this_thread::sleep_for(std::chrono::seconds(DELAY_PERIOD_SECONDS)); /* make current thread sleep for 90s */

  read.open(LOG_FILE);
  ASSERT_TRUE(read.is_open());

  bool passed{false};
  while (std::getline(read, line)) {
    unsigned int finalTime = parseTimeStamp(line);
    if (finalTime - initialTime >= 60) {
      passed = true;
      break;
    }
    // automatically terminates if we reach the end.
  };

  EXPECT_TRUE(passed);

  read.close();
}
