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
import harbour.morse.CSVHandler 0.1

Page {
    id: page

    //property alias csvhandler: appWindow.csvhandler

    // To enable PullDownMenu, place our content in a SilicaFlickable
    // Fortunately SilicaListView seems to inherit correct
    SilicaListView {
        id: files

        anchors.fill: parent

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
        PullDownMenu {
            MenuItem {
                text: "About Morse"
                onClicked: pageStack.push(Qt.resolvedUrl("about.qml"))
            }
        }

        // Tell SilicaFlickable the height of its content.
        contentHeight: page.height

        // Tell the user things
        header: PageHeader {
            title: "Morse"
        }

        // This is harder than CSS, maybe a footer is enough ;P
        footer: Label {
            text: "Choose a file to import SMS data from"
        }

        width: page.width
        spacing: Theme.paddingLarge

        // FIXME: Deal with an empty list somehow
        // FIXME: Maybe deal with file going away or coming in
        model: appWindow.csvhandler.getCSVFiles()

        delegate: ListItem {
            width: ListView.view.width
            height: Theme.itemSizeSmall

            // Our QStrings of file names
            Label {
                text: modelData
            }

            onClicked: {
                appWindow.csvhandler.setFile(modelData);
                console.log('Proceed. ' + appWindow.csvhandler.getFilePath());
                pageStack.push(Qt.resolvedUrl("parse_csv.qml"))
            }
        }
    }
}


