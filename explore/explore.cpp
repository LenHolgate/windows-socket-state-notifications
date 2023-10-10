
///////////////////////////////////////////////////////////////////////////////
// File: explore.cpp
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

#include "../shared/shared.h"
#include "../shared/tcp_socket.h"
#include "../shared/wsssn.h"

#include <SDKDDKVer.h>

#include <winternl.h>

#include <iostream>

#pragma comment(lib, "ntdll.lib")

using std::cout;
using std::endl;

int main()
{
   InitialiseWinsock();

   // Create an IOCP for notifications...

   HANDLE hIOCP = CreateIOCP();

   SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

   if (s == INVALID_SOCKET)
   {
      ErrorExit("socket");
   }

   SOCK_NOTIFY_REGISTRATION registration {};

   registration.completionKey = nullptr;
   registration.eventFilter = SOCK_NOTIFY_REGISTER_EVENTS_ALL;
   registration.operation = SOCK_NOTIFY_OP_ENABLE;
   //registration.triggerFlags = SOCK_NOTIFY_TRIGGER_LEVEL;
   //registration.triggerFlags = SOCK_NOTIFY_TRIGGER_PERSISTENT | SOCK_NOTIFY_TRIGGER_LEVEL;
   //registration.triggerFlags = SOCK_NOTIFY_TRIGGER_ONESHOT |SOCK_NOTIFY_TRIGGER_LEVEL;
   //registration.triggerFlags = SOCK_NOTIFY_TRIGGER_ONESHOT | SOCK_NOTIFY_TRIGGER_EDGE;
   registration.triggerFlags = SOCK_NOTIFY_TRIGGER_PERSISTENT | SOCK_NOTIFY_TRIGGER_EDGE;
   registration.socket = s;

   DWORD errorCode = ProcessSocketNotifications(
      hIOCP,
      1,                                           // number of registrations in following array
      &registration,                               // array of registrations
      0,                                           // timeout
      0,                                           // num completions
      NULL,                                        // array of completions
      NULL);                                       // number of completions returned

   if (errorCode == 0)
   {
      cout << "success" << endl;

      cout << "events after notification registration: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;

      const auto listeningSocket = CreateListeningSocket();

      sockaddr_in addr {};

      /* Attempt to connect to an address that we won't be able to connect to. */
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      addr.sin_port = htons(listeningSocket.port);

      int result = connect(s, (struct sockaddr*) &addr, sizeof addr);

      if (result == SOCKET_ERROR)
      {
         ErrorExit("connect");
      }

      cout << "events after connect: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;

      // Set it as non-blocking

      unsigned long one = 1;

      if (0 != ioctlsocket(s, (long) FIONBIO, &one))
      {
         ErrorExit("ioctlsocket");
      }

      cout << "events after setting non-blocking: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;

      const SOCKET accepted = listeningSocket.Accept();

      cout << "events after listen accepts: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;

      static const char data[] = { 1, 2, 3, 4, 5 };

      send(accepted, data, sizeof(data), 0);

      cout << "events after accepted sends: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;

      send(accepted, data, sizeof(data), 0);

      cout << "events after accepted sends: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;

      char buf[100];

      int bytesIn = recv(s, buf, sizeof(buf), 0);

      cout << "bytes in: " << bytesIn << " events: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;

      send(accepted, data, sizeof(data), 0);

      cout << "events after accepted sends: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;

      Close(accepted);

      cout << "events after accepted closes: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;

      bytesIn = recv(s, buf, sizeof(buf), 0);

      cout << "bytes in: " << bytesIn << " events: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;

      bytesIn = recv(s, buf, sizeof(buf), 0);

      cout << "bytes in: " << bytesIn << " events: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;

      registration.operation = SOCK_NOTIFY_OP_REMOVE;

      errorCode = ProcessSocketNotifications(
         hIOCP,
         1,                                           // number of registrations in following array
         &registration,                               // array of registrations
         0,                                           // timeout
         0,                                           // num completions
         NULL,                                        // array of completions
         NULL);                                       // number of completions returned

      if (errorCode != 0)
      {
         ErrorExit("ProcessSocketNotifications");

      }

      cout << "events after unregister: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;

      Close(s);

      cout << "events after connected closes: " << EventsAsString(GetEvents(hIOCP, 0)) << endl;


   }
   else if (errorCode == WAIT_TIMEOUT)
   {
      cout << "pending" << endl;
   }
   else
   {
      ErrorExit("ProcessSocketNotifications");
   }

   closesocket(s);

   CloseHandle(hIOCP);

   cout << "all done" << endl;
}

///////////////////////////////////////////////////////////////////////////////
// End of file: explore.cpp
///////////////////////////////////////////////////////////////////////////////
