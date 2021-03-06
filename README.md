jolla-morse
===========

Ever wanted your text messages imported from your Nokia n900?
Using GlogArchive (http://maemo.org/packages/view/glogarchive/)?

Or maybe you wanted to implement your own conversion using an open-source app that can insert
messages into your Jolla?

If you answered yes, Morse is your app!

Just dump your live db using GlogArchive, transfer the file to your Jolla Documents and open it
with Morse.

OpenRepos page: https://openrepos.net/content/mjtorn/morse

Screenshots: http://imgur.com/a/vO1QU

RMPS: https://github.com/mjtorn/jolla-morse/tree/master/RPMS

This software comes with no warranty. Take a backup, eg. by taking a tarball of
/home/nemo/.local/share/commhistory/ or however you want it.

You may want to run the app in airplane mode so no incoming sms will screw you up, especially
if you have to restore the backup later for some reason.

I tested importing the csv twice and the duplicates were recognized properly, but no
guarantees, warrantees, or anythings.

IMPORTANT NOTICE
----------------

There is partial support in the code for importing phone calls. This is disabled, however
because of certain bugs:

 * The n900 doesn't store phone call durations, which makes it a bit less useful
 * My CSV parser must be buggy, because it inserts empty and ";" messages when inserting phone calls
 * When looking at the call history after an import, phone calls are in no particular order. Bug in Jolla?

FAQ
---

_Why does Morse insert more messages than were seen?_

The short answer is that the commhistory db duplicates data based on multiple
recipients. See Notes below for more info.

_Do you take pull requests?_

Absolutely! I know there's a lot of stuff that could and should look better, so that's
definitely welcome. Also if you want to have support for other dumps that n900/Glog,
I'm sure a lot of people are interested!

Only I can't test everything. I got my messages imported on the Jolla and that's
all I can say, so I will trust you guys to do your testing if you submit PRs!

Notes
----

It would appear every phone number is its own remoteUid. When an SMS is sent to, say, two
people, it would appear a group is created containing their numbers. It's probably safe
to assume the separate numbers are also created as groups.

When querying commhistory.db, table Groups, it's quite clear the three groups do exist.

Querying the table Events, eg. WHERE groupId IN (4, 19, 20); you see the two unique
remoteUid values and one without a remoteUid value. The one without a value is related
to the Group with both numbers.

There are discrepancies between the n900 and Jolla data here and there, please refer
to the comments in insertworker.cpp for more info.

The way Jolla appears to store phone calls is as follows:

Outgoing calls: direction=2, isRead=1, isMissedCall=0, endTime <> startTime

Outgoing calls, aborted by caller or receiver: direction=2, isRead=1, isMissedCall=0, endTime = startTime

Incoming, aborted calls: direction=1, isRead=0, isMissedCall=0, endTime = startTime

Incoming, called canceled: direction=1, isRead=0, isMissedCall=1, endTime = startTime

Why outgoing calls are marked read is beyond me.

When someone sent you a Contact Card, GlogArchive exports it as an empty message, which is
stored as ";" in the messages due to the CSV parser having problems with empty messages.

Caveats
-------

There's a lot of detailing I didn't have time to do. Try reading a second file while
already reading a file; it won't start new threads but it changes the label.

It requires libcommhistory-qt5-devel. Install it through the IDE/SDK.

I have been solidly in the Python + PostgreSQL = Django world for years and years, so
what little I may have known of c++ is lost. That's more than I can say for QT, which
I never did before. The code may therefore be amateurish and hacky. Any feedback is
welcome, especially if it's educational.

Store?
------

Morse will probably never find itself in the store. I'm sure there's too much of process
in the way for an app I'm doing mostly for myself for a pretty special case.

Considering how hard the reverse-engineering has been, and what risks always lie
in that kind of code, I would never carry the responsibility of this app. Sorry.

