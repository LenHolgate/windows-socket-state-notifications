#pragma once
///////////////////////////////////////////////////////////////////////////////
// File: wsssn.h
///////////////////////////////////////////////////////////////////////////////
//
// The code in this file is released under the The MIT License (MIT)
//
// Copyright (c) 2023 Len Holgate.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "shared.h"

using std::string;

inline DWORD GetEvents(
   HANDLE hIOCP,
   DWORD timeoutMs)
{
   OVERLAPPED_ENTRY notification{};

   DWORD notificationEventCount = 0;

   if (::GetQueuedCompletionStatusEx(hIOCP, &notification, 1, &notificationEventCount, timeoutMs, FALSE))
   {
      return SocketNotificationRetrieveEvents(&notification);
   }

   const DWORD lastError = GetLastError();

   if (WAIT_TIMEOUT == lastError)
   {
      return 0;
   }
 
   ErrorExit("GetEvents - GetQueuedCompletionStatusEx");
}

inline string EventsAsString(
   const DWORD events)
{
   string asString;

   if (events == 0)
   {
      return "NONE";
   }

   if (events & SOCK_NOTIFY_EVENT_IN)
   {
      asString += "IN";
   }

   if (events & SOCK_NOTIFY_EVENT_OUT)
   {
      if (!asString.empty())
      {
         asString += ", ";
      }
      asString += "OUT";
   }

   if (events & SOCK_NOTIFY_EVENT_HANGUP)
   {
      if (!asString.empty())
      {
         asString += ", ";
      }
      asString += "HUP";
   }

   if (events & SOCK_NOTIFY_EVENT_ERR)
   {
      if (!asString.empty())
      {
         asString += ", ";
      }
      asString += "ERR";
   }

   if (events & SOCK_NOTIFY_EVENT_REMOVE)
   {
      if (!asString.empty())
      {
         asString += ", ";
      }
      asString += "REMOVED";
   }

   if (asString.empty())
   {
      asString = "UNKNOWN";
   }

   return asString;
}

///////////////////////////////////////////////////////////////////////////////
// End of file: wsssn.h
///////////////////////////////////////////////////////////////////////////////
