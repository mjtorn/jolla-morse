jolla-morse
===========

Ever wanted your text messages imported from your n900?
Using GlogArchive (http://maemo.org/packages/view/glogarchive/)?

Or maybe you wanted to implement your own conversion using an open-source app that can insert
messages into your Jolla?

If you answered yes, Morse is your app!

Just dump your live db using GlogArchive, transfer the file to your phone and open it
with Morse.

FAQ
---

Why does Morse insert more messages than were seen?

The short answer is that the commhistory db duplicates data based on multiple
recipients. See Notes below for more info.

Notes
----

It would appear every phone number is its own remoteUid. When an SMS is sent to, say, two
people, it would appear a group is created containing their numbers. It's probably safe
to assume the separate numbers are also created as groups.

When querying commhistory.db, table Groups, it's quite clear the three groups do exist.

Querying the table Events, eg. WHERE groupId IN (4, 19, 20); you see the two unique
remoteUid values and one without a remoteUid value. The one without a value is related
to the Group with both numbers.

Caveats
-------

There's a lot of detailing I didn't have time to do. Try reading another file while
already reading another one; it won't start new threads but it changes the label.

It requires libcommhistory-qt5-devel.

I have been solidly in the Python + PostgreSQL = Django world for years and years, so
what little I may have known of c++ is lost. That's more than I can say for QT, which
I never did before. The code may therefore be amateurish and hacky. Any feedback is
welcome, especially if it's educational.

Store?
------

Morse will probably never find itself in the store. I'm sure there's too much of process
in the way for an app I'm doing mostly for myself for a pretty special case. Let's see.
