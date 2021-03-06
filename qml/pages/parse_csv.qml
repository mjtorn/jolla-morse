/*
  Copyright (C) 2013 mjt.
  Contact: Markus Törnqvist <mjt@nysv.org>
  All rights reserved.

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Jolla Ltd nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: parse_page
    SilicaFlickable {
        id: main_flickable
        anchors.fill: parent

        Column {
            width: parse_page.width
            height: parse_page.height

            Component.onCompleted: {
                appWindow.csvhandler.parseFile();
            }

            PageHeader {
                title: appWindow.csvhandler.getFileName();
            }

            Label {
                id: state
                text: appWindow.csvhandler.state + "\n"
                x: Theme.paddingLarge
            }

            Label {
                id: bytes_label
                text: "Read bytes: " + appWindow.csvhandler.readBytes
                x: Theme.paddingLarge
            }

            Label {
                id: entries_label
                text: "Seen entries: " + appWindow.csvhandler.seenEntries
                x: Theme.paddingLarge
            }

            Label {
                id: sms_label
                text: "Seen SMS and calls: " + appWindow.csvhandler.seenSMS
                x: Theme.paddingLarge
            }

            Label {
                id: csv_duplicates_label
                text: "Seen CSV duplicates: " + appWindow.csvhandler.seenCSVDuplicates
                x: Theme.paddingLarge
            }

            Label {
                id: groups_label
                text: "Seen groups: " + appWindow.csvhandler.seenGroups
                x: Theme.paddingLarge
            }

            Label {
                id: new_groups_label
                text: "New groups: " + appWindow.csvhandler.newGroups
                x: Theme.paddingLarge
            }

            Label {
                id: duplicate_sms_label
                text: "Duplicate SMS and calls: " + appWindow.csvhandler.duplicateSMS
                x: Theme.paddingLarge
            }

            Label {
                id: inserted_sms_label
                text: "Inserted SMS: " + appWindow.csvhandler.insertedSMS
                x: Theme.paddingLarge
            }

            Label {
                id: inserted_calls_label
                text: "Inserted calls: " + appWindow.csvhandler.insertedCalls
                x: Theme.paddingLarge
            }
        }
    }
}
