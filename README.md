jolla-morse
===========

Ever wanted your text messages imported from your Nokia n900?
Using GlogArchive (http://maemo.org/packages/view/glogarchive/)?

Or maybe you wanted to implement your own conversion using an open-source app that can insert
messages into your Jolla?

If you answered yes, Morse is your app!

Just dump your live db using GlogArchive, transfer the file to your Jolla Documents and open it
with Morse.

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

The sdk emulator ships older versions of the libcommhistory library than what's currently
on the phone. When I tried to get phone call logs imported on the phone, it doesn't find
any of the stored messages or other events anymore. Therefore it inserts duplicates.

I ran a git bisect, marking those commits bad that duplicate SMSs on the phone. I wound up
with this commit: https://github.com/mjtorn/jolla-morse/commit/40618ddda7b6af428de44adc34048f1d5e30a752

The bisect was run on _a rebase of master_ on top of _branch-0.0.4_, so I could easily deploy
on the phone and still find the csv.

Unfortunately that commit works on the emulator, so I don't know where to take this.

Consider this software to be in arrested development. Not the TV show.

Version 0.0.4 was not packaged as an RPM, it's basically about playing around with variable
names, no functional changes.

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

Caveats
-------

There's a lot of detailing I didn't have time to do. Try reading another file while
already reading another one; it won't start new threads but it changes the label.

It requires libcommhistory-qt5-devel. Install it through the IDE/SDK.

I have been solidly in the Python + PostgreSQL = Django world for years and years, so
what little I may have known of c++ is lost. That's more than I can say for QT, which
I never did before. The code may therefore be amateurish and hacky. Any feedback is
welcome, especially if it's educational.

Store?
------

Morse will probably never find itself in the store. I'm sure there's too much of process
in the way for an app I'm doing mostly for myself for a pretty special case. Let's see.

