#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <string>
#include <stdint.h>

extern bool eventlog_show;
void ResetEventClock();
void AddNewEvent(const std::string& content);
void ClearEventLog();
void DrawEventLog(const float size, uint8_t*);

#endif