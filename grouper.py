#!/usr/bin/env python
# vim: fileencoding=utf-8

import sys

# (uid, txt)

SMSDATA = (
    ('1', 'foo'),
    ('2', 'foo'),
    ('1', 'bar'),
    ('3', 'asdf'),
    ('4', 'asdf'),
    ('5', '<3'),
    ('5', 'bar'),
    ('5', 'joo'),
    ('1', 'jolla'),
    ('2', 'jolla'),
    ('7', 'seven'), # <- comment out to test group 1,2,4
    ('4', 'jolla'),
    ('4', 'other'),
    ('7', 'other'),
    ('4', 'other half'),
    ('7', 'other half'),
    ('7', 'other side'),
    ('4', 'other side'),
)

def main():
    seen_uids = set()
    prev_txt = ''
    uid_stack = []
    groups = []

    for i in xrange(len(SMSDATA)):
        uid, txt = SMSDATA[i]
        uid_stack.append(uid)

        # Creating a group of the first entry and then the second is a bug
        # As is creating group 4,5
        if txt != prev_txt:
            uid_stack.sort()
            joined_uids = ','.join(uid_stack)
            if joined_uids in seen_uids:
                print 'already seen %s' % joined_uids
                continue

            seen_uids.add(joined_uids)
            print 'Create group %s' % joined_uids
            groups.append(joined_uids)
            uid_stack = []

        prev_txt = txt

    # This is a bug
    print uid_stack

    print groups

    return 0

def main_fixed():
    seen_uids = set()
    uid_set = set()
    groups = []

    for i in xrange(len(SMSDATA) - 1):
        uid, txt = SMSDATA[i]
        next_uid, next_txt = SMSDATA[i + 1]

        uid_set.add(uid)

        if next_txt != txt:
            uid_stack = list(uid_set)
            uid_stack.sort()
            joined_uids = ','.join(uid_stack)
            if joined_uids in seen_uids:
                print 'already seen %s' % joined_uids
                uid_set = set()
                continue

            seen_uids.add(joined_uids)
            print 'Create group %s' % joined_uids
            groups.append(joined_uids)
            uid_set = set()

    # Handle the last one separate
    next_uid, next_txt = SMSDATA[-1]
    uid_set.add(next_uid)

    uid_stack = list(uid_set)
    uid_stack.sort()
    joined_uids = ','.join(uid_stack)
    if joined_uids in seen_uids:
        print 'already seen %s' % joined_uids
        uid_set = set()
    else:
        seen_uids.add(joined_uids)
        print 'Create group %s' % joined_uids
        groups.append(joined_uids)

        uid_set = set()

    print uid_set

    print groups

    return 0

if __name__ == '__main__':
    print SMSDATA
    sys.exit(main_fixed())

# EOF

